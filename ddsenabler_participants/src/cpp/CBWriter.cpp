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

#include <ddsenabler_participants/serialization.hpp>
#include <ddsenabler_participants/RpcUtils.hpp>
#include <ddsenabler_participants/types/dynamic_types_collection/DynamicTypesCollection.hpp>

#include <ddsenabler_participants/CBWriter.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

using namespace eprosima::ddsenabler::participants::serialization;
using namespace eprosima::ddspipe::core::types;

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
            service.request_topic().topic_name().c_str(),
            service.reply_topic().topic_name().c_str(),
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

        action_callback_(
            action.action_name.c_str(),
            action.goal.request_topic().topic_name().c_str(),
            action.goal.reply_topic().topic_name().c_str(),
            action.cancel.request_topic().topic_name().c_str(),
            action.cancel.reply_topic().topic_name().c_str(),
            action.result.request_topic().topic_name().c_str(),
            action.result.reply_topic().topic_name().c_str(),
            action.feedback.topic_name().c_str(),
            goal_request_serialized_qos.c_str(),
            goal_reply_serialized_qos.c_str(),
            cancel_request_serialized_qos.c_str(),
            cancel_reply_serialized_qos.c_str(),
            result_request_serialized_qos.c_str(),
            result_reply_serialized_qos.c_str(),
            feedback_serialized_qos.c_str()
            );
    }
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
