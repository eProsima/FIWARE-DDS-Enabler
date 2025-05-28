// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file EnablerParticipant.cpp
 */

#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/data/RpcPayloadData.hpp>
#include <ddspipe_core/types/dds/Payload.hpp>
#include <ddspipe_core/types/dynamic_types/types.hpp>
#include <ddspipe_participants/participant/rtps/CommonParticipant.hpp>
#include <ddspipe_participants/reader/auxiliar/BlankReader.hpp>

#include <ddsenabler_participants/CBHandler.hpp>
#include <ddsenabler_participants/serialization.hpp>

#include <ddsenabler_participants/EnablerParticipant.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;

EnablerParticipant::EnablerParticipant(
        std::shared_ptr<EnablerParticipantConfiguration> participant_configuration,
        std::shared_ptr<PayloadPool> payload_pool,
        std::shared_ptr<DiscoveryDatabase> discovery_database,
        std::shared_ptr<ISchemaHandler> schema_handler)
    : ddspipe::participants::SchemaParticipant(participant_configuration, payload_pool, discovery_database,
            schema_handler)
{
}

bool EnablerParticipant::service_discovered_nts(
        const std::string& service_name,
        const DdsTopic& topic,
        RpcUtils::RpcType rpc_type)
{
    auto [it, inserted] = services_.try_emplace(service_name, service_name);
    return it->second.add_topic(topic, rpc_type);
}

bool EnablerParticipant::action_discovered_nts(
    const std::string& action_name,
    const DdsTopic& topic,
    RpcUtils::RpcType rpc_type)
{
    auto [it, inserted] = actions_.try_emplace(action_name, action_name);
    return it->second.add_topic(topic, rpc_type);
}

// TODO filter ros default services as /get_parameters ... ?
std::shared_ptr<IReader> EnablerParticipant::create_reader(
        const ITopic& topic)
{
    if (is_type_object_topic(topic))
    {
        return std::make_shared<BlankReader>();
    }

    std::shared_ptr<IReader> reader;
    {
        std::lock_guard<std::mutex> lck(mtx_);
        auto dds_topic = dynamic_cast<const DdsTopic&>(topic);
        std::string rpc_name;
        RpcUtils::RpcType rpc_type = RpcUtils::get_rpc_name(dds_topic.m_topic_name, rpc_name);
        if (RpcUtils::RpcType::RPC_NONE != rpc_type)
        {
            RpcUtils::RpcType rpc_direction = RpcUtils::get_service_direction(rpc_type);
            if (RpcUtils::RpcType::RPC_NONE != rpc_direction)
                reader = std::make_shared<InternalRpcReader>(id(), dds_topic);
            else
                reader = std::make_shared<InternalReader>(id());

            if (RpcUtils::ActionType::NONE == RpcUtils::get_action_type(rpc_type))
            {
                if (service_discovered_nts(rpc_name, dds_topic, rpc_type))
                {
                    RpcTopic service = services_.find(rpc_name)->second.get_service();
                    std::static_pointer_cast<CBHandler>(schema_handler_)->add_service(service);
                }
            }
            else
            {
                if (action_discovered_nts(rpc_name, dds_topic, rpc_type))
                {
                    auto action_discovered = actions_.find(rpc_name)->second;
                    services_.insert({action_discovered.goal.service_name, action_discovered.goal});
                    services_.insert({action_discovered.result.service_name, action_discovered.result});
                    services_.insert({action_discovered.cancel.service_name, action_discovered.cancel});
                    auto action = action_discovered.get_action(rpc_name);
                    std::static_pointer_cast<CBHandler>(schema_handler_)->add_action(action);
                }
            }
        }
        else
        {
            reader = std::make_shared<InternalReader>(id());
            std::static_pointer_cast<CBHandler>(schema_handler_)->add_topic(dds_topic);
        }
        readers_[dds_topic] = reader;
    }
    cv_.notify_all();
    return reader;
}

ddspipe::core::types::Endpoint EnablerParticipant::create_topic_writer_nts_(
        const DdsTopic& topic,
        std::shared_ptr<IReader>& reader,
        std::unique_lock<std::mutex>& lck)
{
    auto request_edp = rtps::CommonParticipant::simulate_endpoint(topic, this->id());
    this->discovery_database_->add_endpoint(request_edp);

    cv_.wait(lck, [&]
            {
                return nullptr != (reader = lookup_reader_nts_(topic.m_topic_name));
            });
    // (Optionally) wait for writer created in DDS participant to match with external readers, to avoid losing this
    // message when not using transient durability
    std::this_thread::sleep_for(std::chrono::milliseconds(std::static_pointer_cast<EnablerParticipantConfiguration>(
                configuration_)->initial_publish_wait));

    // TODO handle potential failure
    return request_edp;
}

