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
 * @file CBWriter.cpp
 */

 #include <nlohmann/json.hpp>

#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/utils.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/rtps/common/Types.hpp>

#include <ddsenabler_participants/CBWriter.hpp>
#include <ddsenabler_participants/serialization.hpp>
#include <ddsenabler_participants/RpcUtils.hpp>
#include <ddsenabler_participants/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsenabler_participants/CBHandler.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

using namespace eprosima::ddsenabler::participants::serialization;
using namespace eprosima::ddspipe::core::types;


UUID json_to_uuid(const nlohmann::json& json)
{
    UUID uuid;
    if (!json.contains("uuid") || !json["uuid"].is_array() || json["uuid"].size() != sizeof(UUID))

    {
        throw std::invalid_argument("Invalid UUID format in JSON");
    }

    for (size_t i = 0; i < sizeof(UUID); ++i)
    {
        uuid[i] = json["uuid"][i].get<uint8_t>();
    }

    return uuid;
}

void CBWriter::write_schema(
        const fastdds::dds::DynamicType::_ref_type& dyn_type,
        const fastdds::dds::xtypes::TypeIdentifier& type_id)
{
    assert(nullptr != dyn_type);

    const std::string& type_name = dyn_type->get_name().to_string();

    //Schema has not been registered
    EPROSIMA_LOG_INFO(DDSENABLER_CB_WRITER,
            "Writing schema: " << type_name << ".");

    std::stringstream ss_idl;
    auto ret = fastdds::dds::idl_serialize(dyn_type, ss_idl);
    if (ret != fastdds::dds::RETCODE_OK)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Failed to serialize DynamicType to idl for type with name: " << type_name);
        return;
    }

    DynamicTypesCollection types_collection;
    if (!serialize_dynamic_type(type_name, type_id, types_collection))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Failed to serialize dynamic types collection: " << type_name);
        return;
    }

    std::unique_ptr<fastdds::rtps::SerializedPayload_t> types_collection_payload = serialize_dynamic_types(
        types_collection);
    if (nullptr == types_collection_payload)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Failed to serialize dynamic types collection: " << type_name);
        return;
    }

    std::stringstream ss_data_holder;
    ss_data_holder << std::setw(4);
    if (fastdds::dds::RETCODE_OK !=
            fastdds::dds::json_serialize(fastdds::dds::DynamicDataFactory::get_instance()->create_data(dyn_type),
            fastdds::dds::DynamicDataJsonFormat::EPROSIMA, ss_data_holder))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to generate data placeholder for type " << type_name << ".");
        return;
    }

    //STORE SCHEMA
    if (type_callback_)
    {
        type_callback_(
            type_name.c_str(),
            ss_idl.str().c_str(),
            (unsigned char*)types_collection_payload->data,
            types_collection_payload->length,
            ss_data_holder.str().c_str()
            );
    }
}

void CBWriter::write_topic(
        const DdsTopic& topic)
{
    EPROSIMA_LOG_INFO(DDSENABLER_CB_WRITER,
            "Writting topic: " << topic.topic_name() << ".");

    if (topic_callback_)
    {
        std::string serialized_qos = serialize_qos(topic.topic_qos);
        topic_callback_(
            topic.topic_name().c_str(),
            topic.type_name.c_str(),
            serialized_qos.c_str()
            );
    }
}

void CBWriter::write_data(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
    std::shared_ptr<void> json_ptr = prepare_json_data(msg, dyn_type);
    if (nullptr == json_ptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to generate JSON data for topic " << msg.topic.topic_name() << ".");
        return;
    }
    std::shared_ptr<nlohmann::json> json_output = std::static_pointer_cast<nlohmann::json>(json_ptr);

    //STORE DATA
    if (data_callback_)
    {
        data_callback_(
            msg.topic.topic_name().c_str(),
            json_output->dump(4).c_str(),
            msg.publish_time.to_ns()
            );
    }
}

void CBWriter::write_service(
    const ddspipe::core::types::RpcTopic& service)
{
    EPROSIMA_LOG_INFO(DDSENABLER_CB_WRITER,
            "Writting service: " << service.service_name() << ".");

    if (service_callback_)
    {
        std::string request_serialized_qos = serialize_qos(service.request_topic().topic_qos);
        std::string reply_serialized_qos = serialize_qos(service.reply_topic().topic_qos);
        service_callback_(
            service.service_name().c_str(),
            service.request_topic().type_name.c_str(),
            service.reply_topic().type_name.c_str(),
            request_serialized_qos.c_str(),
            reply_serialized_qos.c_str()
            );
    }
}

