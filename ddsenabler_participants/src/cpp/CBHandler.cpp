// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

    auto received_type = dyn_type->get_name().to_string();

    assert(nullptr != dyn_type);

    // Check if it exists already
    auto it = schemas_.find(type_id);
    if (it != schemas_.end())
    {
        return;
    }

    // Add to schemas map
    EPROSIMA_LOG_INFO(DDSENABLER_CB_HANDLER,
            "Adding schema with name " << dyn_type->get_name().to_string() << ".");

    schemas_[type_id] = dyn_type;
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

    fastdds::dds::DynamicType::_ref_type dyn_type;
    fastdds::dds::xtypes::TypeIdentifier type_id;

    if (eprosima::fastdds::dds::xtypes::TK_NONE != topic.type_identifiers.type_identifier1()._d())
    {
        type_id = topic.type_identifiers.type_identifier1();
    }
    else if (eprosima::fastdds::dds::xtypes::TK_NONE != topic.type_identifiers.type_identifier2()._d())
    {
        type_id = topic.type_identifiers.type_identifier2();
    }
    else
    {
        // NO TYPE_IDENTIFIERS
        EPROSIMA_LOG_WARNING(DDSENABLER_CB_HANDLER,
                "Received Schema for type " << topic.type_name << " with no TypeIdentifier.");
        return;
    }

    auto it = schemas_.find(type_id);
    if (it != schemas_.end())
    {
        dyn_type = it->second;
        // Schema available -> write
        write_sample(msg, dyn_type);
    }
    else
    {
        EPROSIMA_LOG_WARNING(DDSENABLER_CB_HANDLER,
                "Schema for type " << topic.type_name << " not available.");
    }

}

void CBHandler::write_sample(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
    cb_writer_->write_data(msg, dyn_type);
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
