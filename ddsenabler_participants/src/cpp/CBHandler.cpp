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

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <vector>

#include <yaml-cpp/yaml.h>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>

#include <ddsenabler_participants/CBHandler.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

using namespace eprosima::fastdds::dds;
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
    cb_publisher_ = std::make_unique<CBPublisher>();
}

CBHandler::~CBHandler()
{
    EPROSIMA_LOG_INFO(DDSENABLER_CB_HANDLER,
            "Destroying CB handler.");
}

eprosima::fastdds::rtps::GuidPrefix_t CBHandler::get_publisher_guid()
{
    return cb_publisher_.get()->get_publisher_guid();
}

void CBHandler::set_data_callback(
        participants::DdsNotification callback)
{
    cb_writer_.get()->set_data_callback(callback);
}

void CBHandler::set_type_callback(
        participants::DdsTypeNotification callback)
{
    cb_writer_.get()->set_type_callback(callback);
}

void CBHandler::add_schema(
        const DynamicType::_ref_type& dyn_type,
        const xtypes::TypeIdentifier& type_id)
{
    std::lock_guard<std::mutex> lock(mtx_);

    assert(nullptr != dyn_type);

    // Add to schemas map
    EPROSIMA_LOG_INFO(DDSENABLER_CB_HANDLER,
            "Adding schema with name " << dyn_type->get_name().to_string() << ".");

    auto ret =  add_known_type(dyn_type, type_id);
    if (ret == utils::ReturnCode::RETCODE_OK)
    {
        EPROSIMA_LOG_INFO(DDSENABLER_CB_HANDLER,
                "Schema with name " << dyn_type->get_name().to_string() << " added successfully.");
    }
    else if (ret == utils::ReturnCode::RETCODE_NO_DATA)
    {
        EPROSIMA_LOG_INFO(DDSENABLER_CB_HANDLER,
                "Schema with name " << dyn_type->get_name().to_string() << " already added.");
    }
    else
    {
        EPROSIMA_LOG_WARNING(DDSENABLER_CB_HANDLER,
                "Error adding schema with name " << dyn_type->get_name().to_string() << ".");
    }
}

void CBHandler::add_data(
        const DdsTopic& topic,
        RtpsPayloadData& data)
{
    std::unique_lock<std::mutex> lock(mtx_);

    EPROSIMA_LOG_INFO(DDSENABLER_CB_HANDLER,
            "Adding data in topic: " << topic << ".");

    CBMessage msg;
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

    if (xtypes::TK_NONE == topic.type_identifiers.type_identifier1()._d() &&
            xtypes::TK_NONE == topic.type_identifiers.type_identifier2()._d())
    {
        // NO TYPE_IDENTIFIERS
        EPROSIMA_LOG_WARNING(DDSENABLER_CB_HANDLER,
                "Received Schema for type " << topic.type_name << " with no TypeIdentifier.");
        return;
    }

    auto it = known_types_.find(topic.type_name);
    if (it != known_types_.end())
    {
        // Schema available -> write
        write_schema(msg, it->second);
        write_sample(msg, it->second);
    }
    else
    {
        EPROSIMA_LOG_WARNING(DDSENABLER_CB_HANDLER,
                "Schema for type " << topic.type_name << " not available.");
    }
}

ReturnCode_t CBHandler::publish_sample(
        std::string topic_name,
        std::string type_name,
        std::string data_json)
{
    auto known_type = get_known_type(type_name);

    if (known_type.has_value())
    {
        if (!cb_publisher_->create_writer(topic_name, known_type.value()))
        {
            EPROSIMA_LOG_ERROR(DDSENABLER_CB_HANDLER,
                    "CBHandler::publish_sample, can not create writer for type:" << type_name << ".");
            return RETCODE_PRECONDITION_NOT_MET;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_HANDLER,
                "CBHandler::publish_sample, uknown type: " << type_name << ".");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return cb_publisher_->publish_data(topic_name, known_type.value(), data_json);
}

void CBHandler::write_schema(
        const CBMessage& msg,
        KnownType& known_type)
{
    if (!known_type.is_written_)
    {
        //Schema has not been registered
        EPROSIMA_LOG_INFO(DDSENABLER_CB_WRITER,
                "Writing schema: " << msg.topic.type_name << ".");

        //STORE SCHEMA
        cb_writer_->write_schema(msg, known_type);
    }
    else
    {
        //Schema has been registered
        EPROSIMA_LOG_INFO(DDSENABLER_CB_WRITER,
                "Schema: " << msg.topic.type_name << " already registered.");
    }
}

void CBHandler::write_sample(
        const CBMessage& msg,
        KnownType& known_type)
{
    cb_writer_->write_data(msg, known_type);
}

utils::ReturnCode CBHandler::add_known_type(
        const DynamicType::_ref_type& dyn_type,
        const xtypes::TypeIdentifier& type_id)
{
    std::lock_guard<std::mutex> guard(known_types_mutex_);
    const std::string type_name = dyn_type->get_name().to_string();
    try
    {
        if (known_types_.find(type_name) == known_types_.end())
        {
            KnownType a_type;
            a_type.type_id_ = type_id;
            a_type.dyn_type_ = dyn_type;
            a_type.type_sup_.reset(new DynamicPubSubType(dyn_type));

            known_types_.emplace(type_name, a_type);
        }
        else
        {
            return utils::ReturnCode::RETCODE_PRECONDITION_NOT_MET;
        }
    }
    catch (const std::exception& e)
    {
        return utils::ReturnCode::RETCODE_ERROR;
    }

    return utils::ReturnCode::RETCODE_OK;
}

std::optional<KnownType> CBHandler::get_known_type(
        const std::string type_name)
{
    std::lock_guard<std::mutex> guard(known_types_mutex_);
    auto it = known_types_.find(type_name);
    if (it != known_types_.end())
    {
        return it->second;
    }
    else
    {
        EPROSIMA_LOG_WARNING(DDSENABLER_CB_HANDLER,
                "Tried to get uknown type " << type_name << ".");
        return std::nullopt;
    }
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