void CBWriter::write_reply(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type,
        const uint64_t request_id)
{
    std::shared_ptr<void> json_ptr = prepare_json_data(msg, dyn_type);
    if (nullptr == json_ptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to generate JSON data for topic " << msg.topic.topic_name() << ".");
        return;
    }
    std::shared_ptr<nlohmann::json> json_output = std::static_pointer_cast<nlohmann::json>(json_ptr);

    // Get the service name
    std::string service_name;
    if(RpcUtils::RpcType::RPC_REPLY != RpcUtils::get_rpc_name(msg.topic.topic_name(), service_name))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Wrong topic name for service reply: " << msg.topic.topic_name() << ".");
        return;
    }

    //STORE DATA
    if (reply_callback_)
    {
        reply_callback_(
            service_name.c_str(),
            json_output->dump(4).c_str(),
            request_id,
            msg.publish_time.to_ns()
            );
    }
}

void CBWriter::write_request(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type,
        const uint64_t request_id)
{
    std::shared_ptr<void> json_ptr = prepare_json_data(msg, dyn_type);
    if (nullptr == json_ptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to generate JSON data for topic " << msg.topic.topic_name() << ".");
        return;
    }
    std::shared_ptr<nlohmann::json> json_output = std::static_pointer_cast<nlohmann::json>(json_ptr);

    // Get the service name
    std::string service_name;
    if(RpcUtils::RpcType::RPC_REQUEST != RpcUtils::get_rpc_name(msg.topic.topic_name(), service_name))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Wrong topic name for service request: " << msg.topic.topic_name() << ".");
        return;
    }

    //STORE DATA
    if (request_callback_)
    {
        request_callback_(
            service_name.c_str(),
            json_output->dump(4).c_str(),
            request_id,
            msg.publish_time.to_ns()
            );
    }
}

void CBWriter::write_action(
        const RpcUtils::RpcAction& action)
{
    EPROSIMA_LOG_INFO(DDSENABLER_CB_WRITER,
            "Writting action: " << action.action_name << ".");

    if (action_callback_)
    {
        std::string goal_request_serialized_qos = serialize_qos(action.goal.request_topic().topic_qos);
        std::string goal_reply_serialized_qos = serialize_qos(action.goal.reply_topic().topic_qos);
        std::string cancel_request_serialized_qos = serialize_qos(action.cancel.request_topic().topic_qos);
        std::string cancel_reply_serialized_qos = serialize_qos(action.cancel.reply_topic().topic_qos);
        std::string result_request_serialized_qos = serialize_qos(action.result.request_topic().topic_qos);
        std::string result_reply_serialized_qos = serialize_qos(action.result.reply_topic().topic_qos);
        std::string feedback_serialized_qos = serialize_qos(action.feedback.topic_qos);
        std::string status_serialized_qos = serialize_qos(action.status.topic_qos);

        action_callback_(
            action.action_name.c_str(),
            action.goal.request_topic().type_name.c_str(),
            action.goal.reply_topic().type_name.c_str(),
            action.cancel.request_topic().type_name.c_str(),
            action.cancel.reply_topic().type_name.c_str(),
            action.result.request_topic().type_name.c_str(),
            action.result.reply_topic().type_name.c_str(),
            action.feedback.type_name.c_str(),
            action.status.type_name.c_str(),
            goal_request_serialized_qos.c_str(),
            goal_reply_serialized_qos.c_str(),
            cancel_request_serialized_qos.c_str(),
            cancel_reply_serialized_qos.c_str(),
            result_request_serialized_qos.c_str(),
            result_reply_serialized_qos.c_str(),
            feedback_serialized_qos.c_str(),
            status_serialized_qos.c_str()
            );
    }
}

void CBWriter::write_action_result(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type,
        const UUID& action_id)
{
    std::shared_ptr<void> json_ptr = prepare_json_data(msg, dyn_type);
    if (nullptr == json_ptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to generate JSON data for topic " << msg.topic.topic_name() << ".");
        return;
    }
    std::shared_ptr<nlohmann::json> json_output = std::static_pointer_cast<nlohmann::json>(json_ptr);

    // Get the action name
    std::string action_name;
    if(RpcUtils::RpcType::ACTION_RESULT_REPLY != RpcUtils::get_rpc_name(msg.topic.topic_name(), action_name))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Wrong topic name for action result: " << msg.topic.topic_name() << ".");
        return;
    }

    //STORE DATA
    if (action_result_callback_)
    {
        action_result_callback_(
            action_name.c_str(),
            json_output->dump(4).c_str(),
            action_id,
            msg.publish_time.to_ns()
            );
    }
}

