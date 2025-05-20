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
 * @file CBHandler.cpp
 */

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include <cpp_utils/exception/InconsistencyException.hpp>

#include <ddsenabler_participants/serialization.hpp>
#include <ddsenabler_participants/RpcUtils.hpp>
#include <ddsenabler_participants/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsenabler_participants/CBHandler.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

using namespace eprosima::ddspipe::core::types;

CBHandler::CBHandler(
        const CBHandlerConfiguration& config,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool)
    : configuration_(config)
    , payload_pool_(payload_pool)
{
    EPROSIMA_LOG_INFO(DDSENABLER_CB_HANDLER,
            "Creating CB handler instance.");

    cb_writer_ = std::make_unique<CBWriter>();

    cb_writer_->set_is_UUID_active_callback(
        [this](const UUID& uuid) {
            return this->is_UUID_active(uuid);
        }
    );

    cb_writer_->set_erase_action_UUID_callback(
        [this](const UUID& uuid) {
            return this->erase_action_UUID(uuid);
        }
    );
}

CBHandler::~CBHandler()
{
    EPROSIMA_LOG_INFO(DDSENABLER_CB_HANDLER,
            "Destroying CB handler.");
}

void CBHandler::add_schema(
        const fastdds::dds::DynamicType::_ref_type& dyn_type,
        const fastdds::dds::xtypes::TypeIdentifier& type_id)
{
    std::lock_guard<std::mutex> lock(mtx_);

    assert(nullptr != dyn_type);

    const std::string& type_name = dyn_type->get_name().to_string();

    // Check if it exists already
    auto it = schemas_.find(type_name);
    if (it != schemas_.end())
    {
        return;
    }
    schemas_[type_name] = {type_id, dyn_type};

    // Add to schemas map
    EPROSIMA_LOG_INFO(DDSENABLER_CB_HANDLER,
            "Adding schema with name " << type_name << ".");

    write_schema_(dyn_type, type_id);
}

void CBHandler::add_topic(
        const DdsTopic& topic)
{
    cb_writer_->write_topic(topic);
}

void CBHandler::add_service(
        const RpcTopic& service)
{
    cb_writer_->write_service(service);
}

void CBHandler::add_action(
        const RpcUtils::RpcAction& action)
{
    cb_writer_->write_action(action);
}

void CBHandler::add_data(
        const DdsTopic& topic,
        RtpsPayloadData& data)
{
    CBMessage msg;
    fastdds::dds::DynamicType::_ref_type dyn_type;
    {
        std::lock_guard<std::mutex> lock(mtx_);

        EPROSIMA_LOG_INFO(DDSENABLER_CB_HANDLER,
                "Adding data in topic: " << topic << ".");

        auto it = schemas_.find(topic.type_name);
        if (it == schemas_.end())
        {
            EPROSIMA_LOG_WARNING(DDSENABLER_CB_HANDLER,
                    "Schema for type " << topic.type_name << " not available.");
            return;
        }
        dyn_type = it->second.second;

        msg.sequence_number = unique_sequence_number_++;
        msg.publish_time = data.source_timestamp;
        if (data.payload.length > 0)
        {
            msg.topic = topic;
            msg.instanceHandle = data.instanceHandle;
            msg.source_guid = data.source_guid;

            if (data.payload_owner != nullptr)
            {
                payload_pool_->get_payload(
                    data.payload,
                    msg.payload);

                msg.payload_owner = payload_pool_.get();
            }
            else
            {
                throw utils::InconsistencyException(STR_ENTRY << "Payload owner not found in data received.");
            }
        }
        else
        {
            throw utils::InconsistencyException(STR_ENTRY << "Received sample with no payload.");
        }
    }

    std::string rpc_name;
    RpcUtils::RpcType rpc_type = RpcUtils::get_rpc_name(topic.m_topic_name, rpc_name);
    switch (rpc_type)
    {
        case RpcUtils::RpcType::RPC_NONE:
            write_sample_(msg, dyn_type);
            break;

        // SERVICES
        case RpcUtils::RpcType::RPC_REQUEST:
        {
            received_requests_id_++;
            auto request_id = dynamic_cast<ddspipe::core::types::RpcPayloadData&>(data).sent_sequence_number.to64long();
            RequestInfo request_info(
                topic.m_topic_name,
                request_id);
            request_id_map_.emplace(received_requests_id_, request_info);
            write_request_(msg, dyn_type, received_requests_id_);
            break;
        }

        case RpcUtils::RpcType::RPC_REPLY:
        {
            auto request_id = dynamic_cast<ddspipe::core::types::RpcPayloadData&>(data).write_params.get_reference().related_sample_identity().sequence_number().to64long();
            write_reply_(msg, dyn_type, request_id);
            break;
        }

        // ACTIONS: CB AS CLIENT
        case RpcUtils::RpcType::ACTION_RESULT_REPLY:
        {
            auto action_id = dynamic_cast<ddspipe::core::types::RpcPayloadData&>(data).write_params.get_reference().related_sample_identity().sequence_number().to64long();
            UUID action_id_uuid;
            if (pop_action_request_UUID(action_id, action_id_uuid))
                write_action_result_(msg, dyn_type, action_id_uuid);
            break;
        }

        case RpcUtils::RpcType::ACTION_GOAL_REPLY:
        {
            // TODO: If it is accepted send directly the get_result_request, if it is not accepted send updated status. How to read the data without deserializing it?
            // TODO: all the send_goal_responses have an "accepted" parameter?
            auto action_id = dynamic_cast<ddspipe::core::types::RpcPayloadData&>(data).write_params.get_reference().related_sample_identity().sequence_number().to64long();
            UUID action_id_uuid;
            if (pop_action_request_UUID(action_id, action_id_uuid))
                write_action_goal_reply_(msg, dyn_type, action_id_uuid);
            break;
        }

        case RpcUtils::RpcType::ACTION_CANCEL_REPLY:
        {
            auto action_id = dynamic_cast<ddspipe::core::types::RpcPayloadData&>(data).write_params.get_reference().related_sample_identity().sequence_number().to64long();
            UUID action_id_uuid;
            if (pop_action_request_UUID(action_id, action_id_uuid))
                write_action_cancel_reply_(msg, dyn_type, action_id_uuid);
            break;
        }

        case RpcUtils::RpcType::ACTION_FEEDBACK:
        {
            // TODO strip the UUID from the type when adding the schema
            write_action_feedback_(msg, dyn_type);
            break;
        }

        case RpcUtils::RpcType::ACTION_STATUS:
        {
            write_action_status_(msg, dyn_type);
            break;
        }

        case RpcUtils::RpcType::ACTION_RESULT_REQUEST:
            // No need to do anything
            break;

        default:
            EPROSIMA_LOG_ERROR(DDSENABLER_CB_HANDLER,
                    "Unknown RPC type for topic " << topic.m_topic_name << ".");
            break;
    }
}