bool EnablerParticipant::publish(
        const std::string& topic_name,
        const std::string& json)
{
    std::unique_lock<std::mutex> lck(mtx_);

    std::string type_name;
    auto i_reader = std::dynamic_pointer_cast<IReader>(lookup_reader_nts_(topic_name, type_name));

    if (nullptr == i_reader)
    {

        DdsTopic topic;
        if(!request_topic(topic_name, topic))
        {
            return false;
        }

        create_topic_writer_nts_(topic, i_reader, lck);

        // (Optionally) wait for writer created in DDS participant to match with external readers, to avoid losing this
        // message when not using transient durability
        std::this_thread::sleep_for(std::chrono::milliseconds(std::static_pointer_cast<EnablerParticipantConfiguration>(
                    configuration_)->initial_publish_wait));
    }

    auto reader = std::dynamic_pointer_cast<InternalReader>(i_reader);

    auto data = std::make_unique<RtpsPayloadData>();

    Payload payload;
    if (!std::static_pointer_cast<CBHandler>(schema_handler_)->get_serialized_data(type_name, json, payload))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to publish data in topic " << topic_name << " : data serialization failed.");
        return false;
    }

    if (!payload_pool_->get_payload(payload, data->payload))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to publish data in topic " << topic_name << " : get_payload failed.");
        return false;
    }

    reader->simulate_data_reception(std::move(data));
    return true;
}

bool EnablerParticipant::publish_rpc(
    const std::string& topic_name,
    const std::string& json,
    const uint64_t request_id)
{
    std::unique_lock<std::mutex> lck(mtx_);

    std::string service_name;
    RpcUtils::RpcType rpc_type = RpcUtils::get_service_name(topic_name, service_name);

    {
        auto it = services_.find(service_name);
        if (it == services_.end())
        {
            // There is no case where none of the service topics are discovered and yet the publish should be done
            EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                    "Failed to publish data in service " << service_name << " : service does not exist.");
            return false;
        }
    }

    std::string type_name;
    auto reader = std::dynamic_pointer_cast<InternalRpcReader>(lookup_reader_nts_(topic_name, type_name));

    if (nullptr == reader)
    {
        // TODO is this situation possible??
        ServiceDiscovered service(service_name);
        if(!request_service(service))
        {
            return false;
        }
        bool ret = create_service_writer_nts_(rpc_type, service, lck);
        reader = std::dynamic_pointer_cast<InternalRpcReader>(lookup_reader_nts_(topic_name, type_name));
        if (!ret || nullptr == reader)
        {
            EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                    "Failed to publish data in service " << service_name << " : service writer creation failed.");
            return false;
        }
        services_.try_emplace(service_name, service);
    }

    DdsTopic topic;
    if (!services_.at(service_name).get_topic(rpc_type, topic))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to publish data in service " << service_name << " : topic not found.");
        return false;
    }

    if(type_name != topic.type_name)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to publish data in topic " << topic.m_topic_name << " : type name mismatch.");
        return false;
    }

    auto data = std::make_unique<RpcPayloadData>();

    Payload payload;
    if (!std::static_pointer_cast<CBHandler>(schema_handler_)->get_serialized_data(type_name, json, payload))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to publish data in topic " << topic.m_topic_name << " : data serialization failed.");
        return false;
    }

    if (!payload_pool_->get_payload(payload, data->payload))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to publish data in topic " << topic.m_topic_name << " : get_payload failed.");
        return false;
    }

    data->origin_sequence_number = eprosima::fastdds::rtps::SequenceNumber_t(request_id);
    data->sent_sequence_number = eprosima::fastdds::rtps::SequenceNumber_t(request_id);
    data->participant_receiver = id();

    fastdds::rtps::SampleIdentity sample_identity;
    sample_identity.sequence_number(fastdds::rtps::SequenceNumber_t(request_id));
    sample_identity.writer_guid(reader->guid());
    data->write_params.get_reference().sample_identity(sample_identity);
    data->write_params.get_reference().related_sample_identity(sample_identity);

    reader->simulate_data_reception(std::move(data));
    return true;
}