void CBWriter::write_action_feedback(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
    std::shared_ptr<void> json_ptr = prepare_json_data(msg, dyn_type);
    if (nullptr == json_ptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to generate JSON data for topic " << msg.topic.topic_name() << ".");
        return;
    }
    std::shared_ptr<nlohmann::json> json_output = std::static_pointer_cast<nlohmann::json>(json_ptr);

    std::cout << "Feedback JSON: " << (*json_output).dump(4) << std::endl;

    // Get the action name
    std::string action_name;
    RpcUtils::get_rpc_name(msg.topic.topic_name(), action_name);

    // TODO check what we want to have in the json now that the type is not the same as the one in the action
    std::stringstream instanceHandle;
    instanceHandle << msg.instanceHandle;
    if (action_feedback_callback_)
    {
        action_feedback_callback_(
            action_name.c_str(),
            (*json_output)[msg.topic.topic_name()]["data"][instanceHandle.str()]["feedback"].dump(4).c_str(),
            json_to_uuid((*json_output)[msg.topic.topic_name()]["data"][instanceHandle.str()]["goal_id"]),
            msg.publish_time.to_ns()
            );
    }
}

void CBWriter::write_action_goal_reply(
    const CBMessage& msg,
    const fastdds::dds::DynamicType::_ref_type& dyn_type,
    const UUID& action_id)
{
    std::string action_name;
    RpcUtils::get_rpc_name(msg.topic.topic_name(), action_name);

    std::shared_ptr<void> json_ptr = prepare_json_data(msg, dyn_type);
    if (nullptr == json_ptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to generate JSON data for topic " << msg.topic.topic_name() << ".");
        return;
    }
    std::shared_ptr<nlohmann::json> json_output = std::static_pointer_cast<nlohmann::json>(json_ptr);

    std::string status_message = "Action goal accepted";
    ddsenabler::participants::STATUS_CODE status_code = ddsenabler::participants::STATUS_CODE::STATUS_ACCEPTED;
    std::stringstream instanceHandle;
    instanceHandle << msg.instanceHandle;
    if (!(*json_output)[msg.topic.topic_name()]["data"][instanceHandle.str()]["accepted"])
    {
        status_message = "Action goal rejected";
        status_code = ddsenabler::participants::STATUS_CODE::STATUS_REJECTED;
    }
    if (action_status_callback_)
    {
        action_status_callback_(
            action_name.c_str(),
            action_id,
            status_code,
            status_message.c_str(),
            msg.publish_time.to_ns()
            );
    }

    if (action_send_get_result_request_callback_ && !action_send_get_result_request_callback_(
            action_name.c_str(),
            action_id))
    {
        status_message = "Action goal rejected";
        status_code = ddsenabler::participants::STATUS_CODE::STATUS_ABORTED;

        if (action_status_callback_)
        {
            action_status_callback_(
                action_name.c_str(),
                action_id,
                status_code,
                status_message.c_str(),
                msg.publish_time.to_ns()
                );
        }
    }
}

