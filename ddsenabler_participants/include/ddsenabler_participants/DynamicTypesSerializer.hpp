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
 * @file DynamicTypesSerializer.hpp
 */

#pragma once

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

#include <ddsenabler_participants/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsenabler_participants/types/dynamic_types_collection/DynamicTypesCollectionPubSubTypes.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

class DynamicTypesSerializer
{
public:

    DynamicTypesSerializer() = default;
    ~DynamicTypesSerializer() = default;

    /**
     * @brief Serialize type identifier and object, and insert the result into a \c DynamicTypesCollection .
     *
     * @param [in] type_name        The name of the type, which serves as the key for storing the serialized type
     *                              identifier and object in the \c dynamic_types map.
     * @param [in] type_identifier  The TypeIdentifier that represents the type to be serialized.
     * @param [in, out] dynamic_types  The collection where the serialized type and identifier are stored.
     * @return bool  Returns true if the serialization and insertion were successful, false otherwise.
     */
    static bool store_dynamic_type(
            const std::string& type_name,
            const fastdds::dds::xtypes::TypeIdentifier& type_identifier,
            DynamicTypesCollection& dynamic_types);

    /**
     * @brief Serialize type identifier and object, and insert the result into a \c DynamicTypesCollection .
     *
     * @param [in] type_identifier Type identifier to be serialized and stored.
     * @param [in] type_object Type object to be serialized and stored.
     * @param [in] type_name Name of the type to be stored, used as key in \c dynamic_types map.
     * @param [in,out] dynamic_types Collection where to store serialized dynamic type.
     * @return bool  Returns true if the serialization and insertion were successful, false otherwise.
     */
    static bool store_dynamic_type(
            const fastdds::dds::xtypes::TypeIdentifier& type_identifier,
            const fastdds::dds::xtypes::TypeObject& type_object,
            const std::string& type_name,
            DynamicTypesCollection& dynamic_types);

    /**
     * @brief Serialize given \c DynamicTypesCollection into a \c SerializedPayload .
     *
     * @param [in] dynamic_types Dynamic types collection to be serialized.
     * @return Serialized payload for the given dynamic types collection.
     */
    static fastdds::rtps::SerializedPayload_t* serialize_dynamic_types(
            DynamicTypesCollection& dynamic_types);


    /**
     * @brief Serialize the provided dynamic type data into a string format.
     *
     * This method converts the given \c type_data of type \c TypeIdentifier / \c TypeObject into a serialized
     * string representation.
     *
     * @tparam DynamicTypeData  The type of the dynamic type data to be serialized ( \c TypeIdentifier / \c TypeObject )
     * @param [in] type_data    The data to be serialized, represented as an instance of \c DynamicTypeData.
     * @return std::string      A string containing the serialized representation of the \c type_data.
     */
    template<class DynamicTypeData>
    static std::string serialize_type_data(
            const DynamicTypeData& type_data);

    /**
     * @brief Serialize a \c TypeIdentifier into a string.
     *
     * @param [in] type_identifier TypeIdentifier to be serialized
     * @return Serialized TypeIdentifier string
     */
    static std::string serialize_type_identifier(
            const fastdds::dds::xtypes::TypeIdentifier& type_identifier);

    /**
     * @brief Serialize a \c TypeObject into a string.
     *
     * @param [in] type_object TypeObject to be serialized
     * @return Serialized TypeObject string
     */
    static std::string serialize_type_object(
            const fastdds::dds::xtypes::TypeObject& type_object);

};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
