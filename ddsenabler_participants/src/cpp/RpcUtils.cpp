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
 * @file RpcUtils.cpp
 */

#include <ddsenabler_participants/RpcUtils.hpp>


namespace eprosima {
namespace ddsenabler {
namespace participants {
namespace RpcUtils {

/**
 * @brief Extracts the service name from a given topic name.
 *
 * @param [in] topic_name Topic name to extract the service name from
 * @return Extracted service name
 */
RpcType get_rpc_name(const std::string& topic_name, std::string& rpc_name)
{
    rpc_name = topic_name;

    bool is_request = false;
    bool is_reply = false;

    // Detect and remove prefix
    if (rpc_name.rfind("rq/", 0) == 0)
    {
        is_request = true;
        rpc_name = rpc_name.substr(3);
    }
    else if (rpc_name.rfind("rr/", 0) == 0)
    {
        is_reply = true;
        rpc_name = rpc_name.substr(3);
    }

    // Check for action-related services
    if (is_request || is_reply)
    {
        if (rpc_name.size() >= 7 && rpc_name.substr(rpc_name.size() - 7) == "Request")
        {
            std::string base = rpc_name.substr(0, rpc_name.size() - 7);

            if (base.size() >= 9 && base.substr(base.size() - 9) == "send_goal")
            {
                rpc_name = base.substr(0, base.size() - 9);
                return is_request ? ACTION_GOAL_REQUEST : ACTION_GOAL_REPLY;
            }
            else if (base.size() >= 10 && base.substr(base.size() - 10) == "get_result")
            {
                rpc_name = base.substr(0, base.size() - 10);
                return is_request ? ACTION_RESULT_REQUEST : ACTION_RESULT_REPLY;
            }
            else if (base.size() >= 11 && base.substr(base.size() - 11) == "cancel_goal")
            {
                rpc_name = base.substr(0, base.size() - 11);
                return is_request ? ACTION_CANCEL_REQUEST : ACTION_CANCEL_REPLY;
            }

            rpc_name = base;
            return RPC_REQUEST;
        }
        else if (rpc_name.size() >= 5 && rpc_name.substr(rpc_name.size() - 5) == "Reply")
        {
            std::string base = rpc_name.substr(0, rpc_name.size() - 5);

            if (base.size() >= 9 && base.substr(base.size() - 9) == "send_goal")
            {
                rpc_name = base.substr(0, base.size() - 9);
                return is_request ? ACTION_GOAL_REQUEST : ACTION_GOAL_REPLY;
            }
            else if (base.size() >= 10 && base.substr(base.size() - 10) == "get_result")
            {
                rpc_name = base.substr(0, base.size() - 10);
                return is_request ? ACTION_RESULT_REQUEST : ACTION_RESULT_REPLY;
            }
            else if (base.size() >= 11 && base.substr(base.size() - 11) == "cancel_goal")
            {
                rpc_name = base.substr(0, base.size() - 11);
                return is_request ? ACTION_CANCEL_REQUEST : ACTION_CANCEL_REPLY;
            }

            rpc_name = base;
            return RPC_REPLY;
        }

        return RPC_NONE;
    }

    // TODO what if is a regular topic that ends with "/feedback" or "/status"? Maybe also check for "action" in the name?
    // Check for action feedback/status topics
    if (rpc_name.size() >= 9 && rpc_name.substr(rpc_name.size() - 9) == "/feedback")
    {
        rpc_name = rpc_name.substr(0, rpc_name.size() - 8);
        rpc_name = rpc_name.substr(3);
        return ACTION_FEEDBACK;
    }
    else if (rpc_name.size() >= 7 && rpc_name.substr(rpc_name.size() - 7) == "/status")
    {
        rpc_name = rpc_name.substr(0, rpc_name.size() - 6);
        rpc_name = rpc_name.substr(3);
        return ACTION_STATUS;
    }

    return RPC_NONE;
}

RpcType get_service_direction(RpcType rpc_type)
{
    switch (rpc_type)
    {
        case RPC_REQUEST:
        case RPC_REPLY:
            return rpc_type;
        case ACTION_GOAL_REQUEST:
        case ACTION_CANCEL_REQUEST:
        case ACTION_RESULT_REQUEST:
            return RPC_REQUEST;
        case ACTION_GOAL_REPLY:
        case ACTION_CANCEL_REPLY:
        case ACTION_RESULT_REPLY:
            return RPC_REPLY;
        default:
            break;
    }
    return RPC_NONE;
}

ActionType get_action_type(RpcType rpc_type)
{
    switch (rpc_type)
    {
        case ACTION_GOAL_REQUEST:
        case ACTION_GOAL_REPLY:
            return GOAL;
        case ACTION_RESULT_REQUEST:
        case ACTION_RESULT_REPLY:
            return RESULT;
        case ACTION_CANCEL_REQUEST:
        case ACTION_CANCEL_REPLY:
            return CANCEL;
        case ACTION_FEEDBACK:
            return FEEDBACK;
        case ACTION_STATUS:
            return STATUS;
        default:
            break;
    }
    return NONE;
}

UUID generate_UUID()
{
    UUID uuid;
    // Generate a random UUID
    for (size_t i = 0; i < sizeof(uuid); ++i)
    {
        uuid[i] = static_cast<uint8_t>(rand() % 256);
    }
    return uuid;
}

std::string make_send_goal_request_json(const std::string& goal_json, UUID& goal_id)
{
    goal_id = generate_UUID();

    std::string json = "{\"goal_id\": {\"uuid\": [";
    for (size_t i = 0; i < sizeof(goal_id); ++i)
    {
        json += std::to_string(goal_id[i]);
        if (i != sizeof(goal_id) - 1)
        {
            json += ", ";
        }
    }
    json += "]}, \"goal\": " + goal_json + "}";
    return json;
}

} // namespace RpcUtils
} // namespace participants
} // namespace ddsenabler
} // namespace eprosima