bool EnablerParticipant::create_service_writer_nts_(
        const RpcUtils::RpcType& rpc_type,
        ServiceDiscovered& service,
        std::unique_lock<std::mutex>& lck)
{
    DdsTopic* topic;
    if (rpc_type == RpcUtils::RpcType::RPC_REQUEST)
    {
        topic = &service.topic_request;
    }
    else if (rpc_type == RpcUtils::RpcType::RPC_REPLY)
    {
        topic = &service.topic_reply;
    }
    else
        return false;

    std::string _;
    auto reader = std::dynamic_pointer_cast<IReader>(lookup_reader_nts_(topic->m_topic_name, _));

    if (nullptr == reader)
    {
        auto request_edp = create_topic_writer_nts_(
                *topic,
                reader,
                lck);
        server_endpoint_.emplace(service.service_name, request_edp);

        return true;
    }

    // TODO What if there is a server already running in ROS2, currently the announce will fail
    std::cout << "SERVICE ALREADY ANNOUNCED" << std::endl;
    return false;
}

bool EnablerParticipant::announce_service(
    const std::string& service_name)
{
    std::unique_lock<std::mutex> lck(mtx_);

    // auto it = services_.find(service_name);
    // if (it != services_.end())
    // {
    //     // TODO what to do in case the client is already discovered? The discovered info will be overwritten. Check if it is equal to the requested one?
    //     if (it->second.request_discovered)
    //     {
    //         // TODO handle another server already running in ROS2
    //         EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
    //                 "Failed to announce service " << service_name << " : service already announced.");
    //         return false;
    //     }
    // }

    ServiceDiscovered service(service_name);
    if(!request_service(service))
    {
        return false;
    }

    if (create_service_writer_nts_(RpcUtils::RPC_REQUEST, service, lck))
    {
        // TODO once rebased adding it to services_ is only neccessary if it is not being discarded in create reader
        services_.insert_or_assign(service_name, service);
        return true;
    }

    EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to announce service " << service_name << " : service writer creation failed.");
    return false;
}

// TODO after revoking the service the client is still matched, probably the dds participant does not remove its entities
bool EnablerParticipant::revoke_service(
    const std::string& service_name)
{
    std::unique_lock<std::mutex> lck(mtx_);

    return revoke_service_nts(service_name);
}

bool EnablerParticipant::revoke_service_nts(
    const std::string& service_name)
{
    {
        auto it = services_.find(service_name);
        if (it == services_.end())
        {
            EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                    "Failed to stop service " << service_name << " : service not found.");
            return false;
        }
        it->second.remove_topic(RpcUtils::RpcType::RPC_REQUEST);
    }

    std::string request_name = "rq/" + service_name + "Request";

    auto it = server_endpoint_.find(service_name);
    if(it == server_endpoint_.end())
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to stop service " << service_name << " : server not found.");
        return false;
    }

    this->discovery_database_->erase_endpoint(it->second);
    server_endpoint_.erase(it);

    auto reader = lookup_reader_nts_(request_name);
    if (nullptr != reader)
    {
        readers_.erase(reader->topic());
    }

    return true;
}

bool EnablerParticipant::announce_action(
    const std::string& action_name)
{
    std::unique_lock<std::mutex> lck(mtx_);

    ActionDiscovered action(action_name);
    if(!request_action(action))
    {
        return false;
    }

    if (!create_service_writer_nts_(RpcUtils::RPC_REQUEST, action.goal, lck))
        return false;
    // TODO once rebased adding it to services_ is only neccessary if it is not being discarded in create reader
    services_.insert_or_assign(action.goal.service_name, action.goal);

    if (!create_service_writer_nts_(RpcUtils::RPC_REQUEST, action.result, lck))
        return false;
    services_.insert_or_assign(action.result.service_name, action.result);

    if (!create_service_writer_nts_(RpcUtils::RPC_REQUEST, action.cancel, lck))
        return false;
    services_.insert_or_assign(action.cancel.service_name, action.cancel);

    {
        std::string _;
        auto feedback_reader = std::dynamic_pointer_cast<IReader>(lookup_reader_nts_(action.feedback.m_topic_name, _));
        if (!feedback_reader)
            create_topic_writer_nts_(
                    action.feedback,
                    feedback_reader,
                    lck);
    }

    {
        std::string _;
        auto status_reader = std::dynamic_pointer_cast<IReader>(lookup_reader_nts_(action.status.m_topic_name, _));
        if (!status_reader)
            create_topic_writer_nts_(
                    action.status,
                    status_reader,
                    lck);
    }
    // TODO once rebased adding it to actions_ is only neccessary if it is not being discarded in create reader
    actions_.insert_or_assign(action_name, action);
    return true;
}

