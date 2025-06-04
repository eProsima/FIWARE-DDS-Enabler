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

#include <map>

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

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

    DDSENABLER_PARTICIPANTS_DllAPI
    CBWriter() = default;

    DDSENABLER_PARTICIPANTS_DllAPI
    ~CBWriter() = default;

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_data_notification_callback(
            DdsDataNotification callback)
    {
        data_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_type_notification_callback(
            DdsTypeNotification callback)
    {
        type_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_topic_notification_callback(
            DdsTopicNotification callback)
    {
        topic_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_service_notification_callback(
            ServiceNotification callback)
    {
        service_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_service_reply_notification_callback(
            ServiceReplyNotification callback)
    {
        service_reply_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_service_request_notification_callback(
            ServiceRequestNotification callback)
    {
        service_request_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_notification_callback(
            ActionNotification callback)
    {
        action_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_result_notification_callback(
            ActionResultNotification callback)
    {
        action_result_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_feedback_notification_callback(
            ActionFeedbackNotification callback)
    {
        action_feedback_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_status_notification_callback(
            ActionStatusNotification callback)
    {
        action_status_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_goal_request_notification_callback(
            ActionGoalRequestNotification callback)
    {
        action_goal_request_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_cancel_request_notification_callback(
            ActionCancelRequestNotification callback)
    {
        action_cancel_request_notification_callback_ = callback;
    }

    /**
     * @brief Writes the schema of a DynamicType to CB.
     *
     * @param [in] dyn_type DynamicType containing the type information required.
     * @param [in] type_id TypeIdentifier of the DynamicType.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void write_schema(
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const fastdds::dds::xtypes::TypeIdentifier& type_id);

    /**
     * @brief Writes the topic to CB.
     *
     * @param [in] topic DDS topic to be added.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void write_topic(
            const ddspipe::core::types::DdsTopic& topic);

    /**
     * @brief Writes data.
     *
     * @param [in] msg Pointer to the data to be written.
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void write_data(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    DDSENABLER_PARTICIPANTS_DllAPI
    void write_service_notification(
            const ddspipe::core::types::RpcTopic& service);

    DDSENABLER_PARTICIPANTS_DllAPI
    void write_service_reply_notification(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    DDSENABLER_PARTICIPANTS_DllAPI
    void write_service_request_notification(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    DDSENABLER_PARTICIPANTS_DllAPI
    void write_action_notification(
            const RpcUtils::RpcAction& action);

    DDSENABLER_PARTICIPANTS_DllAPI
    void write_action_result_notification(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const UUID& action_id);

    DDSENABLER_PARTICIPANTS_DllAPI
    void write_action_feedback_notification(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    DDSENABLER_PARTICIPANTS_DllAPI
    void write_action_goal_reply_notification(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const UUID& action_id);

    DDSENABLER_PARTICIPANTS_DllAPI
    void write_action_cancel_reply_notification(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    DDSENABLER_PARTICIPANTS_DllAPI
    void write_action_status_notification(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    DDSENABLER_PARTICIPANTS_DllAPI
    void write_action_request_notification(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_is_UUID_active_callback(
            std::function<bool(const std::string&, const UUID&)> callback)
    {
        is_UUID_active_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_erase_action_UUID_callback(
            std::function<void(const UUID&)> callback)
    {
        erase_action_UUID_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_send_action_get_result_request_callback(
            std::function<bool(const std::string&, const participants::UUID&)> callback)
    {
        send_action_get_result_request_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_send_action_send_goal_reply_callback(
            std::function<void(const std::string&, const uint64_t, bool accepted)> callback)
    {
        send_action_send_goal_reply_callback_ = callback;
    }

    UUID uuid_from_request_json(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type);

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

    /**
     * @brief Returns the pubsub type of a dyn_type.
     *
     * @param [in] dyn_type DynamicType from which to get the pubsub type.
     * @return The pubsub type associated to the given dyn_type.
     * @note If the pubsub type is not already created, it will be created and stored in the map.
     */
    fastdds::dds::DynamicPubSubType get_pubsub_type_(
            const fastdds::dds::DynamicType::_ref_type& dyn_type) noexcept;

    bool prepare_json_data_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            nlohmann::json& json_output);

    // Callbacks to notify the CB
    DdsDataNotification data_notification_callback_;
    DdsTypeNotification type_notification_callback_;
    DdsTopicNotification topic_notification_callback_;
    ServiceNotification service_notification_callback_;
    ServiceReplyNotification service_reply_notification_callback_;
    ServiceRequestNotification service_request_notification_callback_;
    ActionNotification action_notification_callback_;
    ActionResultNotification action_result_notification_callback_;
    ActionFeedbackNotification action_feedback_notification_callback_;
    ActionStatusNotification action_status_notification_callback_;
    ActionGoalRequestNotification action_goal_request_notification_callback_;
    ActionCancelRequestNotification action_cancel_request_notification_callback_;



    // Map to store the pubsub types associated to dynamic types so they can be reused
    std::map<fastdds::dds::DynamicType::_ref_type, fastdds::dds::DynamicPubSubType> dynamic_pubsub_types_;

    std::function<bool(const std::string&, const UUID&)> is_UUID_active_callback_;
    std::function<void(const UUID&)> erase_action_UUID_callback_;
    std::function<bool(const std::string&, const participants::UUID&)> send_action_get_result_request_callback_;
    std::function<void(const std::string&, const uint64_t, bool accepted)> send_action_send_goal_reply_callback_;

};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
