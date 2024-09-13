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

#include <fstream>
#include <string>

#include <cpp_utils/utils.hpp>

#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include <ddsenabler_participants/CBWriter.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {


void write_to_file(
        const std::string& write_msg)
{
    std::ofstream logFile("/tmp/CBWriterlog.txt", std::ios_base::app); // Use an absolute path
    if (logFile.is_open())
    {
        logFile << write_msg << std::endl;
        logFile.close();
    }
}

std::string guid_to_str(
        const eprosima::ddspipe::core::types::Guid guid)
{
    std::stringstream ss;
    ss << std::hex;
    for (const auto& elem : guid.guidPrefix.value)
    {
        ss << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(elem);
    }
    for (const auto& elem : guid.entityId.value)
    {
        ss << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(elem);
    }

    std::string str = ss.str();
    std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(std::tolower(c));
            });
    return str;
}

std::vector<std::string> split_attributes(
        const std::string& input)
{
    std::vector<std::string> attributes;
    std::istringstream ss(input);
    std::string attribute;
    while (std::getline(ss, attribute, '}'))
    {
        if (!attribute.empty())
        {
            attribute += "}";
            attributes.push_back(attribute);
        }
    }
    return attributes;
}

void CBWriter::write_data(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
    std::lock_guard<std::mutex> lock(mutex_);

    write_schema(msg, dyn_type);

    logInfo(DDSENABLER_CB_WRITER,
            "Writing message: " << utils::from_bytes(msg.dataSize) << ".");

    // Get the data as JSON
    fastdds::dds::DynamicData::_ref_type dym_data = get_dynamic_data_(msg, dyn_type);

    std::stringstream ss;
    ss << std::setw(4);

    write_to_file("json_serialize1");
    fastdds::dds::json_serialize(dym_data, fastdds::dds::DynamicDataJsonFormat::EPROSIMA, ss);
    write_to_file("json_serialize2");

    // Create the base JSON structure
    nlohmann::ordered_json output;
    output["id"] = "msg.source_guid";
    output["type"] = "fastdds";
    output[msg.topic.topic_name()] = {
        {"type", msg.topic.type_name},
        {"data", nlohmann::json::object()}
    };

    // Split the attributes
    std::vector<std::string> attributes = split_attributes(ss.str());

    // Parse each attribute and add it to the output JSON
    for (const auto& attribute : attributes)
    {
        nlohmann::ordered_json parsed_attribute = nlohmann::json::parse(attribute);
        output[msg.topic.topic_name()]["data"]["data.instanceHandle.value"] = parsed_attribute;
    }

    //STORE DATA
    //store_data_callback(msg.topic.topic_name(), msg.topic.type_name, output);

    std::string write_msg = "\n---------------\n";
    write_msg = write_msg + output.dump(4);
    write_msg = write_msg + "\n---------------\n";
    write_to_file(write_msg);
}

void CBWriter::write_schema(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
    const std::string topic_name = msg.topic.topic_name();
    const fastdds::dds::xtypes::TypeIdentifier type_id = msg.topic.type_identifiers.type_identifier1();
    const std::string type_name = msg.topic.type_name;

    auto it = stored_schemas_.find(topic_name);
    if (it == stored_schemas_.end())
    {
        //Schema has not been registered
        logInfo(DDSENABLER_CB_WRITER,
                "Writing schema: " << type_name << " on topic: " << topic_name << ".");

        //STORE SCHEMA
        //store_schema_callback(topic_name, type_name, type_id, serialized_type_object);

        //Add the schema and topic to stored_schemas_
        stored_schemas_[topic_name] = type_id;

        std::string write_msg = "\n---------------\n";
        write_msg = write_msg + "Writing schema: " + type_name + " on topic: " + topic_name;
        write_msg = write_msg + "\n---------------\n";
        write_to_file(write_msg);
    }
    else
    {
        //Schema has been registered
        logInfo(DDSENABLER_CB_WRITER,
                "Schema: " + type_name + " already registered for topic: " + topic_name + ".");
    }
}

fastdds::dds::DynamicData::_ref_type CBWriter::get_dynamic_data_(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type) noexcept
{
    fastdds::dds::DynamicPubSubType pubsub_type(dyn_type);
    fastdds::dds::DynamicData::_ref_type dyn_data(fastdds::dds::DynamicDataFactory::get_instance()->
                    create_data(dyn_type));

    auto& data_no_const = const_cast<eprosima::fastdds::rtps::SerializedPayload_t&>(msg.payload);
    pubsub_type.deserialize(data_no_const, &dyn_data);

    return dyn_data;
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
