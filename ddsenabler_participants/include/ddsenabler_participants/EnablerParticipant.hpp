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
 * @file EnablerParticipant.hpp
 */

#pragma once

#include <condition_variable>
#include <map>
#include <mutex>
#include <optional>

#include <ddspipe_participants/participant/dynamic_types/SchemaParticipant.hpp>

#include <ddsenabler_participants/CBCallbacks.hpp>
#include <ddsenabler_participants/EnablerParticipantConfiguration.hpp>
#include <ddsenabler_participants/library/library_dll.h>
#include <ddsenabler_participants/InternalRpcReader.hpp>
#include <ddsenabler_participants/RpcUtils.hpp>


namespace eprosima {
namespace ddsenabler {
namespace participants {

struct ServiceDiscovered
{

    ServiceDiscovered(const std::string& service_name)
        : service_name(service_name)
    {
    }

    std::string service_name;

    ddspipe::core::types::DdsTopic topic_request;
    bool request_discovered{false};

    ddspipe::core::types::DdsTopic topic_reply;
    bool reply_discovered{false};

    std::optional<ddspipe::core::types::RpcTopic> rpc_topic;
    bool fully_discovered{false};
    bool enabler_as_server{false};

    bool add_topic(
            const ddspipe::core::types::DdsTopic& topic,
            RpcUtils::RpcType rpc_type)
    {
        if(rpc_type == RpcUtils::RpcType::RPC_REQUEST)
        {
                if(request_discovered)
                        return false;
                topic_request = topic;
                request_discovered = true;
        }
        else
        {
                if(reply_discovered)
                        return false;
                topic_reply = topic;
                reply_discovered = true;
        }

        if(request_discovered && reply_discovered)
        {
                if (service_name.empty())
                {
                        // TODO handle error
                        return false;
                }
                fully_discovered = true;
                rpc_topic = std::make_optional<ddspipe::core::types::RpcTopic>(
                    service_name,
                    topic_request,
                    topic_reply);
                return true;
        }

        return false;
    }

    bool remove_topic(RpcUtils::RpcType rpc_type)
    {
        if(rpc_type == RpcUtils::RpcType::RPC_REQUEST)
        {
            request_discovered = false;
            topic_request = ddspipe::core::types::DdsTopic();
        }
        else
        {
            reply_discovered = false;
            topic_reply = ddspipe::core::types::DdsTopic();
        }

        fully_discovered = false;
        rpc_topic = std::nullopt;

        return true;
    }

    ddspipe::core::types::RpcTopic get_service()
    {
        if(!fully_discovered || rpc_topic == std::nullopt)
            throw std::runtime_error("Service not fully discovered");
        return rpc_topic.value();
    }

    bool get_topic(
            const RpcUtils::RpcType& rpc_type,
            ddspipe::core::types::DdsTopic& topic)
    {
        if(rpc_type == RpcUtils::RpcType::RPC_REQUEST)
        {
            if(!request_discovered)
                return false;
            topic = topic_request;
            return true;
        }
        if (rpc_type == RpcUtils::RpcType::RPC_REPLY)
        {
            if(!reply_discovered)
                return false;
            topic = topic_reply;
            return true;
        }
        return false;
    }
};

struct ActionDiscovered
{
    ActionDiscovered(const std::string& action_name)
        : action_name(action_name)
    {
    }

    std::string action_name;
    std::weak_ptr<ServiceDiscovered> goal;
    std::weak_ptr<ServiceDiscovered> result;
    std::weak_ptr<ServiceDiscovered> cancel;
    ddspipe::core::types::DdsTopic feedback;
    bool feedback_discovered{false};
    ddspipe::core::types::DdsTopic status;
    bool status_discovered{false};
    bool fully_discovered{false};
    bool enabler_as_server{false};

    bool check_fully_discovered()
    {
        auto g = goal.lock();
        auto r = result.lock();
        auto c = cancel.lock();

        if (g && r && c &&
            g->fully_discovered && r->fully_discovered && c->fully_discovered &&
            feedback_discovered && status_discovered)
        {
            fully_discovered = true;
            return true;
        }
        fully_discovered = false;
        return false;
    }

    bool add_service(
            std::shared_ptr<ServiceDiscovered> service,
            RpcUtils::RpcType rpc_type)
    {
        switch (rpc_type)
        {
            case RpcUtils::RpcType::ACTION_GOAL_REQUEST:
            case RpcUtils::RpcType::ACTION_GOAL_REPLY:
                goal = service;
                break;
            case RpcUtils::RpcType::ACTION_RESULT_REQUEST:
            case RpcUtils::RpcType::ACTION_RESULT_REPLY:
                result = service;
                break;
            case RpcUtils::RpcType::ACTION_CANCEL_REQUEST:
            case RpcUtils::RpcType::ACTION_CANCEL_REPLY:
                cancel = service;
                break;
            default:
                return false;
        }
        return true;
    }


