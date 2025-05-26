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

// Struct that contains the information of a request: service name, request id and guid
struct RequestInfo
{
    RequestInfo(
            std::string request_topic,
            uint64_t request_id)
            : request_topic(std::move(request_topic))
            , request_id(request_id)
    {
    }

    RequestInfo() = default;

    std::string request_topic;
    uint64_t request_id;
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
    bool get_request_info(
            const uint64_t request_id,
            RequestInfo& request_info)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = request_id_map_.find(request_id);
        if (it != request_id_map_.end())
        {
            request_info = it->second;
            request_id_map_.erase(it);
            return true;
        }
        return false;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void store_action_request_UUID(
            const UUID& action_id,
            const uint64_t request_id)
    {
        std::lock_guard<std::mutex> lock(mtx_action_);
        action_request_id_to_uuid_[request_id] = action_id;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    bool pop_action_request_UUID(
                const uint64_t request_id,
                UUID& action_id)
    {
        std::lock_guard<std::mutex> lock(mtx_action_);
        auto it = action_request_id_to_uuid_.find(request_id);
        if (it != action_request_id_to_uuid_.end())
        {
            action_id = it->second;
            action_request_id_to_uuid_.erase(it);
            return true;
        }
        return false;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void store_action_UUID(
            const UUID& action_id)
    {
        std::lock_guard<std::mutex> lock(mtx_action_);
        action_uuid_vector_.insert(action_id);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void erase_action_UUID(
            const UUID& action_id)
    {
        std::lock_guard<std::mutex> lock(mtx_action_);
        action_uuid_vector_.erase(action_id);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    bool is_UUID_active(
            const UUID& action_id)
    {
        std::lock_guard<std::mutex> lock(mtx_action_);
        auto it = action_uuid_vector_.find(action_id);
        if (it != action_uuid_vector_.end())
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
        action_send_get_result_request_callback_ = callback;
        cb_writer_->set_action_send_get_result_request_callback(callback);
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_goal_request_notification_callback(
            participants::RosActionGoalRequestNotification callback)
    {
        cb_writer_->set_action_goal_request_notification_callback(callback);
    }

protected:

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
    std::mutex mtx_action_;

    DdsTypeRequest type_req_callback_;

    //! Counter of received requests
    uint64_t received_requests_id_{0};

    //! Map of request_id to request information
    std::unordered_map<uint64_t, RequestInfo> request_id_map_;

    //! Map of any action services to the action's UUID
    std::unordered_map<uint64_t, participants::UUID> action_request_id_to_uuid_;

    std::unordered_set<participants::UUID> action_uuid_vector_;

    std::function<bool(const std::string&, const participants::UUID&)> action_send_get_result_request_callback_;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
