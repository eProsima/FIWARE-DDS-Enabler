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
 RpcType get_service_name(const std::string& topic_name, std::string& service_name)
 {
     service_name = topic_name;

     // Remove prefix "rr/" or "rq/"
     if (service_name.rfind("rr/", 0) == 0)
     {
         service_name = service_name.substr(3);
     }
     else if (service_name.rfind("rq/", 0) == 0)
     {
         service_name = service_name.substr(3);
     }
     else
     {
         return RpcType::RPC_NONE;
     }

     // Remove suffix "Reply" or "Request"
     if (service_name.rfind("Reply", service_name.length() - 5) == service_name.length() - 5)
     {
             service_name = service_name.substr(0, service_name.length() - 5);
             return RpcType::RPC_REPLY;
     }
     else if (service_name.rfind("Request", service_name.length() - 7) == service_name.length() - 7)
     {
             service_name = service_name.substr(0, service_name.length() - 7);
             return RpcType::RPC_REQUEST;
     }

     return RpcType::RPC_NONE;
 }

 } // namespace RpcUtils
 } // namespace participants
 } // namespace ddsenabler
 } // namespace eprosima