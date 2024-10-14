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

#include <cpp_utils/utils.hpp>

#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include <ddsenabler_participants/CBWriter.hpp>
#include <ddsenabler_participants/DynamicTypesSerializer.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {


void CBWriter::set_data_callback(
        DdsNotification callback)
{
    data_callback_ = callback;
}

void CBWriter::set_type_callback(
        DdsTypeNotification callback)
{
    type_callback_ = callback;
}

void CBWriter::write_data(
        const CBMessage& msg,
        KnownType& known_type)
{
    std::lock_guard<std::mutex> lock(mutex_);

    EPROSIMA_LOG_INFO(DDSENABLER_CB_WRITER,
            "Writing message from topic: " << msg.topic.topic_name() << ".");

    // Get the data as JSON
    fastdds::dds::DynamicData::_ref_type dyn_data = get_dynamic_data(msg, known_type.dyn_type_);

    std::stringstream ss_dyn_data;
    ss_dyn_data << std::setw(4);
    if (fastdds::dds::RETCODE_OK !=
            fastdds::dds::json_serialize(dyn_data, fastdds::dds::DynamicDataJsonFormat::EPROSIMA, ss_dyn_data))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Not able to serialize data of topic " << msg.topic.topic_name() << " into JSON format.");
        return;
    }

    // Create the base JSON structure
    nlohmann::ordered_json json_output;

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
    nlohmann::ordered_json parsed_dyn_data = nlohmann::json::parse(ss_dyn_data.str());
    json_output[msg.topic.topic_name()]["data"][ss_instanceHandle.str()] = parsed_dyn_data;

    //STORE DATA
    if (data_callback_)
    {
        data_callback_(
            msg.topic.type_name.c_str(),
            msg.topic.topic_name().c_str(),
            json_output.dump(4).c_str(),
            msg.publish_time.to_ns()
            );
    }
    else
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                "Missing DdsNotification callback.");
    }
}

void CBWriter::write_schema(
        const CBMessage& msg,
        KnownType& known_type)
{
    std::lock_guard<std::mutex> lock(mutex_);
    const std::string topic_name = msg.topic.topic_name();
    const std::string type_name = msg.topic.type_name;

    DynamicTypesCollection dynamic_types;

    // Store dynamic type in dynamic_types collection
    if (DynamicTypesSerializer::store_dynamic_type(type_name, known_type.type_id_, dynamic_types))
    {
        // Serialize dynamic types collection
        const auto serialized_dynamic_types = DynamicTypesSerializer::serialize_dynamic_types(dynamic_types);

        auto serialized_data = reinterpret_cast<std::byte*>(serialized_dynamic_types->data);
        auto dataSize = serialized_dynamic_types->length;
        auto serialized_data_char = reinterpret_cast<const char*>(serialized_dynamic_types->data);

        //STORE SCHEMA
        if (type_callback_)
        {
            type_callback_(
                type_name.c_str(),
                topic_name.c_str(),
                serialized_data_char
                );

            known_type.is_written_ = true;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DDSENABLER_CB_WRITER,
                    "Missing DdsTypeNotification callback.");
        }
    }
}

fastdds::dds::DynamicData::_ref_type CBWriter::get_dynamic_data(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type) noexcept
{
    // TODO fast this should not be done, but dyn types API is like it is.
    auto& data_no_const = const_cast<eprosima::fastdds::rtps::SerializedPayload_t&>(msg.payload);

    // Create PubSub Type
    fastdds::dds::DynamicPubSubType pubsub_type(dyn_type);
    fastdds::dds::DynamicData::_ref_type dyn_data(
        fastdds::dds::DynamicDataFactory::get_instance()->create_data(dyn_type));

    pubsub_type.deserialize(data_no_const, &dyn_data);

    return dyn_data;
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
