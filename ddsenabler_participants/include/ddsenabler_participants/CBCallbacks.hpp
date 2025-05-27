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
#include <memory>
#include <string>

namespace eprosima {
namespace ddsenabler {
namespace participants {

/**
 * DdsLogFunc - callback executed when consuming log messages
 */
typedef void (* DdsLogFunc)(
        const char* file_name,
        int line_no,
        const char* func_name,
        int category,
        const char* msg);

/**
 * DdsTypeNotification - callback for notifying the reception of DDS types
 */
typedef void (* DdsTypeNotification)(
        const char* type_name,
        const char* serialized_type,
        const unsigned char* serialized_type_internal,
        uint32_t serialized_type_internal_size,
        const char* data_placeholder);

/**
 * DdsTopicNotification - callback for notifying the reception of DDS topics
 */
typedef void (* DdsTopicNotification)(
        const char* topic_name,
        const char* type_name,
        const char* serialized_qos);

/**
 * DdsDataNotification - callback for notifying the reception of DDS data
 */
typedef void (* DdsDataNotification)(
        const char* topic_name,
        const char* json,
        int64_t publish_time);

/**
 * DdsTopicRequest - callback for requesting information (type and QoS) of a DDS topic
 */
typedef bool (* DdsTopicRequest)(
        const char* topic_name,
        std::string& type_name,
        std::string& serialized_qos);

/**
 * DdsTypeRequest - callback for requesting information (serialized description and size) of a DDS type
 */
typedef bool (* DdsTypeRequest)(
        const char* type_name,
        std::unique_ptr<const unsigned char []>& serialized_type_internal,
        uint32_t& serialized_type_internal_size);

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