bool CBHandler::get_type_identifier(
        const std::string& type_name,
        fastdds::dds::xtypes::TypeIdentifier& type_identifier)
{
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = schemas_.find(type_name);
    if (it != schemas_.end())
    {
        type_identifier = it->second.first;
        return true;
    }

    if (!type_req_callback_)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_HANDLER,
                "Type request callback not set.");
        return false;
    }

    unsigned char* serialized_type;
    uint32_t serialized_type_size;
    type_req_callback_(type_name.c_str(), serialized_type, serialized_type_size);
    // TODO: handle fail case
    // TODO: free resources allocated by user (serialized_type), or redesign interaction

    return register_type_nts_(type_name, serialized_type, serialized_type_size, type_identifier);
}

bool CBHandler::get_serialized_data(
        const std::string& type_name,
        const std::string& json,
        Payload& payload)
{
    std::lock_guard<std::mutex> lock(mtx_);

    fastdds::dds::DynamicType::_ref_type dyn_type;
    auto it = schemas_.find(type_name);
    if (it == schemas_.end())
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_HANDLER,
                "Failed to deserialize data for type " << type_name << " : schema not available.");
        return false;
    }
    dyn_type = it->second.second;

    fastdds::dds::DynamicData::_ref_type dyn_data;
    if ((fastdds::dds::RETCODE_OK !=
            fastdds::dds::json_deserialize(json, dyn_type, fastdds::dds::DynamicDataJsonFormat::EPROSIMA,
            dyn_data)) || !dyn_data)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_HANDLER,
                "Failed to deserialize data for type " << type_name << " : json deserialization failed.");
        return false;
    }

    // TODO: double chekc XCDR2 is the right choice -> only supposed to work against fastdds 3, and appendable only working with XCDR2
    fastdds::dds::DynamicPubSubType pubsub_type(dyn_type);
    uint32_t payload_size = pubsub_type.calculate_serialized_size(&dyn_data,
                    fastdds::dds::DataRepresentationId::XCDR2_DATA_REPRESENTATION);

    if (!payload_pool_->get_payload(payload_size, payload))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_HANDLER,
                "Failed to deserialize data for type " << type_name << " : get_payload failed.");
        return false;
    }

    if (!pubsub_type.serialize(&dyn_data, payload, fastdds::dds::DataRepresentationId::XCDR2_DATA_REPRESENTATION))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_HANDLER,
                "Failed to deserialize data for type " << type_name << " : payload serialization failed.");
        return false;
    }

    return true;
}

