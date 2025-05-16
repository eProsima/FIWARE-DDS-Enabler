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
    ddspipe::core::types::DdsTopic topic_request;
    bool request_discovered{false};

    ddspipe::core::types::DdsTopic topic_reply;
    bool reply_discovered{false};

    std::optional<ddspipe::core::types::RpcTopic> rpc_topic;
    bool fully_discovered{false};

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
                fully_discovered = true;
                rpc_topic = std::make_optional<ddspipe::core::types::RpcTopic>(
                    topic_request.topic_name(),
                    topic_request,
                    topic_reply);
                return true;
        }

        return false;
    }

    ddspipe::core::types::RpcTopic get_service()
    {
        if(!fully_discovered || rpc_topic == std::nullopt)
            throw std::runtime_error("Service not fully discovered");
        return rpc_topic.value();
    }
};

struct ActionDiscovered
{
        ServiceDiscovered goal;
        ServiceDiscovered result;
        ServiceDiscovered cancel;
        ddspipe::core::types::DdsTopic feedback;
        bool feedback_discovered{false};
        ddspipe::core::types::DdsTopic status;
        bool status_discovered{false};
        bool fully_discovered{false};

        bool check_fully_discovered()
        {
            if(goal.fully_discovered && result.fully_discovered && cancel.fully_discovered &&
                   feedback_discovered && status_discovered)
            {
                fully_discovered = true;
                return true;
            }
            return false;
        }

        bool add_topic(
                const ddspipe::core::types::DdsTopic& topic,
                RpcUtils::RpcType rpc_type)
        {
            if(fully_discovered)
                return false;

            RpcUtils::RpcType action_direction = RpcUtils::get_service_direction(rpc_type);
            RpcUtils::ActionType action_type = RpcUtils::get_action_type(rpc_type);
            ServiceDiscovered* service = nullptr;
            switch (action_type)
            {
                case RpcUtils::ActionType::GOAL:
                    service = &goal;
                    break;
                case RpcUtils::ActionType::RESULT:
                    service = &result;
                    break;
                case RpcUtils::ActionType::CANCEL:
                    service = &cancel;
                    break;
                case RpcUtils::ActionType::FEEDBACK:
                    feedback = topic;
                    feedback_discovered = true;
                    break;
                case RpcUtils::ActionType::STATUS:
                    status = topic;
                    status_discovered = true;
                    break;
                default:
                    return false;
            }
            if(action_direction != RpcUtils::RpcType::RPC_NONE)
                service->add_topic(topic, action_direction);

            return check_fully_discovered();
        }

        RpcUtils::RpcAction get_action(const std::string& action_name)
        {
            if(!fully_discovered)
                throw std::runtime_error("Action not fully discovered");

            return RpcUtils::RpcAction(
                action_name,
                goal.get_service(),
                result.get_service(),
                cancel.get_service(),
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
    bool announce_service(
            const std::string& service_name);

    DDSENABLER_PARTICIPANTS_DllAPI
    bool revoke_service(
            const std::string& service_name);

protected:

    bool request_topic(
            const std::string& topic_name,
            ddspipe::core::types::DdsTopic& topic);

    bool request_service(
            const std::string& service_name,
            ServiceDiscovered& service);

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

    std::map<ddspipe::core::types::DdsTopic, std::shared_ptr<ddspipe::core::IReader>> readers_;

    std::map<std::string, ServiceDiscovered> services_;

    std::map<std::string, ActionDiscovered> actions_;

    std::mutex mtx_;

    std::condition_variable cv_;

    DdsTopicRequest topic_req_callback_;

    ServiceTypeRequest service_req_callback_;

    // Store for a given service server its corresponding endpoint
    std::map<std::string, ddspipe::core::types::Endpoint> server_endpoint_;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
