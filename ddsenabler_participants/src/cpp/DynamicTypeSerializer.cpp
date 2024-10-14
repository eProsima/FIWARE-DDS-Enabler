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
 * @file DynamicTypesSerializer.cpp
 */

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/utils.hpp>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/utils.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/rtps/common/Types.hpp>

#include <ddsenabler_participants/DynamicTypesSerializer.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

bool DynamicTypesSerializer::store_dynamic_type(
        const std::string& type_name,
        const fastdds::dds::xtypes::TypeIdentifier& type_identifier,
        DynamicTypesCollection& dynamic_types)
{
    fastdds::dds::xtypes::TypeIdentifierPair type_identifiers;

    // NOTE: type_identifier is assumed to be complete
    type_identifiers.type_identifier1(type_identifier);

    fastdds::dds::xtypes::TypeInformation type_info;
    if (fastdds::dds::RETCODE_OK !=
            fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_information(
                type_identifiers,
                type_info,
                true))
    {
        EPROSIMA_LOG_WARNING(DDSENABLER_SERIALIZER,
                "Error getting TypeInformation for type " << type_name);
        return false;
    }

    std::string dependency_name;
    unsigned int dependency_index = 0;
    const auto type_dependencies = type_info.complete().dependent_typeids();
    for (auto dependency : type_dependencies)
    {
        fastdds::dds::xtypes::TypeIdentifier dependency_type_identifier;
        dependency_type_identifier = dependency.type_id();

        fastdds::dds::xtypes::TypeObject dependency_type_object;
        if (fastdds::dds::RETCODE_OK !=
                fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                    dependency_type_identifier,
                    dependency_type_object))
        {
            EPROSIMA_LOG_WARNING(DDSENABLER_SERIALIZER, "Error getting TypeObject of dependency "
                    << "for type " << type_name);
            return false;
        }

        dependency_name = type_name + "_" + std::to_string(dependency_index);

        // Store dependency in dynamic_types collection
        store_dynamic_type(dependency_type_identifier, dependency_type_object, dependency_name, dynamic_types);

        // Increment suffix counter
        dependency_index++;
    }

    fastdds::dds::xtypes::TypeObject type_object;
    if (fastdds::dds::RETCODE_OK !=
            fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                type_identifier,
                type_object))
    {
        EPROSIMA_LOG_WARNING(DDSENABLER_SERIALIZER, "Error getting TypeObject for type " << type_name);
        return false;
    }

    // Store dynamic type in dynamic_types collection
    return store_dynamic_type(type_identifier, type_object, type_name, dynamic_types);
}

bool DynamicTypesSerializer::store_dynamic_type(
        const fastdds::dds::xtypes::TypeIdentifier& type_identifier,
        const fastdds::dds::xtypes::TypeObject& type_object,
        const std::string& type_name,
        DynamicTypesCollection& dynamic_types)
{
    DynamicType dynamic_type;
    dynamic_type.type_name(type_name);

    try
    {
        dynamic_type.type_information(utils::base64_encode(serialize_type_identifier(type_identifier)));
        dynamic_type.type_object(utils::base64_encode(serialize_type_object(type_object)));
    }
    catch (const utils::InconsistencyException& e)
    {
        EPROSIMA_LOG_WARNING(DDSENABLER_SERIALIZER,
                "Error serializing DynamicType. Error message:\n " << e.what());
        return false;
    }

    dynamic_types.dynamic_types().push_back(dynamic_type);

    return true;
}

fastdds::rtps::SerializedPayload_t* DynamicTypesSerializer::serialize_dynamic_types(
        DynamicTypesCollection& dynamic_types)
{
    // Serialize dynamic types collection using CDR
    fastdds::dds::TypeSupport type_support(new DynamicTypesCollectionPubSubType());
    fastdds::rtps::SerializedPayload_t* serialized_payload = new fastdds::rtps::SerializedPayload_t(
        type_support.calculate_serialized_size(&dynamic_types, fastdds::dds::DEFAULT_DATA_REPRESENTATION));
    type_support.serialize(&dynamic_types, *serialized_payload, fastdds::dds::DEFAULT_DATA_REPRESENTATION);

    return serialized_payload;
}

