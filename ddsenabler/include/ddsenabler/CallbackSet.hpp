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
 * @file CallbackSet.hpp
 */

#pragma once

#include <ddsenabler_participants/CBCallbacks.hpp>

namespace eprosima {
namespace ddsenabler {

/**
 * @brief Struct that encapsulates all the DDS related callbacks used by the DDS Enabler.
 */
struct DdsCallbacks
{
    participants::DdsTypeNotification type_notification{nullptr};
    participants::DdsTopicNotification topic_notification{nullptr};
    participants::DdsDataNotification data_notification{nullptr};
    participants::DdsTypeQuery type_query{nullptr};
    participants::DdsTopicQuery topic_query{nullptr};
};

struct ServiceCallbacks
{
    participants::ServiceNotification service_notification{nullptr};
    participants::ServiceRequestNotification service_request_notification{nullptr};
    participants::ServiceReplyNotification service_reply_notification{nullptr};
    participants::ServiceQuery service_query{nullptr};
};

struct ActionCallbacks
{
    participants::ActionNotification action_notification{nullptr};
    participants::ActionGoalRequestNotification action_goal_request_notification{nullptr};
    participants::ActionFeedbackNotification action_feedback_notification{nullptr};
    participants::ActionCancelRequestNotification action_cancel_request_notification{nullptr};
    participants::ActionResultNotification action_result_notification{nullptr};
    participants::ActionStatusNotification action_status_notification{nullptr};
    participants::ActionQuery action_query{nullptr};
};

/**
 * @brief Struct that encapsulates all the callbacks used by the DDS Enabler.
 */
struct CallbackSet
{
    participants::DdsLogFunc log{nullptr};
    DdsCallbacks dds;
    ServiceCallbacks service;
    ActionCallbacks action;
};

} /* namespace ddsenabler */
} /* namespace eprosima */