    bool add_topic(
            const ddspipe::core::types::DdsTopic& topic,
            RpcUtils::RpcType rpc_type)
    {
        switch (rpc_type)
        {
            case RpcUtils::RpcType::ACTION_FEEDBACK:
                feedback = topic;
                feedback_discovered = true;
                break;
            case RpcUtils::RpcType::ACTION_STATUS:
                status = topic;
                status_discovered = true;
                break;
            default:
                return false;
        }

        return true;
    }

    RpcUtils::RpcAction get_action(const std::string& action_name)
    {
        auto g = goal.lock();
        auto r = result.lock();
        auto c = cancel.lock();

        if (!fully_discovered || !g || !r || !c)
            throw std::runtime_error("Action not fully discovered or ServiceDiscovered expired");

        return RpcUtils::RpcAction(
            action_name,
            g->get_service(),
            r->get_service(),
            c->get_service(),
            feedback,
            status);
    }
};



class EnablerParticipant : public ddspipe::participants::SchemaParticipant
{
public:

    DDSENABLER_PARTICIPANTS_DllAPI
    EnablerParticipant(
            std::shared_ptr<EnablerParticipantConfiguration> participant_configuration,
            std::shared_ptr<ddspipe::core::PayloadPool> payload_pool,
            std::shared_ptr<ddspipe::core::DiscoveryDatabase> discovery_database,
            std::shared_ptr<ddspipe::participants::ISchemaHandler> schema_handler);

    DDSENABLER_PARTICIPANTS_DllAPI
    bool is_rtps_kind() const noexcept override
    {
        return true; // Temporal workaround until Pipe refactor
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    std::shared_ptr<ddspipe::core::IReader> create_reader(
            const ddspipe::core::ITopic& topic) override;

    DDSENABLER_PARTICIPANTS_DllAPI
    bool publish(
            const std::string& topic_name,
            const std::string& json);

    DDSENABLER_PARTICIPANTS_DllAPI
    bool publish_rpc(
            const std::string&  topic_name,
            const std::string& json,
            const uint64_t request_id);

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_topic_request_callback(
            participants::DdsTopicRequest callback)
    {
        topic_req_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_service_request_callback(
            participants::ServiceTypeRequest callback)
    {
        service_req_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_action_request_callback(
            participants::RosActionTypeRequest callback)
    {
        action_req_callback_ = callback;
    }


    DDSENABLER_PARTICIPANTS_DllAPI
    bool announce_service(
            const std::string& service_name);

    DDSENABLER_PARTICIPANTS_DllAPI
    bool revoke_service(
            const std::string& service_name);

    DDSENABLER_PARTICIPANTS_DllAPI
    bool announce_action(
            const std::string& action_name);

    DDSENABLER_PARTICIPANTS_DllAPI
    bool revoke_action(
            const std::string& action_name);

protected:
    // TODO unify name criteria for ending with _
    bool request_topic(
            const std::string& topic_name,
            ddspipe::core::types::DdsTopic& topic);

    bool request_service(
            std::shared_ptr<ServiceDiscovered> service);

    bool request_action_nts(
            ActionDiscovered& action,
            std::unique_lock<std::mutex>& lck);

    ddspipe::core::types::Endpoint create_topic_writer_nts_(
            const ddspipe::core::types::DdsTopic& topic,
            std::shared_ptr<eprosima::ddspipe::core::IReader>& reader,
            std::unique_lock<std::mutex>& lck);

    bool create_service_writer_nts_(
            const RpcUtils::RpcType& rpc_type,
            std::shared_ptr<ServiceDiscovered> service,
            std::unique_lock<std::mutex>& lck);

    bool fullfill_topic_type(
        const std::string& topic_name,
        const char* _type_name,
        const char* serialized_qos_content,
        ddspipe::core::types::DdsTopic& topic);

    bool fullfill_service_type(
            const char* _request_type_name,
            const char* serialized_request_qos_content,
            const char* _reply_type_name,
            const char* serialized_reply_qos_content,
            std::shared_ptr<ServiceDiscovered> service);

    std::shared_ptr<ddspipe::core::IReader> lookup_reader_nts_(
            const std::string& topic_name,
            std::string& type_name) const;

    std::shared_ptr<ddspipe::core::IReader> lookup_reader_nts_(
            const std::string& topic_name) const;

    bool service_discovered_nts(
            const std::string& service_name,
            const ddspipe::core::types::DdsTopic& topic,
            RpcUtils::RpcType rpc_type);

    bool action_discovered_nts(
            const std::string& action_name,
            const ddspipe::core::types::DdsTopic& topic,
            RpcUtils::RpcType rpc_type);

    bool revoke_service_nts(
            const std::string& service_name);

    std::map<ddspipe::core::types::DdsTopic, std::shared_ptr<ddspipe::core::IReader>> readers_;

    std::map<std::string, std::shared_ptr<ServiceDiscovered>> services_;

    std::map<std::string, ActionDiscovered> actions_;

    std::mutex mtx_;

    std::condition_variable cv_;

    DdsTopicRequest topic_req_callback_;

    ServiceTypeRequest service_req_callback_;

    RosActionTypeRequest action_req_callback_;

    // Store for a given service server its corresponding endpoint
    std::map<std::string, ddspipe::core::types::Endpoint> server_endpoint_;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