bool EnablerParticipant::revoke_action(
    const std::string& action_name)
{
    std::unique_lock<std::mutex> lck(mtx_);

    auto it = actions_.find(action_name);
    if (it == actions_.end() || !it->second.fully_discovered)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to stop action " << action_name << " : action not found.");
        return false;
    }

    auto& action = it->second;

    if (this->revoke_service_nts(action.goal.service_name) &&
            this->revoke_service_nts(action.result.service_name) &&
            this->revoke_service_nts(action.cancel.service_name))
    {
        action.fully_discovered = false;
        return true;
    }

    return false;
}

bool EnablerParticipant::request_topic(
    const std::string& topic_name,
    DdsTopic& topic)
{
    if (!topic_req_callback_)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to publish data in topic " << topic_name <<
                            " : topic is unknown and topic request callback not set.");
        return false;
    }
    char* _type_name;
    char* serialized_qos_content;

    topic_req_callback_(topic_name.c_str(), _type_name, serialized_qos_content); // TODO: allow the user not to provide QoS + handle fail case

    return fullfill_topic_type(topic_name, _type_name, serialized_qos_content, topic);
}

bool EnablerParticipant::fullfill_topic_type(
    const std::string& topic_name,
    const char* _type_name,
    const char* serialized_qos_content,
    DdsTopic& topic)

{
    std::string type_name = std::string(_type_name); // TODO: free resources allocated by user, or redesign interaction (same for serialized_qos_content)
    std::string serialized_qos(serialized_qos_content);

    TopicQoS qos = serialization::deserialize_qos(serialized_qos); // TODO: handle fail case (try-catch?)

    fastdds::dds::xtypes::TypeIdentifier type_identifier;
    if (!std::static_pointer_cast<CBHandler>(schema_handler_)->get_type_identifier(type_name, type_identifier))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to create topic " << topic_name << " : type identifier not found.");
        return false;
    }

    topic.m_topic_name = topic_name;
    topic.type_name = type_name;
    topic.topic_qos = qos;
    topic.type_identifiers.type_identifier1(type_identifier);

    return true;
}

bool EnablerParticipant::fullfill_service_type(
    const char* _request_type_name,
    const char* serialized_request_qos_content,
    const char* _reply_type_name,
    const char* serialized_reply_qos_content,
    ServiceDiscovered& service)
{

    DdsTopic topic_request;
    std::string topic_request_name = "rq/" + service.service_name + "Request";
    if(!fullfill_topic_type(topic_request_name, _request_type_name, serialized_request_qos_content, topic_request))
        return false;
    service.add_topic(topic_request, RpcUtils::RPC_REQUEST);

    DdsTopic topic_reply;
    std::string topic_reply_name = "rr/" + service.service_name + "Reply";
    if(!fullfill_topic_type(topic_reply_name, _reply_type_name, serialized_reply_qos_content, topic_reply))
        return false;
    service.add_topic(topic_reply, RpcUtils::RPC_REPLY);

    if (!service.fully_discovered)
        return false;

    return true;
}

bool EnablerParticipant::request_service(
        ServiceDiscovered& service)
{
    if(!service_req_callback_)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to announce service " << service.service_name <<
                            " : service is unknown and service request callback not set.");
        return false;
    }

    char* _request_type_name = nullptr;
    char* serialized_request_qos_content = nullptr;
    char* _reply_type_name = nullptr;
    char* serialized_reply_qos_content = nullptr;


    if (!service_req_callback_(service.service_name.c_str(), _request_type_name, serialized_request_qos_content,
            _reply_type_name, serialized_reply_qos_content)) // TODO: allow the user not to provide QoS + handle fail case
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to announce service " << service.service_name << " : service type request failed.");
        return false;
    }

    return fullfill_service_type(_request_type_name, serialized_request_qos_content,
            _reply_type_name, serialized_reply_qos_content, service);
}