void CBWriter::write_action_cancel_reply(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type,
        const UUID& action_id)
{
    ddsenabler::participants::STATUS_CODE status_code = ddsenabler::participants::STATUS_CODE::STATUS_CANCELED;
    std::string action_name;
    RpcUtils::get_rpc_name(msg.topic.topic_name(), action_name);

    std::shared_ptr<void> json_ptr = prepare_json_data(msg, dyn_type);
    if (nullptr == json_ptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to generate JSON data for topic " << msg.topic.topic_name() << ".");
        return;
    }
    std::shared_ptr<nlohmann::json> json_output = std::static_pointer_cast<nlohmann::json>(json_ptr);

    std::stringstream instanceHandle;
    instanceHandle << msg.instanceHandle;

    std::string status_message;
    int return_code = (*json_output)[msg.topic.topic_name()]["data"][instanceHandle.str()]["return_code"];
    switch (return_code)
    {
        case 0:
            status_message = "Action cancelled successfully";
            status_code = ddsenabler::participants::STATUS_CODE::STATUS_CANCELED;
            break;
        case 1:
            status_message = "Action cancel request rejected";
            status_code = ddsenabler::participants::STATUS_CODE::STATUS_REJECTED;
            break;
        case 2:
            status_message = "Action cancel request unknown goal ID";
            status_code = ddsenabler::participants::STATUS_CODE::STATUS_CANCEL_REQUEST_FAILED;
            break;
        case 3:
            status_message = "Action cancel request rejected as goal was already terminated";
            status_code = ddsenabler::participants::STATUS_CODE::STATUS_CANCEL_REQUEST_FAILED;
            break;
        default:
            status_message = "Action cancel request unknown code";
            status_code = ddsenabler::participants::STATUS_CODE::STATUS_UNKNOWN;
            break;
    }

    const auto& goals = (*json_output)[msg.topic.topic_name()]["data"][instanceHandle.str()]["goals_canceling"];
    for (const auto& goal : goals)
    {
        UUID msg_action_id = json_to_uuid(goal["goal_id"]);
        if(msg_action_id != action_id)
            continue;

        if (action_status_callback_)
        {
            action_status_callback_(
                action_name.c_str(),
                action_id,
                status_code,
                status_message.c_str(),
                msg.publish_time.to_ns()
                );
        }
    }
}

void CBWriter::write_action_status(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
    std::shared_ptr<void> json_ptr = prepare_json_data(msg, dyn_type);
    if (nullptr == json_ptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to generate JSON data for topic " << msg.topic.topic_name() << ".");
        return;
    }
    std::shared_ptr<nlohmann::json> json_output = std::static_pointer_cast<nlohmann::json>(json_ptr);

    // Get the action name
    std::string action_name;
    RpcUtils::get_rpc_name(msg.topic.topic_name(), action_name);

    std::stringstream instanceHandle;
    instanceHandle << msg.instanceHandle;

    const auto& list = (*json_output)[msg.topic.topic_name()]["data"][instanceHandle.str()]["status_list"];
    for (const auto& status : list)
    {
        UUID uuid = json_to_uuid(status["goal_info"]["goal_id"]);
        if(is_UUID_active_callback_ && !is_UUID_active_callback_(uuid))
            continue;

        ddsenabler::participants::STATUS_CODE status_code(status["status"]);
        std::string status_message;

        switch (status_code)
        {
            case ddsenabler::participants::STATUS_CODE::STATUS_UNKNOWN:
                status_message = "The status has not been properly set";
                break;

            case ddsenabler::participants::STATUS_CODE::STATUS_ACCEPTED:
                status_message = "The goal has been accepted and is awaiting execution";
                break;

            case ddsenabler::participants::STATUS_CODE::STATUS_EXECUTING:
                status_message = "The goal is currently being executed by the action server";
                break;

            case ddsenabler::participants::STATUS_CODE::STATUS_CANCELING:
                status_message = "The client has requested that the goal be canceled and the action server has accepted the cancel request";
                break;

            case ddsenabler::participants::STATUS_CODE::STATUS_SUCCEEDED:
                status_message = "The goal was achieved successfully by the action server";
                break;

            case ddsenabler::participants::STATUS_CODE::STATUS_CANCELED:
                status_message = "The goal was canceled after an external request from an action client";
                break;

            case ddsenabler::participants::STATUS_CODE::STATUS_ABORTED:
                if (erase_action_UUID_callback_)
                    erase_action_UUID_callback_(uuid);
                status_message = "The goal was terminated by the action server without an external request";
                break;
            case ddsenabler::participants::STATUS_CODE::STATUS_REJECTED:
                if (erase_action_UUID_callback_)
                    erase_action_UUID_callback_(uuid);
                status_message = "The goal was rejected by the action server, it will not be executed";
                break;
            default:
                status_message = "Unknown status code";
                break;
        }

        if (action_status_callback_)
        {
            action_status_callback_(
                action_name.c_str(),
                uuid,
                status_code,
                status_message.c_str(),
                msg.publish_time.to_ns()
                );
        }
    }
}

