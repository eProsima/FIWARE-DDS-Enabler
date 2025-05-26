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
#include <ddspipe_core/types/topic/rpc/RpcTopic.hpp>

#include <ddsenabler_participants/CBCallbacks.hpp>
#include <ddsenabler_participants/CBMessage.hpp>
#include <ddsenabler_participants/RpcUtils.hpp>

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

    void set_service_callback(
            ServiceNotification callback)
    {
        service_callback_ = callback;
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

    void set_service_type_request_callback(
            ServiceTypeRequest callback)
    {
        service_type_request_callback_ = callback;
    }

    void set_action_callback(
            RosActionNotification callback)
    {
        action_callback_ = callback;
    }

    void set_action_result_callback(
            RosActionResultNotification callback)
    {
        action_result_callback_ = callback;
    }

    void set_action_feedback_callback(
            RosActionFeedbackNotification callback)
    {
        action_feedback_callback_ = callback;
    }

    void set_action_status_callback(
            RosActionStatusNotification callback)
    {
        action_status_callback_ = callback;
    }

    void set_action_goal_request_notification_callback(
            RosActionGoalRequestNotification callback)
    {
        action_goal_request_notification_callback_ = callback;
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

    void write_service(
            const ddspipe::core::types::RpcTopic& service);

    void write_reply(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    void write_request(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    void write_action(
            const RpcUtils::RpcAction& action);

    void write_action_result(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const UUID& action_id);

    void write_action_feedback(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    void write_action_goal_reply(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const UUID& action_id);

    void write_action_cancel_reply(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const UUID& action_id);

    void write_action_status(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    void write_action_request(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    void set_is_UUID_active_callback(
            std::function<bool(const UUID&)> callback)
    {
        is_UUID_active_callback_ = callback;
    }

    void set_erase_action_UUID_callback(
            std::function<void(const UUID&)> callback)
    {
        erase_action_UUID_callback_ = callback;
    }

    void set_action_send_get_result_request_callback(
            std::function<bool(const std::string&, const participants::UUID&)> callback)
    {
        action_send_get_result_request_callback_ = callback;
    }

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

    // Callbacks to notify the CB
    DdsNotification data_callback_;
    DdsTypeNotification type_callback_;
    DdsTopicNotification topic_callback_;
    ServiceNotification service_callback_;
    ServiceReplyNotification reply_callback_;
    ServiceRequestNotification request_callback_;
    ServiceTypeRequest service_type_request_callback_;
    RosActionNotification action_callback_;
    RosActionResultNotification action_result_callback_;
    RosActionFeedbackNotification action_feedback_callback_;
    RosActionStatusNotification action_status_callback_;
    RosActionGoalRequestNotification action_goal_request_notification_callback_;

    std::function<bool(const UUID&)> is_UUID_active_callback_;
    std::function<void(const UUID&)> erase_action_UUID_callback_;
    std::function<bool(const std::string&, const participants::UUID&)> action_send_get_result_request_callback_;

};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