void CBHandler::write_schema_(
        const fastdds::dds::DynamicType::_ref_type& dyn_type,
        const fastdds::dds::xtypes::TypeIdentifier& type_id)
{
    cb_writer_->write_schema(dyn_type, type_id);
}

void CBHandler::write_topic_(
        const DdsTopic& topic)
{
    cb_writer_->write_topic(topic);
}

void CBHandler::write_sample_(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
    cb_writer_->write_data(msg, dyn_type);
}

void CBHandler::write_reply_(
    const CBMessage& msg,
    const fastdds::dds::DynamicType::_ref_type& dyn_type,
    const uint64_t request_id)
{
    cb_writer_->write_reply(msg, dyn_type, request_id);
}

void CBHandler::write_request_(
    const CBMessage& msg,
    const fastdds::dds::DynamicType::_ref_type& dyn_type,
    const uint64_t request_id)
{
    cb_writer_->write_request(msg, dyn_type, request_id);
}

void CBHandler::write_action_result_(
    const CBMessage& msg,
    const fastdds::dds::DynamicType::_ref_type& dyn_type,
    const UUID& action_id)
{
    cb_writer_->write_action_result(msg, dyn_type, action_id);
}

void CBHandler::write_action_feedback_(
    const CBMessage& msg,
    const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
    cb_writer_->write_action_feedback(msg, dyn_type);
}

void CBHandler::write_action_goal_reply_(
    const CBMessage& msg,
    const fastdds::dds::DynamicType::_ref_type& dyn_type,
    const UUID& action_id)
{
    cb_writer_->write_action_goal_reply(msg, dyn_type, action_id);
}

void CBHandler::write_action_cancel_reply_(
    const CBMessage& msg,
    const fastdds::dds::DynamicType::_ref_type& dyn_type,
    const UUID& action_id)
{
    cb_writer_->write_action_cancel_reply(msg, dyn_type, action_id);
}

void CBHandler::write_action_status_(
    const CBMessage& msg,
    const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
    cb_writer_->write_action_status(msg, dyn_type);
}

bool CBHandler::register_type_nts_(
        const std::string& type_name,
        const unsigned char* serialized_type,
        uint32_t serialized_type_size,
        fastdds::dds::xtypes::TypeIdentifier& type_identifier)
{
    DynamicTypesCollection dynamic_types;
    serialization::deserialize_dynamic_types(serialized_type, serialized_type_size, dynamic_types); // TODO: handle fail case

    std::string _type_name;
    fastdds::dds::xtypes::TypeIdentifier _type_identifier;
    fastdds::dds::xtypes::TypeObject _type_object;

    // Deserialize and register all dependencies and main type (last one in collection)
    for (DynamicType& dynamic_type : dynamic_types.dynamic_types())
    {
        if (!serialization::deserialize_dynamic_type(dynamic_type, _type_name, _type_identifier, _type_object))
        {
            EPROSIMA_LOG_ERROR(DDSENABLER_CB_HANDLER,
                    "Failed to deserialize " << dynamic_type.type_name() << " DynamicType.");
            return false;
        }

        // Create a TypeIdentifierPair to use in register_type_identifier
        fastdds::dds::xtypes::TypeIdentifierPair type_identifiers;
        type_identifiers.type_identifier1(_type_identifier);

        // Register in factory
        if (fastdds::dds::RETCODE_OK !=
                fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(
                    _type_object, type_identifiers))
        {
            EPROSIMA_LOG_ERROR(DDSENABLER_CB_HANDLER,
                    "Failed to register " << dynamic_type.type_name() << " DynamicType.");
            // TODO: check if trying to register a type for the second time would return OK or not
            return false;
        }
    }

    if (_type_name != type_name)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_HANDLER,
                "Unexpected dynamic types collection format: " << type_name << " expected to be last item, found " << _type_name <<
                            " instead.");
        return false;
    }

    fastdds::dds::DynamicType::_ref_type dyn_type =
            fastdds::dds::DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(_type_object)
                    ->build();
    if (!dyn_type)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_HANDLER,
                "Failed to create Dynamic Type " << type_name);
        return false;
    }

    // Add to schemas map
    EPROSIMA_LOG_INFO(DDSENABLER_CB_HANDLER,
            "Adding schema with name " << type_name << ".");
    schemas_[_type_name] = {_type_identifier, dyn_type};

    type_identifier = _type_identifier;
    return true;

    // TODO: analyze case where dependencies are not added to schemas map, so if afterwards the user tries to publish
    // in one of the dependencies type, we will try to register the same type again -> will it fail? if not, will it
    // skip registration but still return internal TypeIdentifier?
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
