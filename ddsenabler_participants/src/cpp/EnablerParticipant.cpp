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
    auto it = services_.find(service_name);
    if (it == services_.end())
    {
        ServiceDiscovered service;
        services_[service_name] = service;
        it = services_.find(service_name);
    }

    if(it->second.fully_discovered)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Service " << service_name << " already fully discovered.");
        return false;
    }

    if(rpc_type == RpcUtils::RpcType::RPC_REQUEST)
    {
        if(it->second.request_discovered)
        {
            EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                    "Service " << service_name << " already discovered.");
            return false;
        }
        it->second.topic_request = topic;
        it->second.request_discovered = true;
    }
    else
    {
        if(it->second.reply_discovered)
        {
            EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                    "Service " << service_name << " already discovered.");
            return false;
        }
        it->second.topic_reply = topic;
        it->second.reply_discovered = true;
    }

    if(it->second.request_discovered && it->second.reply_discovered)
    {
        it->second.fully_discovered = true;
        EPROSIMA_LOG_INFO(DDSENABLER_ENABLER_PARTICIPANT,
                "Service " << service_name << " fully discovered.");
        return true;
    }

    return false;
}

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
        std::string service_name;
        RpcUtils::RpcType rpc_type = RpcUtils::get_service_name(dds_topic.m_topic_name, service_name);
        if (RpcUtils::RpcType::RPC_NONE != rpc_type)
        {
            reader = std::make_shared<InternalRpcReader>(id(), dds_topic);
            if(service_discovered_nts(service_name, dds_topic, rpc_type))
            {
                std::static_pointer_cast<CBHandler>(schema_handler_)->add_service(dds_topic);
            }
        }
        else
        {
            reader = std::make_shared<InternalReader>(id());
            std::static_pointer_cast<CBHandler>(schema_handler_)->add_topic(dds_topic);
        }
        readers_[dds_topic] = reader;
    }
    cv_.notify_one();
    return reader;
}

bool EnablerParticipant::publish(
        const std::string& topic_name,
        const std::string& json)
{
    std::unique_lock<std::mutex> lck(mtx_);

    std::string type_name;
    auto reader = std::dynamic_pointer_cast<InternalRpcReader>(lookup_reader_nts_(topic_name, type_name));

    if (nullptr == reader)
    {

        DdsTopic topic;
        if(!request_topic(topic_name, topic))
        {
            return false;
        }
        this->discovery_database_->add_endpoint(rtps::CommonParticipant::simulate_endpoint(topic, this->id()));

        // Wait for reader to be created from discovery thread
        cv_.wait(lck, [&]
                {
                    return nullptr != (reader = std::dynamic_pointer_cast<InternalRpcReader>(lookup_reader_nts_(topic_name)));
                });                                                                          // TODO: handle case when stopped before processing queue item

        // (Optionally) wait for writer created in DDS participant to match with external readers, to avoid losing this
        // message when not using transient durability
        std::this_thread::sleep_for(std::chrono::milliseconds(std::static_pointer_cast<EnablerParticipantConfiguration>(
                    configuration_)->initial_publish_wait));
    }

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

    DdsTopic topic;

    // TODO: Check whether the server exists and not only the topic
    if(!this->discovery_database_->topic_exists(topic_name, topic))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to publish in " << topic_name << " : topic does not exist.");
        return false;
    }
    // TODO: Use topic_req_callback_?
    // if(!request_topic(reply_name, topic))
    // {
    //     return false;
    // }

    std::string type_name;
    auto reader = std::dynamic_pointer_cast<InternalRpcReader>(lookup_reader_nts_(topic.m_topic_name, type_name));

    if (nullptr == reader)
    {
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

bool EnablerParticipant::announce_service(
    const std::string& service_name)
{
    std::string request_name = "rq/" + service_name + "Request";

    std::unique_lock<std::mutex> lck(mtx_);

    std::string type_name;
    auto reader = std::dynamic_pointer_cast<InternalRpcReader>(lookup_reader_nts_(request_name, type_name));

    if (nullptr == reader)
    {

        DdsTopic topic;
        // if(!request_topic(request_name, topic))
        // {
        //     return false;
        // }
        // TODO remove this and change by the request
        if(!this->discovery_database_->topic_exists(request_name, topic))
        {
            std::cout << "Topic not discovered yet, test not possible" << std::endl;
            return false;
        }
        auto request_edp = rtps::CommonParticipant::simulate_endpoint(topic, this->id());
        this->discovery_database_->add_endpoint(request_edp);
        server_endpoint_.emplace(service_name, request_edp);

        // Wait for reader to be created from discovery thread
        cv_.wait(lck, [&]
                {
                    return nullptr != (reader = std::dynamic_pointer_cast<InternalRpcReader>(lookup_reader_nts_(request_name)));
                });                                                                          // TODO: handle case when stopped before processing queue item

        // (Optionally) wait for writer created in DDS participant to match with external readers, to avoid losing this
        // message when not using transient durability
        std::this_thread::sleep_for(std::chrono::milliseconds(std::static_pointer_cast<EnablerParticipantConfiguration>(
                    configuration_)->initial_publish_wait));

        return true;
    }
    else
    {
        // TODO What if there is a server already running in ROS2, currently the announce will fail
        std::cout << "SERVICE ALREADY ANNOUNCED" << std::endl;
    }
    return false;
}

bool EnablerParticipant::revoke_service(
    const std::string& service_name)
{
    std::string request_name = "rq/" + service_name + "Request";

    std::unique_lock<std::mutex> lck(mtx_);

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
    std::string type_name;
    TopicQoS qos;
    fastdds::dds::xtypes::TypeIdentifier type_identifier;
    topic_req_callback_(topic_name.c_str(), _type_name, serialized_qos_content); // TODO: allow the user not to provide QoS + handle fail case
    type_name = std::string(_type_name); // TODO: free resources allocated by user, or redesign interaction (same for serialized_qos_content)
    std::string serialized_qos(serialized_qos_content);

    qos = serialization::deserialize_qos(serialized_qos); // TODO: handle fail case (try-catch?)

    if (!std::static_pointer_cast<CBHandler>(schema_handler_)->get_type_identifier(type_name, type_identifier))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_ENABLER_PARTICIPANT,
                "Failed to publish data in topic " << topic_name << " : type identifier not found.");
        return false;
    }

    topic.m_topic_name = topic_name;
    topic.type_name = type_name;
    topic.topic_qos = qos;
    topic.type_identifiers.type_identifier1(type_identifier);

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
