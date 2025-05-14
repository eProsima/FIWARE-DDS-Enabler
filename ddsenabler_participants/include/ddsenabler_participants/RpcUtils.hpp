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


 namespace eprosima {
 namespace ddsenabler {
 namespace participants {
 namespace RpcUtils {

enum RpcType
{
    RPC_NONE = 0,
    RPC_REQUEST,
    RPC_REPLY
};

/**
 * @brief Extracts the service name from a given topic name.
 *
 * @param [in] topic_name Topic name to extract the service name from
 * @return Extracted service name
 */
RpcType get_service_name(const std::string& topic_name, std::string& service_name);

} // namespace RpcUtils
} // namespace participants
} // namespace ddsenabler
} // namespace eprosima