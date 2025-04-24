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
 * @file CBWriter.hpp
 */

#pragma once

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>

#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsenabler_participants/CBCallbacks.hpp>
#include <ddsenabler_participants/CBMessage.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

class CBWriter
{
public:

    CBWriter() = default;
    ~CBWriter() = default;

    void set_data_callback(
            DdsNotification callback)
    {
        data_callback_ = callback;
    }

    void set_reply_callback(
            ServiceReplyNotification callback)
    {
        reply_callback_ = callback;
    }

    void set_request_callback(
            ServiceRequestNotification callback)
    {
        request_callback_ = callback;
    }

    void set_type_callback(
            DdsTypeNotification callback)
    {
        type_callback_ = callback;
    }

    void set_topic_callback(
            DdsTopicNotification callback)
    {
        topic_callback_ = callback;
    }

    void write_schema(
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const fastdds::dds::xtypes::TypeIdentifier& type_id);

    void write_topic(
            const ddspipe::core::types::DdsTopic& topic);

    /**
     * @brief Writes data.
     *
     * @param [in] msg Pointer to the data to be written.
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    void write_data(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    void write_reply(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    void write_request(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

protected:

    /**
     * @brief Returns the dyn_data of a dyn_type.
     *
     * @param [in] msg Pointer to the data.
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    fastdds::dds::DynamicData::_ref_type get_dynamic_data_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type) noexcept;

    std::shared_ptr<void> prepare_json_data(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    std::string get_service_name(const std::string& topic_name);

    // Callbacks to notify the CB
    DdsNotification data_callback_;
    ServiceReplyNotification reply_callback_;
    ServiceRequestNotification request_callback_;
    DdsTypeNotification type_callback_;
    DdsTopicNotification topic_callback_;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
