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
 * @file RpcUtils.hpp
 */

#pragma once

#include <string>

#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>
#include <ddspipe_core/types/topic/rpc/RpcTopic.hpp>
#include <ddsenabler_participants/CBCallbacks.hpp>


 namespace eprosima {
 namespace ddsenabler {
 namespace participants {
 namespace RpcUtils {

struct RpcAction
{
    RpcAction() = default;
    RpcAction(
            const std::string& action_name,
            const ddspipe::core::types::RpcTopic& goal,
            const ddspipe::core::types::RpcTopic& result,
            const ddspipe::core::types::RpcTopic& cancel,
            const ddspipe::core::types::DdsTopic& feedback,
            const ddspipe::core::types::DdsTopic& status)
        : action_name(action_name)
        , goal(goal)
        , result(result)
        , cancel(cancel)
        , feedback(feedback)
        , status(status)
    {
    }

    std::string action_name;
    ddspipe::core::types::RpcTopic goal;
    ddspipe::core::types::RpcTopic result;
    ddspipe::core::types::RpcTopic cancel;
    ddspipe::core::types::DdsTopic feedback;
    ddspipe::core::types::DdsTopic status;
};

enum RpcType
{
    RPC_NONE = 0,
    RPC_REQUEST,
    RPC_REPLY,
    ACTION_GOAL_REQUEST,
    ACTION_GOAL_REPLY,
    ACTION_RESULT_REQUEST,
    ACTION_RESULT_REPLY,
    ACTION_CANCEL_REQUEST,
    ACTION_CANCEL_REPLY,
    ACTION_FEEDBACK,
    ACTION_STATUS
};

enum ActionType
{
    NONE = 0,
    GOAL,
    RESULT,
    CANCEL,
    FEEDBACK,
    STATUS
};

/**
 * @brief Extracts the service/action name from a given topic name.
 *
 * @param [in] topic_name Topic name to extract the service/action name from
 * @return Extracted service name
 */
RpcType get_rpc_name(const std::string& topic_name, std::string& rpc_name);

RpcType get_service_name(const std::string& topic_name, std::string& service_name);

/**
 * @brief Returns the service direction (request or reply) based on the RPC type.
 *
 * @param rpc_type The RPC type to check.
 * @return The service direction (request or reply) or NONE if it is not a service.
 */
RpcType get_service_direction(RpcType rpc_type);

/**
 * @brief Returns the action type based on the RPC type.
 *
 * @param rpc_type The RPC type to check.
 * @return The action type (goal, result, cancel, feedback, status).
 */
ActionType get_action_type(RpcType rpc_type);

UUID generate_UUID();

/**
 * @brief Creates a JSON string for sending a goal request.
 *
 * @param goal_json The JSON string representing the goal (without the UUID part).
 * @return The JSON string for sending the goal request.
 */
std::string make_send_goal_request_json(const std::string& goal_json, UUID& goal_id);

void save_type_to_file(
    const std::string& directory,
    const char* typeName,
    const unsigned char*& serializedTypeInternal,
    const uint32_t& serializedTypeInternalSize);

bool load_type_from_file(
    const std::string& directory,
    const char* typeName,
    unsigned char*& serializedTypeInternal,
    uint32_t& serializedTypeInternalSize);

void save_service_to_file(
    const char* service_name,
    const char* request_type_name,
    const char* reply_type_name,
    const char* request_serialized_qos,
    const char* reply_serialized_qos,
    const std::string& filename);

bool load_service_from_file(
    const char* service_name,
    char*& request_type_name,
    char*& reply_type_name,
    char*& request_serialized_qos,
    char*& reply_serialized_qos,
    const std::string& filename);

void save_action_to_file(
    const char* action_name,
    const char* goal_request_action_type,
    const char* goal_reply_action_type,
    const char* cancel_request_action_type,
    const char* cancel_reply_action_type,
    const char* result_request_action_type,
    const char* result_reply_action_type,
    const char* feedback_action_type,
    const char* status_action_type,
    const char* goal_request_action_serialized_qos,
    const char* goal_reply_action_serialized_qos,
    const char* cancel_request_action_serialized_qos,
    const char* cancel_reply_action_serialized_qos,
    const char* result_request_action_serialized_qos,
    const char* result_reply_action_serialized_qos,
    const char* feedback_action_serialized_qos,
    const char* status_action_serialized_qos,
    const std::string& filename);

bool load_action_from_file(
    const char* action_name,
    char*& goal_request_action_type,
    char*& goal_reply_action_type,
    char*& cancel_request_action_type,
    char*& cancel_reply_action_type,
    char*& result_request_action_type,
    char*& result_reply_action_type,
    char*& feedback_action_type,
    char*& status_action_type,
    char*& goal_request_action_serialized_qos,
    char*& goal_reply_action_serialized_qos,
    char*& cancel_request_action_serialized_qos,
    char*& cancel_reply_action_serialized_qos,
    char*& result_request_action_serialized_qos,
    char*& result_reply_action_serialized_qos,
    char*& feedback_action_serialized_qos,
    char*& status_action_serialized_qos,
    const std::string& filename);

} // namespace RpcUtils
} // namespace participants
} // namespace ddsenabler
} // namespace eprosima

std::ostream& operator<<(std::ostream& os, const eprosima::ddsenabler::participants::UUID& uuid);