bool EnablerParticipant::request_action(
    ActionDiscovered& action)
{
    auto it = actions_.find(action.action_name);
    if (it != actions_.end())
    {
        // TODO what to do in this case? RN it is being overwritten
    }

    if(!action_req_callback_)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to announce action " << action.action_name <<
                            " : action is unknown and action request callback not set.");
        return false;
    }

    char* _goal_request_action_type = nullptr;
    char* _goal_reply_action_type = nullptr;
    char* _cancel_request_action_type = nullptr;
    char* _cancel_reply_action_type = nullptr;
    char* _result_request_action_type = nullptr;
    char* _result_reply_action_type = nullptr;
    char* _feedback_action_type = nullptr;
    char* _status_action_type = nullptr;
    char* _goal_request_action_serialized_qos = nullptr;
    char* _goal_reply_action_serialized_qos = nullptr;
    char* _cancel_request_action_serialized_qos = nullptr;
    char* _cancel_reply_action_serialized_qos = nullptr;
    char* _result_request_action_serialized_qos = nullptr;
    char* _result_reply_action_serialized_qos = nullptr;
    char* _feedback_action_serialized_qos = nullptr;
    char* _status_action_serialized_qos = nullptr;

    if (!action_req_callback_(action.action_name.c_str(),
            _goal_request_action_type,
            _goal_reply_action_type,
            _cancel_request_action_type,
            _cancel_reply_action_type,
            _result_request_action_type,
            _result_reply_action_type,
            _feedback_action_type,
            _status_action_type,
            _goal_request_action_serialized_qos,
            _goal_reply_action_serialized_qos,
            _cancel_request_action_serialized_qos,
            _cancel_reply_action_serialized_qos,
            _result_request_action_serialized_qos,
            _result_reply_action_serialized_qos,
            _feedback_action_serialized_qos,
            _status_action_serialized_qos))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to announce action " << action.action_name << " : action type request failed.");
        return false;
    }

    std::string goal_service_name = action.action_name + "send_goal";
    ServiceDiscovered goal_service(goal_service_name);
    if(!fullfill_service_type(
            _goal_request_action_type,
            _goal_request_action_serialized_qos,
            _goal_reply_action_type,
            _goal_reply_action_serialized_qos,
            goal_service))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to announce action " << action.action_name << " : goal service type not found.");
        return false;
    }
    action.goal = goal_service;

    std::string cancel_service_name = action.action_name + "cancel_goal";
    ServiceDiscovered cancel_service(cancel_service_name);
    if(!fullfill_service_type(
            _cancel_request_action_type,
            _cancel_request_action_serialized_qos,
            _cancel_reply_action_type,
            _cancel_reply_action_serialized_qos,
            cancel_service))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to announce action " << action.action_name << " : cancel service type not found.");
        return false;
    }
    action.cancel = cancel_service;

    std::string result_service_name = action.action_name + "get_result";
    ServiceDiscovered result_service(result_service_name);
    if(!fullfill_service_type(
            _result_request_action_type,
            _result_request_action_serialized_qos,
            _result_reply_action_type,
            _result_reply_action_serialized_qos,
            result_service))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to announce action " << action.action_name << " : result service type not found.");
        return false;
    }
    action.result = result_service;

    std::string feedback_topic_name = "rt/" + action.action_name + "feedback";
    DdsTopic feedback_topic;
    if(!fullfill_topic_type(
            feedback_topic_name,
            _feedback_action_type,
            _feedback_action_serialized_qos,
            feedback_topic))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to announce action " << action.action_name << " : feedback topic type not found.");
        return false;
    }
    action.feedback = feedback_topic;
    action.feedback_discovered = true;

    std::string status_topic_name = "rt/" + action.action_name + "status";
    DdsTopic status_topic;
    if(!fullfill_topic_type(
            status_topic_name,
            _status_action_type,
            _status_action_serialized_qos,
            status_topic))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to announce action " << action.action_name << " : status topic type not found.");
        return false;
    }
    action.status = status_topic;
    action.status_discovered = true;

    action.fully_discovered = true;

    return true;
}

std::shared_ptr<IReader> EnablerParticipant::lookup_reader_nts_(
        const std::string& topic_name,
        std::string& type_name) const
{
    for (const auto& reader : readers_)
    {
        if (reader.first.m_topic_name == topic_name)
        {
            type_name = reader.first.type_name;
            return reader.second;
        }
    }
    return nullptr;
}

std::shared_ptr<IReader> EnablerParticipant::lookup_reader_nts_(
        const std::string& topic_name) const
{
    std::string _;
    return lookup_reader_nts_(topic_name, _);
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
