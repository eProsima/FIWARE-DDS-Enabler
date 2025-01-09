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

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