void CBWriter::write_action_request(
    const CBMessage& msg,
    const fastdds::dds::DynamicType::_ref_type& dyn_type,
    const uint64_t request_id)
{
    std::shared_ptr<void> json_ptr = prepare_json_data(msg, dyn_type);
    if (nullptr == json_ptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to generate JSON data for topic " << msg.topic.topic_name() << ".");
        return;
    }
    std::shared_ptr<nlohmann::json> json_output = std::static_pointer_cast<nlohmann::json>(json_ptr);

    // Get the action name
    std::string action_name;
    RpcUtils::RpcType rpc_type = RpcUtils::get_rpc_name(msg.topic.topic_name(), action_name);

    std::stringstream instanceHandle;
    instanceHandle << msg.instanceHandle;
    // TODO do not read directly from json without failure handling
    if(RpcUtils::RpcType::ACTION_GOAL_REQUEST == rpc_type)
    {
        bool accepted;
        if (action_goal_request_notification_callback_)
            accepted = action_goal_request_notification_callback_(
                    action_name.c_str(),
                    json_output->dump(4).c_str(),
                    json_to_uuid((*json_output)[msg.topic.topic_name()]["data"][instanceHandle.str()]["goal_id"]),
                    msg.publish_time.to_ns()
                    );

        action_send_send_goal_reply_callback_(
            action_name.c_str(),
            request_id,
            accepted);

        return;
    }
}

UUID CBWriter::uuid_from_request_json(
    const CBMessage& msg,
    const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
    std::shared_ptr<void> json_ptr = prepare_json_data(msg, dyn_type);
    if (nullptr == json_ptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to generate JSON data for topic " << msg.topic.topic_name() << ".");
        return UUID();
    }
    std::shared_ptr<nlohmann::json> json_output = std::static_pointer_cast<nlohmann::json>(json_ptr);

    std::stringstream instanceHandle;
    instanceHandle << msg.instanceHandle;
    return json_to_uuid((*json_output)[msg.topic.topic_name()]["data"][instanceHandle.str()]["goal_id"]);
}

std::shared_ptr<void> CBWriter::prepare_json_data(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
    assert(nullptr != dyn_type);

    EPROSIMA_LOG_INFO(DDSENABLER_CB_WRITER,
            "Processing message from topic: " << msg.topic.topic_name() << ".");

    // Get the data as JSON
    fastdds::dds::DynamicData::_ref_type dyn_data = get_dynamic_data_(msg, dyn_type);

    if (nullptr == dyn_data)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to get DynamicData from topic " << msg.topic.topic_name() << ".");
        return nullptr;
    }

    std::stringstream ss_dyn_data;
    ss_dyn_data << std::setw(4);
    if (fastdds::dds::RETCODE_OK !=
            fastdds::dds::json_serialize(dyn_data, fastdds::dds::DynamicDataJsonFormat::EPROSIMA, ss_dyn_data))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to serialize data of topic " << msg.topic.topic_name() << " into JSON format.");
        return nullptr;
    }

    // Create the base JSON structure
    nlohmann::json json_output;

    std::stringstream ss_source_guid_prefix;
    ss_source_guid_prefix << msg.source_guid.guid_prefix();
    json_output["id"] = ss_source_guid_prefix.str();
    json_output["type"] = "fastdds";
    json_output[msg.topic.topic_name()] = {
        {"type", msg.topic.type_name},
        {"data", nlohmann::json::object()}
    };

    std::stringstream ss_instanceHandle;
    ss_instanceHandle << msg.instanceHandle;
    nlohmann::json parsed_dyn_data = nlohmann::json::parse(ss_dyn_data.str());
    json_output[msg.topic.topic_name()]["data"][ss_instanceHandle.str()] = parsed_dyn_data;

    if (json_output.empty())
    {
        // TODO: handle error
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to generate JSON data for topic " << msg.topic.topic_name() << ".");
    }

    return std::make_shared<nlohmann::json>(json_output);
}

fastdds::dds::DynamicData::_ref_type CBWriter::get_dynamic_data_(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type) noexcept
{
    // TODO fast this should not be done, but dyn types API is like it is.
    auto& data_no_const = const_cast<eprosima::fastdds::rtps::SerializedPayload_t&>(msg.payload);

    // Create PubSub Type
    // TODO: avoid creating this object each time -> store in a map
    fastdds::dds::DynamicPubSubType pubsub_type(dyn_type);
    fastdds::dds::DynamicData::_ref_type dyn_data(
        fastdds::dds::DynamicDataFactory::get_instance()->create_data(dyn_type));

    pubsub_type.deserialize(data_no_const, &dyn_data);

    return dyn_data;
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