template<class DynamicTypeData>
std::string DynamicTypesSerializer::serialize_type_data(
        const DynamicTypeData& type_data)
{
    // Reserve payload and create buffer
    fastcdr::CdrSizeCalculator calculator(fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment {0};
    size_t size = calculator.calculate_serialized_size(type_data, current_alignment) +
            fastdds::rtps::SerializedPayload_t::representation_header_size;

    fastdds::rtps::SerializedPayload_t payload(static_cast<uint32_t>(size));
    fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    // Create CDR serializer
    fastcdr::Cdr ser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN,
            fastcdr::CdrVersion::XCDRv2);

    payload.encapsulation = ser.endianness() == fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Serialize
    fastcdr::serialize(ser, type_data);
    payload.length = (uint32_t)ser.get_serialized_data_length();
    size = (ser.get_serialized_data_length() + 3) & ~3;

    // Create CDR message with payload
    std::unique_ptr<fastdds::rtps::CDRMessage_t> cdr_message = std::make_unique<fastdds::rtps::CDRMessage_t>(payload);

    // Add data
    if (!(cdr_message && (cdr_message->pos + payload.length <= cdr_message->max_size)) ||
            (payload.length > 0 && !payload.data))
    {
        if (!cdr_message)
        {
            throw utils::InconsistencyException(
                      "Error adding data -> cdr_message is null.");
        }
        else if (cdr_message->pos + payload.length > cdr_message->max_size)
        {
            throw utils::InconsistencyException(
                      "Error adding data -> not enough space in cdr_message buffer.");
        }
        else if (payload.length > 0 && !payload.data)
        {
            throw utils::InconsistencyException(
                      "Error adding data -> payload length is greater than 0, but payload data is null.");
        }
    }

    memcpy(&cdr_message->buffer[cdr_message->pos], payload.data, payload.length);
    cdr_message->pos += payload.length;
    cdr_message->length += payload.length;

    fastdds::rtps::octet value = 0;
    for (uint32_t count = payload.length; count < size; ++count)
    {
        const uint32_t size_octet = sizeof(value);
        if (!(cdr_message && (cdr_message->pos + size_octet <= cdr_message->max_size)))
        {
            throw utils::InconsistencyException(
                      "Not enough space in cdr_message buffer.");
        }
        for (uint32_t i = 0; i < size_octet; i++)
        {
            cdr_message->buffer[cdr_message->pos + i] = *((fastdds::rtps::octet*)&value + size_octet - 1 - i);
        }
        cdr_message->pos += size_octet;
        cdr_message->length += size_octet;
    }

    // Copy buffer to string
    std::string typedata_str(reinterpret_cast<char const*>(cdr_message->buffer), size);

    return typedata_str;
}

std::string DynamicTypesSerializer::serialize_type_identifier(
        const fastdds::dds::xtypes::TypeIdentifier& type_identifier)
{
    std::string typeid_string;
    try
    {
        typeid_string = serialize_type_data(type_identifier);
    }
    catch (const utils::InconsistencyException& e)
    {
        throw utils::InconsistencyException(std::string("Failed to serialize TypeIdentifier: ") + e.what());
    }

    return typeid_string;
}

std::string DynamicTypesSerializer::serialize_type_object(
        const fastdds::dds::xtypes::TypeObject& type_object)
{
    std::string typeobj_string;
    try
    {
        typeobj_string = serialize_type_data(type_object);
    }
    catch (const utils::InconsistencyException& e)
    {
        throw utils::InconsistencyException(std::string("Failed to serialize TypeObject: ") + e.what());
    }

    return typeobj_string;
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
