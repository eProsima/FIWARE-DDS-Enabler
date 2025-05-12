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
 * @file serialization.hpp
 */

#pragma once

#include <memory>
#include <string>

#include <fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobject.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>

#include <ddspipe_core/types/dds/TopicQoS.hpp>

#include <ddsenabler_participants/types/dynamic_types_collection/DynamicTypesCollection.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {
namespace serialization {

/**
 * @brief Serialize a \c TopicQoS struct into a string.
 *
 * @param [in] qos TopicQoS to be serialized
 * @return Serialized TopicQoS string
 */
std::string serialize_qos(
        const ddspipe::core::types::TopicQoS& qos);

/**
 * @brief Deserialize a serialized \c TopicQoS string.
 *
 * @param [in] qos_str Serialized \c TopicQoS string
 * @return Deserialized TopicQoS
 */
ddspipe::core::types::TopicQoS deserialize_qos(
        const std::string& qos_str);

bool serialize_dynamic_type(
        const std::string& type_name,
        const fastdds::dds::xtypes::TypeIdentifier& type_identifier,
        DynamicTypesCollection& dynamic_types);

bool serialize_dynamic_type(
        const fastdds::dds::xtypes::TypeIdentifier& type_identifier,
        const fastdds::dds::xtypes::TypeObject& type_object,
        const std::string& type_name,
        DynamicTypesCollection& dynamic_types);

bool deserialize_dynamic_type(
        const DynamicType& dynamic_type,
        std::string& type_name,
        fastdds::dds::xtypes::TypeIdentifier& type_identifier,
        fastdds::dds::xtypes::TypeObject& type_object);

std::unique_ptr<fastdds::rtps::SerializedPayload_t> serialize_dynamic_types(
        const DynamicTypesCollection& dynamic_types);

bool deserialize_dynamic_types(
        const unsigned char* dynamic_types_payload,
        uint32_t dynamic_types_payload_size,
        DynamicTypesCollection& dynamic_types);

} /* namespace serialization */
} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
