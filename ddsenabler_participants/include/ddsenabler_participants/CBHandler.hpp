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
            case RpcUtils::ActionType::CANCEL:
                cancel_request_id = request_id;
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
            case RpcUtils::ActionType::CANCEL:
                return cancel_request_id;
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
    uint64_t cancel_request_id = 0;
    uint64_t result_request_id = 0;
    std::string result;
    int erased{2}; // 2: End Status & End Result not received yet, 1: One of them received, 0: both received, erase the action
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
     * @brief Create and store in \c schemas_ an OMG IDL (.idl format) schema.
     * Any samples following this schema that were received before the schema itself are moved to the memory buffer.
     *
     * @param [in] dyn_type DynamicType containing the type information required to generate the schema.
     * @param [in] type_id TypeIdentifier of the type.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void add_schema(
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const fastdds::dds::xtypes::TypeIdentifier& type_id) override;

    // TODO
    DDSENABLER_PARTICIPANTS_DllAPI
    void add_topic(
            const ddspipe::core::types::DdsTopic& topic);

    DDSENABLER_PARTICIPANTS_DllAPI
    void add_service(
            const ddspipe::core::types::RpcTopic& service);

    DDSENABLER_PARTICIPANTS_DllAPI
    void add_action(
            const RpcUtils::RpcAction& action);

    /**
     * @brief Add a data sample, associated to the given \c topic.
     *
     * The sample is added to buffer without schema.
     *
     * @param [in] topic DDS topic associated to this sample.
     * @param [in] data payload data to be added.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void add_data(
            const ddspipe::core::types::DdsTopic& topic,
            ddspipe::core::types::RtpsPayloadData& data) override;

    DDSENABLER_PARTICIPANTS_DllAPI
    bool get_type_identifier(
            const std::string& type_name,
            fastdds::dds::xtypes::TypeIdentifier& type_identifier);

    DDSENABLER_PARTICIPANTS_DllAPI
    bool get_serialized_data(
            const std::string& type_name,
            const std::string& json,
            ddspipe::core::types::Payload& payload);

    DDSENABLER_PARTICIPANTS_DllAPI
    void store_action_request(
            const std::string& action_name,
            const UUID& action_id,
            const uint64_t request_id,
            const RpcUtils::ActionType action_type)
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_action_);

        auto it = action_request_id_to_uuid_.find(action_id);
        if (it != action_request_id_to_uuid_.end())
        {
            if (it->second.action_name != action_name)
            {
                EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
                        "Action name mismatch for action_id " << action_id
                        << ": expected " << it->second.action_name
                        << ", got " << action_name);
                return;
            }
            // If it exists, update the request_id for the given action_type
            it->second.set_request(request_id, action_type);
        }
        else
        {
            // If it does not exist, create a new entry
            action_request_id_to_uuid_[action_id] = ActionRequestInfo(action_name, action_type, request_id);
        }
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    bool store_action_result(
            const std::string& action_name,
            const UUID& action_id,
            const std::string& result)
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_action_);
        auto it = action_request_id_to_uuid_.find(action_id);
        if (it != action_request_id_to_uuid_.end())
        {
            if (it->second.action_name != action_name)
            {
                EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
                        "Action name mismatch for action_id " << action_id
                        << ": expected " << it->second.action_name
                        << ", got " << action_name);
                return false;
            }
            if (it->second.result_request_id != 0)
            {
                return action_send_result_reply_callback_(
                        action_name,
                        action_id,
                        result,
                        it->second.result_request_id);
            }
            if (it->second.set_result(std::move(result)))
            {
                return true;
            }
            else
            {
                EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
                        "Failed to store action result for action " << action_id
                        << ": result already set.");
                return false;
            }
        }
        EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
                "Failed to send action result to action " << action_id
                << ": goal id not found.");
        return false;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void erase_action_UUID(
            const UUID& action_id,
            bool force_erase = false)
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_action_);
        auto it = action_request_id_to_uuid_.find(action_id);
        if (it != action_request_id_to_uuid_.end())
        {
            it->second.erased--;
            // Only if both end result and end status have been received, erase the action
            if (force_erase || it->second.erased == 0)
                action_request_id_to_uuid_.erase(it);
        }
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    bool is_UUID_active(
            const std::string& action_name,
            const UUID& action_id)
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_action_);
        auto it = action_request_id_to_uuid_.find(action_id);
        if (it != action_request_id_to_uuid_.end() && action_name == it->second.action_name)
            return true;

        return false;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_data_callback(
            participants::DdsNotification callback)
    {
        cb_writer_->set_data_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_topic_callback(
            participants::DdsTopicNotification callback)
    {
        cb_writer_->set_topic_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_type_callback(
            participants::DdsTypeNotification callback)
    {
        cb_writer_->set_type_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_type_request_callback(
            participants::DdsTypeRequest callback)
    {
        type_req_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_service_callback(
            participants::ServiceNotification callback)
    {
        cb_writer_->set_service_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_reply_callback(
            participants::ServiceReplyNotification callback)
    {
        cb_writer_->set_reply_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_request_callback(
            participants::ServiceRequestNotification callback)
    {
        cb_writer_->set_request_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_callback(
            participants::RosActionNotification callback)
    {
        cb_writer_->set_action_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_result_callback(
            participants::RosActionResultNotification callback)
    {
        cb_writer_->set_action_result_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_feedback_callback(
            participants::RosActionFeedbackNotification callback)
    {
        cb_writer_->set_action_feedback_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_status_callback(
            participants::RosActionStatusNotification callback)
    {
        cb_writer_->set_action_status_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_send_get_result_request_callback(
            std::function<bool(const std::string&, const participants::UUID&)> callback)
    {
        cb_writer_->set_action_send_get_result_request_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_goal_request_notification_callback(
            participants::RosActionGoalRequestNotification callback)
    {
        cb_writer_->set_action_goal_request_notification_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_send_send_goal_reply_callback(
            std::function<void(const std::string&, const uint64_t, bool accepted)> callback)
    {
        cb_writer_->set_action_send_send_goal_reply_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_send_result_reply_callback(
        std::function<bool(const std::string&, const UUID&, const std::string&, const uint64_t)> callback)
    {
        action_send_result_reply_callback_ = callback;
    }

protected:

    bool pop_action_request_UUID(
                const uint64_t request_id,
                const RpcUtils::ActionType action_type,
                UUID& action_id)
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_action_);
        for (auto it = action_request_id_to_uuid_.begin(); it != action_request_id_to_uuid_.end(); ++it)
        {
            uint64_t action_request_id = it->second.get_request(action_type);
            if (request_id == action_request_id)
            {
                action_id = it->first;
                return true;
            }
        }
        return false;
    }

    bool get_action_result(
            const UUID& action_id,
            std::string& result)
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_action_);
        auto it = action_request_id_to_uuid_.find(action_id);
        if (it != action_request_id_to_uuid_.end())
        {
            if (!it->second.result.empty())
            {
                result = it->second.result;
                return true;
            }
        }
        return false;
    }

    void write_schema_(
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const fastdds::dds::xtypes::TypeIdentifier& type_id);

    void write_topic_(
            const ddspipe::core::types::DdsTopic& topic);

    /**
     * @brief Write to CB.
     *
     * @param [in] msg CBMessage to be added
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    void write_sample_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    void write_reply_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    void write_request_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    void write_action_result_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const UUID& action_id);

    void write_action_feedback_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    void write_action_goal_reply_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const UUID& action_id);

    void write_action_cancel_reply_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const UUID& action_id);

    void write_action_status_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    void write_action_request_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const uint64_t request_id);

    bool register_type_nts_(
            const std::string& type_name,
            const unsigned char* serialized_type,
            uint32_t serialized_type_size,
            fastdds::dds::xtypes::TypeIdentifier& type_identifier);

    //! Handler configuration
    CBHandlerConfiguration configuration_;

    //! Payload pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! CB writer
    std::unique_ptr<CBWriter> cb_writer_;

    //! Schemas map
    std::unordered_map<std::string, std::pair<fastdds::dds::xtypes::TypeIdentifier, fastdds::dds::DynamicType::_ref_type>> schemas_;

    //! Unique sequence number assigned to received messages. It is incremented with every sample added
    unsigned int unique_sequence_number_{0};

    //! Mutex synchronizing access to object's data structures
    std::mutex mtx_;

    //! Mutex synchronizing access to action related data structures
    std::recursive_mutex mtx_action_;

    DdsTypeRequest type_req_callback_;

    //! Counter of received requests
    uint64_t received_requests_id_{0};

    //! Map of any action services to the action's UUID
    std::unordered_map<participants::UUID, ActionRequestInfo> action_request_id_to_uuid_;

    std::function<bool(const std::string&, const UUID&, const std::string&, const uint64_t)> action_send_result_reply_callback_;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
