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
 * @file CBHandler.hpp
 */

#pragma once

#include <cstdint>
#include <map>
#include <optional>

#include <cpp_utils/ReturnCode.hpp>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/dds/Payload.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddspipe_participants/participant/dynamic_types/ISchemaHandler.hpp>

#include <ddsenabler_participants/CBHandlerConfiguration.hpp>
#include <ddsenabler_participants/types/CBMessage.hpp>
#include <ddsenabler_participants/CBPublisher.hpp>
#include <ddsenabler_participants/CBWriter.hpp>
#include <ddsenabler_participants/library/library_dll.h>

using namespace eprosima::fastdds::dds;

namespace std {
template<>
struct hash<xtypes::TypeIdentifier>
{
    std::size_t operator ()(
            const xtypes::TypeIdentifier& k) const
    {
        // The collection only has direct hash TypeIdentifiers so the EquivalenceHash can be used.
        return (static_cast<size_t>(k.equivalence_hash()[0]) << 16) |
               (static_cast<size_t>(k.equivalence_hash()[1]) << 8) |
               (static_cast<size_t>(k.equivalence_hash()[2]));
    }

};

} // std

namespace eprosima {
namespace ddsenabler {
namespace participants {

/**
 * Class that manages the interaction between DDS Pipe \c (SchemaParticipant) and CB.
 * Payloads are efficiently passed from DDS Pipe to CB without copying data (only references).
 *
 * @implements ISchemaHandler
 */
class CBHandler : public ddspipe::participants::ISchemaHandler
{

public:

    /**
     * @brief CBHandler constructor by required values.
     *
     * Creates CBHandler instance with given configuration, payload pool.
     *
     * @param config:       Structure encapsulating all configuration options.
     * @param payload_pool: Owner of every payload contained in received messages.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    CBHandler(
            const CBHandlerConfiguration& config,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool);

    /**
     * @brief Destructor
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    ~CBHandler();

    /**
     * @brief Get the guidPrefix of the publisher participant.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    eprosima::fastdds::rtps::GuidPrefix_t get_publisher_guid();

    /**
     * @brief Set the callback to notify the context broker of data reception.
     *
     * @param [in] callback Callback to the contest broker.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void set_data_callback(
            participants::DdsNotification callback);

    /**
     * @brief Set the callback to notify the context broker of type reception.
     *
     * @param [in] callback Callback to the contest broker.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void set_type_callback(
            participants::DdsTypeNotification callback);

    /**
     * @brief Create and store in a type in \c known_types_.
     *
     * @param [in] dyn_type DynamicType containing the type information required to generate the schema.
     * @param [in] type_id TypeIdentifier of the type.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void add_schema(
            const DynamicType::_ref_type& dyn_type,
            const xtypes::TypeIdentifier& type_id) override;

    /**
     * @brief Add a data sample, associated to the given \c topic.
     *
     * @param [in] topic DDS topic associated to this sample.
     * @param [in] data payload data to be added.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void add_data(
            const ddspipe::core::types::DdsTopic& topic,
            ddspipe::core::types::RtpsPayloadData& data) override;

    /**
     * @brief Publish a data sample.
     *
     * @param [in] topic_name The name of the topic.
     * @param [in] type_name The name of the type.
     * @param [in] data_json JSON representation of the content.
     *
     * @return \c RETCODE_OK if data is published correctly.
     * @return \c RETCODE_PRECONDITION_NOT_MET if type is not known or unable to create writer.
     * @return \c ReturnCode_t if error when writing.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    ReturnCode_t publish_sample(
            std::string topic_name,
            std::string type_name,
            std::string data_json);

protected:

    /**
     * @brief Write to CB.
     *
     * @param [in] msg CBMessage to be added
     * @param [in] known_type Structure containing the type information required.
     */
    void write_schema(
            const CBMessage& msg,
            KnownType& known_type);

    /**
     * @brief Write to CB.
     *
     * @param [in] msg CBMessage to be added
     * @param [in] known_type Structure containing the type information required.
     */
    void write_sample(
            const CBMessage& msg,
            KnownType& known_type);

    /**
     * @brief Adds a type to known_types_ .
     *
     * @param [in] dyn_type DynamicType containing the type information required.
     * @param [in] type_id TypeIdentifier of the type.
     *
     * @return \c RETCODE_PRECONDITION_NOT_MET if type already existed
     * @return \c RETCODE_ERROR if unable to add type
     * @return \c RETCODE_OK if new type is added
     */
    utils::ReturnCode add_known_type(
            const DynamicType::_ref_type& dyn_type,
            const xtypes::TypeIdentifier& type_id);

    /**
     * @brief Gets a known type from known_types_.
     *
     * @param [in] type_name Name of the type to get.
     */
    std::optional<KnownType> get_known_type(
            const std::string type_name);

    //! Handler configuration
    CBHandlerConfiguration configuration_;

    //! Payload pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! CB writer
    std::unique_ptr<CBWriter> cb_writer_;

    //! CB Publisher
    std::unique_ptr<CBPublisher> cb_publisher_;

    //! Mutex to protect acces to known_types_
    std::mutex known_types_mutex_;

    //! KnownTypes map
    std::unordered_map<std::string, KnownType> known_types_;

    //! Unique sequence number assigned to received messages. It is incremented with every sample added
    unsigned int unique_sequence_number_{0};

    //! Mutex synchronizing access to object's data structures
    std::mutex mtx_;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
