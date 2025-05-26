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

#include <nlohmann/json.hpp>
#include <fstream>


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
    std::string original_topic_name = topic_name;
    rpc_name = topic_name;

    bool is_request = false;

    // Detect and remove prefix
    if (rpc_name.rfind("rq/", 0) == 0)
    {
        is_request = true;
        rpc_name = rpc_name.substr(3);
    }
    else if (rpc_name.rfind("rr/", 0) == 0)
    {
        rpc_name = rpc_name.substr(3);
    }
    else
    {
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

    // Check for action-related services
    if (is_request)
    {
        if (rpc_name.size() >= 7 && rpc_name.substr(rpc_name.size() - 7) == "Request")
        {
            std::string base = rpc_name.substr(0, rpc_name.size() - 7);

            if (base.size() >= 9 && base.substr(base.size() - 9) == "send_goal")
            {
                rpc_name = base.substr(0, base.size() - 9);
                return ACTION_GOAL_REQUEST;
            }
            else if (base.size() >= 10 && base.substr(base.size() - 10) == "get_result")
            {
                rpc_name = base.substr(0, base.size() - 10);
                return ACTION_RESULT_REQUEST;
            }
            else if (base.size() >= 11 && base.substr(base.size() - 11) == "cancel_goal")
            {
                rpc_name = base.substr(0, base.size() - 11);
                return ACTION_CANCEL_REQUEST;
            }

            rpc_name = base;
            return RPC_REQUEST;
        }
    }
    else if (rpc_name.size() >= 5 && rpc_name.substr(rpc_name.size() - 5) == "Reply")
    {
        std::string base = rpc_name.substr(0, rpc_name.size() - 5);

        if (base.size() >= 9 && base.substr(base.size() - 9) == "send_goal")
        {
            rpc_name = base.substr(0, base.size() - 9);
            return ACTION_GOAL_REPLY;
        }
        else if (base.size() >= 10 && base.substr(base.size() - 10) == "get_result")
        {
            rpc_name = base.substr(0, base.size() - 10);
            return ACTION_RESULT_REPLY;
        }
        else if (base.size() >= 11 && base.substr(base.size() - 11) == "cancel_goal")
        {
            rpc_name = base.substr(0, base.size() - 11);
            return ACTION_CANCEL_REPLY;
        }

        rpc_name = base;
        return RPC_REPLY;
    }

    rpc_name = original_topic_name; // Restore original topic name if no suffix matched
    EPROSIMA_LOG_ERROR(DDSENABLER_RPC_UTILS,
            "Invalid topic name for service: " << original_topic_name << ". Expected suffix 'Request' or 'Reply'.");
    return RPC_NONE;
}

