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
 * @file CBHandler.hpp
 */

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/data/RpcPayloadData.hpp>
#include <ddspipe_core/types/dds/Payload.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddspipe_participants/participant/dynamic_types/ISchemaHandler.hpp>

#include <ddsenabler_participants/CBCallbacks.hpp>
#include <ddsenabler_participants/CBHandlerConfiguration.hpp>
#include <ddsenabler_participants/CBMessage.hpp>
#include <ddsenabler_participants/CBWriter.hpp>
#include <ddsenabler_participants/RpcUtils.hpp>
#include <ddsenabler_participants/library/library_dll.h>

namespace std {
template<>
struct hash<eprosima::fastdds::dds::xtypes::TypeIdentifier>
{
    std::size_t operator ()(
            const eprosima::fastdds::dds::xtypes::TypeIdentifier& k) const
    {
        // The collection only has direct hash TypeIdentifiers so the EquivalenceHash can be used.
        return (static_cast<size_t>(k.equivalence_hash()[0]) << 16) |
               (static_cast<size_t>(k.equivalence_hash()[1]) << 8) |
               (static_cast<size_t>(k.equivalence_hash()[2]));
    }

};

template<>
struct hash<eprosima::ddsenabler::participants::UUID> {
    std::size_t operator()(const eprosima::ddsenabler::participants::UUID& uuid) const noexcept {
        std::size_t hash = 0;
        for (uint8_t byte : uuid) {
            hash ^= std::hash<uint8_t>{}(byte) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

} // std

namespace eprosima {
namespace ddsenabler {
namespace participants {

struct ActionRequestInfo
{
    ActionRequestInfo() = default;

    ActionRequestInfo(
            const std::string& _action_name,
            RpcUtils::ActionType action_type,
            uint64_t request_id)
            : action_name(_action_name)
            , goal_accepted_stamp(std::chrono::system_clock::now())
    {
        set_request(request_id, action_type);
    }

    void set_request(
            uint64_t request_id,
            RpcUtils::ActionType action_type)
    {
        switch (action_type)
        {
            case RpcUtils::ActionType::GOAL:
                goal_request_id = request_id;
                break;
            case RpcUtils::ActionType::RESULT:
                result_request_id = request_id;
                break;
            default:
                break;
        }
        return;
    }

    uint64_t get_request(
            RpcUtils::ActionType action_type) const
    {
        switch (action_type)
        {
            case RpcUtils::ActionType::GOAL:
                return goal_request_id;
            case RpcUtils::ActionType::RESULT:
                return result_request_id;
            default:
                return 0;
        }
    }

    bool set_result(const std::string&& str)
    {
        if (str.empty() || !result.empty())
        {
            return false; // Cannot set string if already set or empty
        }
        result = std::move(str);
        return true;
    }

    std::string action_name;
    uint64_t goal_request_id = 0;
    uint64_t result_request_id = 0;
    std::chrono::system_clock::time_point goal_accepted_stamp;
    std::string result;
    int erased{2}; // 2: End Status & Result not received yet, 1: One of them received, 0: both received, erase the action
};

/**
 * Class that manages the interaction between \c EnablerParticipant and CB.
 * Payloads are efficiently passed from DDS Pipe to CB without copying data (only references).
 *
 * @implements ISchemaHandler
 */
class CBHandler : public ddspipe::participants::ISchemaHandler
{

public:

    /**
     * CBHandler constructor by required values.
     *
     * Creates CBHandler instance with given configuration, payload pool.
     *
     * @param config:       Structure encapsulating all configuration options.
     * @param payload_pool: Owner of every payload contained in received messages.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    CBHandler(
            const CBHandlerConfiguration& config,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool);

    /**
     * @brief Destructor
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    ~CBHandler();

    /**
     * @brief Add a type schema, associated to the given \c dyn_type and \c type_id.
     *
     * @param [in] dyn_type DynamicType containing the type information required to generate the schema.
     * @param [in] type_id TypeIdentifier of the type.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void add_schema(
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const fastdds::dds::xtypes::TypeIdentifier& type_id) override;

    /**
     * @brief Add a topic, associated to the given \c topic.
     *
     * @param [in] topic DDS topic to be added.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void add_topic(
            const ddspipe::core::types::DdsTopic& topic);

    // TODO documentation
    DDSENABLER_PARTICIPANTS_DllAPI
    void add_service(
            const ddspipe::core::types::RpcTopic& service);

    DDSENABLER_PARTICIPANTS_DllAPI
    void add_action(
            const RpcUtils::RpcAction& action);

    /**
     * @brief Add a data sample, associated to the given \c topic.
     *
     * @param [in] topic DDS topic associated to this sample.
     * @param [in] data payload data to be added.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void add_data(
            const ddspipe::core::types::DdsTopic& topic,
            ddspipe::core::types::RtpsPayloadData& data) override;

    /**
     * @brief Get the TypeIdentifier associated to the given type name.
     *
     * @param [in] type_name Name of the type to be retrieved.
     * @param [out] type_identifier TypeIdentifier of the type.
     * @return \c true if the type was found, \c false otherwise.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    bool get_type_identifier(
            const std::string& type_name,
            fastdds::dds::xtypes::TypeIdentifier& type_identifier);

    /**
     * @brief Get the serialized data (payload) associated to the given type name from a JSON string.
     *
     * @param [in] type_name Name of the type of the data to be serialized.
     * @param [in] json JSON string containing the data to be serialized.
     * @param [out] payload Payload reference where the serialized data will be stored.
     * @return \c true if the data was successfully serialized, \c false otherwise.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    bool get_serialized_data(
            const std::string& type_name,
            const std::string& json,
            ddspipe::core::types::Payload& payload);

    DDSENABLER_PARTICIPANTS_DllAPI
    bool store_action_request(
            const std::string& action_name,
            const UUID& action_id,
            const uint64_t request_id,
            const RpcUtils::ActionType action_type);

    DDSENABLER_PARTICIPANTS_DllAPI
    bool store_action_result(
            const std::string& action_name,
            const UUID& action_id,
            const std::string& result);

    DDSENABLER_PARTICIPANTS_DllAPI
    void erase_action_UUID(
            const UUID& action_id,
            bool force_erase = false);

    DDSENABLER_PARTICIPANTS_DllAPI
    bool is_UUID_active(
            const std::string& action_name,
            const UUID& action_id,
            std::chrono::system_clock::time_point* goal_accepted_stamp = nullptr);

    /**
     * @brief Set the data notification callback.
     *
     * @param [in] callback Callback to be set.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void set_data_notification_callback(
            participants::DdsDataNotification callback)
    {
        cb_writer_->set_data_notification_callback(callback);
    }

    /**
     * @brief Set the topic notification callback.
     *
     * @param [in] callback Callback to be set.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void set_topic_notification_callback(
            participants::DdsTopicNotification callback)
    {
        cb_writer_->set_topic_notification_callback(callback);
    }

    /**
     * @brief Set the type notification callback.
     *
     * @param [in] callback Callback to be set.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void set_type_notification_callback(
            participants::DdsTypeNotification callback)
    {
        cb_writer_->set_type_notification_callback(callback);
    }

    /**
     * @brief Set the type query callback.
     *
     * @param [in] callback Callback to be set.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void set_type_query_callback(
            participants::DdsTypeQuery callback)
    {
        type_query_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_service_notification_callback(
            participants::ServiceNotification callback)
    {
        cb_writer_->set_service_notification_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_service_reply_notification_callback(
            participants::ServiceReplyNotification callback)
    {
        cb_writer_->set_service_reply_notification_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_service_request_notification_callback(
            participants::ServiceRequestNotification callback)
    {
        cb_writer_->set_service_request_notification_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_notification_callback(
            participants::ActionNotification callback)
    {
        cb_writer_->set_action_notification_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_result_notification_callback(
            participants::ActionResultNotification callback)
    {
        cb_writer_->set_action_result_notification_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_feedback_notification_callback(
            participants::ActionFeedbackNotification callback)
    {
        cb_writer_->set_action_feedback_notification_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_status_notification_callback(
            participants::ActionStatusNotification callback)
    {
        cb_writer_->set_action_status_notification_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_send_action_get_result_request_callback(
            std::function<bool(const std::string&, const participants::UUID&)> callback)
    {
        cb_writer_->set_send_action_get_result_request_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_goal_request_notification_callback(
            participants::ActionGoalRequestNotification callback)
    {
        cb_writer_->set_action_goal_request_notification_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_cancel_request_notification_callback(
            participants::ActionCancelRequestNotification callback)
    {
        cb_writer_->set_action_cancel_request_notification_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_send_action_send_goal_reply_callback(
            std::function<void(const std::string&, const uint64_t, bool accepted)> callback)
    {
        cb_writer_->set_send_action_send_goal_reply_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_send_action_get_result_reply_callback(
            std::function<bool(const std::string&, const UUID&, const std::string&, const uint64_t)> callback)
    {
        send_action_get_result_reply_callback_ = callback;
    }

protected:

    bool pop_action_request_UUID(
                const uint64_t request_id,
                const RpcUtils::ActionType action_type,
                UUID& action_id);

    bool get_action_result(
            const UUID& action_id,
            std::string& result);

    /**
     * @brief Add a schema, associated to the given \c dyn_type and \c type_id.
     *
     * @param [in] dyn_type DynamicType containing the type information required to generate the schema.
     * @param [in] type_id TypeIdentifier of the type.
     * @param [in] write_schema Whether to write the schema to CB or not.
     */
    void add_schema_nts_(
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const fastdds::dds::xtypes::TypeIdentifier& type_id,
            bool write_schema = true);

    /**
     * @brief Add a schema, associated to the given \c type_id and \c type_obj.
     *
     * @param [in] type_id TypeIdentifier of the type.
     * @param [in] type_obj TypeObject of the type.
     * @param [in] write_schema Whether to write the schema to CB or not.
     * @return \c true if the schema was added successfully, \c false otherwise.
     */
    bool add_schema_nts_(
            const fastdds::dds::xtypes::TypeIdentifier& type_id,
            const fastdds::dds::xtypes::TypeObject& type_obj,
            bool write_schema = true);

    /**
     * @brief Write the schema to CB.
     *
     * @param [in] dyn_type DynamicType containing the type information required to generate the schema.
     * @param [in] type_id TypeIdentifier of the type.
     */
    void write_schema_nts_(
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const fastdds::dds::xtypes::TypeIdentifier& type_id);

    /**
     * @brief Write the topic to CB.
     *
     * @param [in] topic DDS topic to be added.
     */
    void write_topic_nts_(
            const ddspipe::core::types::DdsTopic& topic);

    void write_service_nts_(
            const ddspipe::core::types::RpcTopic& service);

    void write_action_nts_(
            const RpcUtils::RpcAction& action);
    /**
     * @brief Write to CB.
     *
     * @param [in] msg CBMessage to be added
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    void write_sample_nts_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    void write_service_reply_nts_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    void write_service_request_nts_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    void write_action_result_nts_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const UUID& action_id);

    void write_action_feedback_nts_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    void write_action_goal_reply_nts_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const UUID& action_id);

    void write_action_cancel_reply_nts_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    void write_action_status_nts_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    void write_action_request_nts_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    /**
     * @brief Register a type using the given serialized type data.
     *
     * @param [in] type_name Name of the type to be registered.
     * @param [in] serialized_type Pointer to the serialized type data.
     * @param [in] serialized_type_size Size of the serialized type data.
     * @param [out] type_identifier TypeIdentifier of the registered type.
     * @param [out] type_object TypeObject of the registered type.
     * @return \c true if the type was registered successfully, \c false otherwise.
     */
    bool register_type_nts_(
            const std::string& type_name,
            const unsigned char* serialized_type,
            uint32_t serialized_type_size,
            fastdds::dds::xtypes::TypeIdentifier& type_identifier,
            fastdds::dds::xtypes::TypeObject& type_object);

    //! Handler configuration
    CBHandlerConfiguration configuration_;

    //! Payload pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! CB writer
    std::unique_ptr<CBWriter> cb_writer_;

    //! Schemas map
    std::map<std::string,
            std::pair<fastdds::dds::xtypes::TypeIdentifier, fastdds::dds::DynamicType::_ref_type>> schemas_;

    //! Unique sequence number assigned to received messages. It is incremented with every sample added
    unsigned int unique_sequence_number_{0};

    //! Mutex synchronizing access to object's data structures
    std::recursive_mutex mtx_;

    //! Callback to request types from the user
    DdsTypeQuery type_query_callback_;

    //! Counter of received requests
    uint64_t received_requests_id_{0};

    //! Map of any action services to the action's UUID
    std::unordered_map<participants::UUID, ActionRequestInfo> action_request_id_to_uuid_;

    std::function<bool(const std::string&, const UUID&, const std::string&, const uint64_t)> send_action_get_result_reply_callback_;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
