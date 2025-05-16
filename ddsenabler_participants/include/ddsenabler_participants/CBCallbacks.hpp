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
 * @file CBCallbacks.hpp
 */

#pragma once

#include <cstdint>

namespace eprosima {
namespace ddsenabler {
namespace participants {

using UUID = uint8_t[16];

enum STATUS_CODE {
        STATUS_UNKNOWN = 0,
        STATUS_ACCEPTED = 1,
        STATUS_EXECUTING = 2,
        STATUS_SUCCEEDED = 3,
        STATUS_ABORTED = 4,
        STATUS_CANCELED = 5,
        STATUS_REJECTED = 6,
        STATUS_TIMEOUT = 7,
        STATUS_FAILED = 8,
    };

/**
 * DdsLogFunc - callback for reception of DDS types
 */
typedef void (*DdsLogFunc)(
        const char* fileName,
        int lineNo,
        const char* funcName,
        int category,
        const char* msg);

/**
 * DdsTypeNotification - callback for reception of DDS types
 *
 * @note For actions the type notified is different from the used in ROS2 as it is stripped of the action UUID.
 */
typedef void (*DdsTypeNotification)(
        const char* typeName,
        const char* serializedType,
        const unsigned char* serializedTypeInternal,
        uint32_t serializedTypeInternalSize,
        const char* dataPlaceholder);

/**
 * DdsTopicNotification - callback for reception of DDS topics
 */
typedef void (*DdsTopicNotification)(
        const char* topicName,
        const char* typeName,
        const char* serializedQos);

/**
 * DdsNotification - callback for reception of DDS data
 */
typedef void (*DdsNotification)(
        const char* topicName,
        const char* json,
        int64_t publishTime);

// TODO: return a boolean in request callbacks? should nevertheless handle malformed strings passed by user
typedef void (*DdsTopicRequest)(
        const char* topicName,
        char*& typeName, // TODO: better pass unique_ptr by ref? Then the user would allocate resources but will always have its ownership
        char*& serializedQos);

typedef void (*DdsTypeRequest)(
        const char* typeName,
        unsigned char*& serializedTypeInternal,
        uint32_t& serializedTypeInternalSize);

struct ddsCallbacks
{
        participants::DdsNotification data_callback = nullptr;
        participants::DdsTypeNotification type_callback = nullptr;
        participants::DdsTopicNotification topic_callback = nullptr;
        participants::DdsTypeRequest type_req_callback = nullptr;
        participants::DdsTopicRequest topic_req_callback = nullptr;
        participants::DdsLogFunc log_callback = nullptr;
};


/**********************/
/*      SERVICES      */
/**********************/


/**
 * @brief Callback for notification of service discovery and its request and reply types.
 *
 * This callback is used to notify the discovery of a service and its associated request and reply types.
 *
 * @param serviceName The name of the service that was discovered.
 * @param requestTypeName The name of the request type associated with the service.
 * @param requestSerializedQos The serialized Quality of Service (QoS) settings for the request type.
 * @param replyTypeName The name of the reply type associated with the service.
 * @param replySerializedQos The serialized Quality of Service (QoS) settings for the reply type.
 */
typedef void (*ServiceNotification)(
        const char* serviceName,
        const char* requestTypeName,
        const char* replyTypeName,
        const char* requestSerializedQos,
        const char* replySerializedQos);

/**
 * @brief Callback for reception of RPC reply data.
 *
 * This callback is used to notify the reception of a reply for a specific service.
 *
 * @param serviceName The name of the service for which the reply was received.
 * @param json The JSON data received in the reply.
 * @param requestId The unique identifier of the request for which this is a reply.
 * @param publishTime The time at which the reply was published.
 */
typedef void (*ServiceReplyNotification)(
        const char* serviceName,
        const char* json,
        uint64_t requestId,
        int64_t publishTime);

/**
 * @brief Callback for reception of service request data.
 *
 * This callback is used to notify the reception of a request for a specific service.
 *
 * @param serviceName The name of the service for which the request was received.
 * @param json The JSON data received in the request.
 * @param requestId The unique identifier of the request.
 * @param publishTime The time at which the request was published.
 *
 * @note The requestId is unique for each request and must be later used to identify the reply.
 */
typedef void (*ServiceRequestNotification)(
        const char* serviceName,
        const char* json,
        uint64_t requestId,
        int64_t publishTime);

// TODO rename it so that "Type" is smth equal to the one in DdsTopicRequest
/**
 * @brief Callback requesting the type information of a given service's request and reply.
 *
 * This callback is used to request the type information for a service's request and reply.
 *
 * @param serviceName The name of the service for which the type information is requested.
 * @param requestTypeName The name of the request type associated with the service.
 * @param requestSerializedQos The serialized Quality of Service (QoS) settings for the request type.
 * @param replyTypeName The name of the reply type associated with the service.
 * @param replySerializedQos The serialized Quality of Service (QoS) settings for the reply type.
 */
typedef void (*ServiceTypeRequest)(
        const char* serviceName,
        char*& requestTypeName, // TODO: better pass unique_ptr by ref? Then the user would allocate resources but will always have its ownership
        char*& requestSerializedQos,
        char*& replyTypeName, // TODO: better pass unique_ptr by ref? Then the user would allocate resources but will always have its ownership
        char*& replySerializedQos);

struct serviceCallbacks
{
        participants::ServiceNotification service_callback = nullptr;
        participants::ServiceReplyNotification reply_callback = nullptr;
        participants::ServiceRequestNotification request_callback = nullptr;
        participants::ServiceTypeRequest type_req_callback = nullptr;
};

/**********************/
/*      ACTIONS       */
/**********************/


/**
 * @brief Callback for notification of action discovery and its associated types.
 *
 * This callback is used to notify the discovery of an action and its associated types.
 *
 * @param actionName The name of the action that was discovered.
 * @param goalRequestActionType The type of the goal request action.
 * @param goalReplyActionType The type of the goal reply action.
 * @param cancelRequestActionType The type of the cancel request action.
 * @param cancelReplyActionType The type of the cancel reply action.
 * @param resultRequestActionType The type of the get result request action.
 * @param resultReplyActionType The type of the get result reply action.
 * @param feedbackActionType The type of the feedback action.
 * @param goalRequestActionSerializedQos The serialized Quality of Service (QoS) settings for the goal request action.
 * @param goalReplyActionSerializedQos The serialized Quality of Service (QoS) settings for the goal reply action.
 * @param cancelRequestActionSerializedQos The serialized Quality of Service (QoS) settings for the cancel request action.
 * @param cancelReplyActionSerializedQos The serialized Quality of Service (QoS) settings for the cancel reply action.
 * @param resultRequestActionSerializedQos The serialized Quality of Service (QoS) settings for the get result request action.
 * @param resultReplyActionSerializedQos The serialized Quality of Service (QoS) settings for the get result reply action.
 * @param feedbackActionSerializedQos The serialized Quality of Service (QoS) settings for the feedback action.
 */
typedef void (*RosActionNotification)(
        const char* action_name,
        const char* goal_request_action_type,
        const char* goal_reply_action_type,
        const char* cancel_request_action_type,
        const char* cancel_reply_action_type,
        const char* result_request_action_type,
        const char* result_reply_action_type,
        const char* feedback_action_type,
        const char* goal_request_action_serialized_qos,
        const char* goal_reply_action_serialized_qos,
        const char* cancel_request_action_serialized_qos,
        const char* cancel_reply_action_serialized_qos,
        const char* result_request_action_serialized_qos,
        const char* result_reply_action_serialized_qos,
        const char* feedback_action_serialized_qos);

/**
 * @brief Callback for notification of action result.
 *
 * This callback is used to notify the result of an action in case of success.
 *
 * @param action_name The name of the action for which the result is being notified.
 * @param json The JSON data representing the result of the action.
 * @param goal_id The unique identifier of the goal associated with the action.
 * @param publish_time The time at which the result was published.
 */
typedef void (*RosActionResultNotification)(
        const char* action_name,
        const char* json,
        const UUID& goal_id,
        int64_t publish_time);

/**
 * @brief Callback for notification of action feedback.
 *
 * This callback is used to notify the feedback of an action.
 *
 * @param action_name The name of the action for which the feedback is being notified.
 * @param json The JSON data representing the feedback of the action.
 * @param goal_id The unique identifier of the goal associated with the action.
 * @param publish_time The time at which the feedback was published.
 */
typedef void (*RosActionFeedbackNotification)(
        const char* action_name,
        const char* json,
        const UUID& goal_id,
        int64_t publish_time);

/**
 * @brief Callback for notification of an update for an action status.
 *
 * This callback is used to notify the update of the status of an action.
 *
 * @param action_name The name of the action for which the status is being notified.
 * @param goal_id The unique identifier of the goal associated with the action.
 * @param status_code The status code representing the current state of the action.
 * @param status_message A message providing additional information about the status.
 * @param publish_time The time at which the status was published.
 */
typedef void (*RosActionStatusNotification)(
    const char* action_name,
    const UUID& goal_id,
    STATUS_CODE status_code,
    const char* status_message,
    int64_t publish_time);

/**
 * @brief Callback for notification of an action goal request.
 *
 * This callback is used to notify the request of an action goal.
 *
 * @param action_name The name of the action for which the goal is being requested.
 * @param json The JSON data representing the goal request.
 * @param goal_id The unique identifier of the goal associated with the action.
 * @param publish_time The time at which the goal request was published.
 * @param status_code The status code as a place holder for the current state of the action.
 */
typedef void (*RosActionGoalRequestNotification)(
    const char* action_name,
    const char* json,
    const UUID& goal_id,
    int64_t publish_time,
    STATUS_CODE& status_code);

/**
 * @brief Callback for requesting the action types.
 *
 * This callback is used to request the action types for a specific action.
 *
 * @param action_name The name of the action for which the types are being requested.
 * @param goal_request_action_type The type of the goal request action.
 * @param goal_reply_action_type The type of the goal reply action.
 * @param cancel_request_action_type The type of the cancel request action.
 * @param cancel_reply_action_type The type of the cancel reply action.
 * @param result_request_action_type The type of the get result request action.
 * @param result_reply_action_type The type of the get result reply action.
 * @param feedback_action_type The type of the feedback action.
 * @param goal_request_action_serialized_qos The serialized Quality of Service (QoS) settings for the goal request action.
 * @param goal_reply_action_serialized_qos The serialized Quality of Service (QoS) settings for the goal reply action.
 * @param cancel_request_action_serialized_qos The serialized Quality of Service (QoS) settings for the cancel request action.
 * @param cancel_reply_action_serialized_qos The serialized Quality of Service (QoS) settings for the cancel reply action.
 * @param result_request_action_serialized_qos The serialized Quality of Service (QoS) settings for the get result request action.
 * @param result_reply_action_serialized_qos The serialized Quality of Service (QoS) settings for the get result reply action.
 * @param feedback_action_serialized_qos The serialized Quality of Service (QoS) settings for the feedback action.
 */
typedef void (*RosActionTypeRequest)(
    const char* action_name,
    const char* goal_request_action_type,
    const char* goal_reply_action_type,
    const char* cancel_request_action_type,
    const char* cancel_reply_action_type,
    const char* result_request_action_type,
    const char* result_reply_action_type,
    const char* feedback_action_type,
    const char* goal_request_action_serialized_qos,
    const char* goal_reply_action_serialized_qos,
    const char* cancel_request_action_serialized_qos,
    const char* cancel_reply_action_serialized_qos,
    const char* result_request_action_serialized_qos,
    const char* result_reply_action_serialized_qos,
    const char* feedback_action_serialized_qos);

/**
 * @brief Callback for notification of an action cancel request.
 *
 * This callback is used to notify the request to cancel an action goal.
 *
 * @param action_name The name of the action for which the cancel request is being made.
 * @param goal_id The unique identifier of the goal associated with the action.
 * @param publish_time The time at which the cancel request was published.
 * @param status_code The status code as a place holder for the current state of the action.
 */
typedef void (*RosActionCancelRequestNotification)(
    const char* action_name,
    const UUID& goal_id,
    int64_t publish_time,
    STATUS_CODE& status_code);

struct actionCallbacks
{
    participants::RosActionNotification action_callback = nullptr;
    participants::RosActionResultNotification result_callback = nullptr;
    participants::RosActionFeedbackNotification feedback_callback = nullptr;
    participants::RosActionStatusNotification status_callback = nullptr;
    participants::RosActionGoalRequestNotification goal_request_callback = nullptr;
    participants::RosActionTypeRequest type_req_callback = nullptr;
    participants::RosActionCancelRequestNotification cancel_request_callback = nullptr;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