RpcType get_service_name(const std::string& topic_name, std::string& service_name)
{
    service_name = topic_name;

    // Detect and remove prefix
    if (service_name.rfind("rq/", 0) == 0)
    {
        service_name = service_name.substr(3);
        if (service_name.size() >= 7 && service_name.substr(service_name.size() - 7) == "Request")
        {
            service_name = service_name.substr(0, service_name.size() - 7);
            return RPC_REQUEST;
        }
    }
    else if (service_name.rfind("rr/", 0) == 0)
    {
        service_name = service_name.substr(3);
        if (service_name.size() >= 5 && service_name.substr(service_name.size() - 5) == "Reply")
        {
            service_name = service_name.substr(0, service_name.size() - 5);
            return RPC_REPLY;
        }
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

void save_type_to_file(
    const std::string& directory,
    const char* typeName,
    const unsigned char*& serializedTypeInternal,
    const uint32_t& serializedTypeInternalSize)
{
    // Sanitize the typeName for file usage (basic version: replace ':' and '/' if any)
    std::string safeTypeName = typeName;
    for (auto& c : safeTypeName)
    {
        if (c == ':' || c == '/' || c == '\\') {
            c = '_';
        }
    }

    // Construct full file path
    std::string filePath = directory + "/" + safeTypeName + ".bin";

    // Open file in binary mode
    std::ofstream ofs(filePath, std::ios::binary);
    if (!ofs)
    {
        std::cerr << "Error opening file for writing: " << filePath << std::endl;
        return;
    }

    // Write the binary data
    ofs.write(reinterpret_cast<const char*>(serializedTypeInternal), serializedTypeInternalSize);

    if (!ofs.good())
    {
        std::cerr << "Error writing to file: " << filePath << std::endl;
    }

    ofs.close();
}


bool load_type_from_file(
    const std::string& directory,
    const char* typeName,
    unsigned char*& serializedTypeInternal,
    uint32_t& serializedTypeInternalSize)
{
    // Sanitize the type name (replace ':' and '/' with '_')
    std::string safeTypeName = typeName;
    for (auto& c : safeTypeName)
    {
        if (c == ':' || c == '/' || c == '\\') {
            c = '_';
        }
    }

    // Build file path
    std::string filePath = directory + safeTypeName + ".bin";

    // Open file in binary mode, position at end to get size
    std::ifstream ifs(filePath, std::ios::binary | std::ios::ate);
    if (!ifs)
    {
        std::cerr << "Error opening file for reading: " << filePath << std::endl;
        return false;
    }

    // Get file size
    std::streamsize size = ifs.tellg();
    if (size <= 0)
    {
        std::cerr << "File is empty or invalid: " << filePath << std::endl;
        return false;
    }
    serializedTypeInternalSize = static_cast<uint32_t>(size);

    // Allocate memory
    serializedTypeInternal = new unsigned char[serializedTypeInternalSize];

    // Read the data
    ifs.seekg(0, std::ios::beg);
    if (!ifs.read(reinterpret_cast<char*>(serializedTypeInternal), serializedTypeInternalSize))
    {
        std::cerr << "Error reading file: " << filePath << std::endl;
        delete[] serializedTypeInternal;
        serializedTypeInternal = nullptr;
        serializedTypeInternalSize = 0;
        return false;
    }

    return true;
}

void save_service_to_file(const char* serviceName,
        const char* requestTypeName,
        const char* replyTypeName,
        const char* requestSerializedQos,
        const char* replySerializedQos,
        const std::string& filename)
{
    nlohmann::json j;
    j["serviceName"] = serviceName;
    j["requestTypeName"] = requestTypeName;
    j["replyTypeName"] = replyTypeName;
    j["requestSerializedQos"] = requestSerializedQos;
    j["replySerializedQos"] = replySerializedQos;

    std::ofstream ofs(filename);
    if (ofs.is_open()) {
    ofs << j.dump(4);
    }
}

bool load_service_from_file(
        const char* serviceName,
        char*& requestTypeName,
        char*& replyTypeName,
        char*& requestSerializedQos,
        char*& replySerializedQos,
        const std::string& filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        return false;
    }

    nlohmann::json j;
    ifs >> j;

    std::string file_serviceName = j["serviceName"].get<std::string>();
    if (file_serviceName != std::string(serviceName)) {
        return false;  // Service name does not match
    }

    std::string _requestTypeName = j["requestTypeName"].get<std::string>();
    requestTypeName = strdup(_requestTypeName.c_str());
    std::string _replyTypeName = j["replyTypeName"].get<std::string>();
    replyTypeName = strdup(_replyTypeName.c_str());
    std::string _requestSerializedQos = j["requestSerializedQos"].get<std::string>();
    requestSerializedQos = strdup(_requestSerializedQos.c_str());
    std::string _replySerializedQos = j["replySerializedQos"].get<std::string>();
    replySerializedQos = strdup(_replySerializedQos.c_str());
    ifs.close();

    return true;
}

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
    const std::string& filename)
{
    nlohmann::json j;
    j["action_name"] = action_name;
    j["goal_request_action_type"] = goal_request_action_type;
    j["goal_reply_action_type"] = goal_reply_action_type;
    j["cancel_request_action_type"] = cancel_request_action_type;
    j["cancel_reply_action_type"] = cancel_reply_action_type;
    j["result_request_action_type"] = result_request_action_type;
    j["result_reply_action_type"] = result_reply_action_type;
    j["feedback_action_type"] = feedback_action_type;
    j["status_action_type"] = status_action_type;
    j["goal_request_action_serialized_qos"] = goal_request_action_serialized_qos;
    j["goal_reply_action_serialized_qos"] = goal_reply_action_serialized_qos;
    j["cancel_request_action_serialized_qos"] = cancel_request_action_serialized_qos;
    j["cancel_reply_action_serialized_qos"] = cancel_reply_action_serialized_qos;
    j["result_request_action_serialized_qos"] = result_request_action_serialized_qos;
    j["result_reply_action_serialized_qos"] = result_reply_action_serialized_qos;
    j["feedback_action_serialized_qos"] = feedback_action_serialized_qos;
    j["status_action_serialized_qos"] = status_action_serialized_qos;

    std::ofstream ofs(filename);
    if (ofs.is_open()) {
        ofs << j.dump(4);
        ofs.close();
    }
}

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
    const std::string& filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        return false;
    }

    nlohmann::json j;
    ifs >> j;

    std::string file_action_name = j["action_name"].get<std::string>();
    if (file_action_name != std::string(action_name)) {
        return false;  // Action name does not match
    }
    std::string _goal_request_action_type = j["goal_request_action_type"].get<std::string>();
    goal_request_action_type = strdup(_goal_request_action_type.c_str());
    std::string _goal_reply_action_type = j["goal_reply_action_type"].get<std::string>();
    goal_reply_action_type = strdup(_goal_reply_action_type.c_str());
    std::string _cancel_request_action_type = j["cancel_request_action_type"].get<std::string>();
    cancel_request_action_type = strdup(_cancel_request_action_type.c_str());
    std::string _cancel_reply_action_type = j["cancel_reply_action_type"].get<std::string>();
    cancel_reply_action_type = strdup(_cancel_reply_action_type.c_str());
    std::string _result_request_action_type = j["result_request_action_type"].get<std::string>();
    result_request_action_type = strdup(_result_request_action_type.c_str());
    std::string _result_reply_action_type = j["result_reply_action_type"].get<std::string>();
    result_reply_action_type = strdup(_result_reply_action_type.c_str());
    std::string _feedback_action_type = j["feedback_action_type"].get<std::string>();
    feedback_action_type = strdup(_feedback_action_type.c_str());
    std::string _status_action_type = j["status_action_type"].get<std::string>();
    status_action_type = strdup(_status_action_type.c_str());
    std::string _goal_request_action_serialized_qos = j["goal_request_action_serialized_qos"].get<std::string>();
    goal_request_action_serialized_qos = strdup(_goal_request_action_serialized_qos.c_str());
    std::string _goal_reply_action_serialized_qos = j["goal_reply_action_serialized_qos"].get<std::string>();
    goal_reply_action_serialized_qos = strdup(_goal_reply_action_serialized_qos.c_str());
    std::string _cancel_request_action_serialized_qos = j["cancel_request_action_serialized_qos"].get<std::string>();
    cancel_request_action_serialized_qos = strdup(_cancel_request_action_serialized_qos.c_str());
    std::string _cancel_reply_action_serialized_qos = j["cancel_reply_action_serialized_qos"].get<std::string>();
    cancel_reply_action_serialized_qos = strdup(_cancel_reply_action_serialized_qos.c_str());
    std::string _result_request_action_serialized_qos = j["result_request_action_serialized_qos"].get<std::string>();
    result_request_action_serialized_qos = strdup(_result_request_action_serialized_qos.c_str());
    std::string _result_reply_action_serialized_qos = j["result_reply_action_serialized_qos"].get<std::string>();
    result_reply_action_serialized_qos = strdup(_result_reply_action_serialized_qos.c_str());
    std::string _feedback_action_serialized_qos = j["feedback_action_serialized_qos"].get<std::string>();
    feedback_action_serialized_qos = strdup(_feedback_action_serialized_qos.c_str());
    std::string _status_action_serialized_qos = j["status_action_serialized_qos"].get<std::string>();
    status_action_serialized_qos = strdup(_status_action_serialized_qos.c_str());
    ifs.close();
    return true;
}

} // namespace RpcUtils
} // namespace participants
} // namespace ddsenabler
} // namespace eprosima

std::ostream& operator<<(std::ostream& os, const eprosima::ddsenabler::participants::UUID& uuid)
{
    for (size_t i = 0; i < uuid.size(); ++i)
    {
        if (i != 0)
            os << "-";
        os << std::to_string(uuid[i]);
    }
    return os;
}
