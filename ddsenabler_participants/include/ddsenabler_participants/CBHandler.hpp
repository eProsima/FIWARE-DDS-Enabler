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
#include <ddsenabler_participants/CBMessage.hpp>
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
     * CBHandler constructor by required values.
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
    CBHandler(
            const CBHandlerConfiguration& config,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
            std::function<void(const std::string&)> add_topic_to_blocklist_callback);
    /**
     * @brief Destructor
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    ~CBHandler();

    /**
     * @brief Create and store in \c schemas_dynamictypes_ an OMG IDL (.idl format) schema.
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
     * @brief Publishes a data sample.
     *
     * @param [in] topic_name The name of the topic.
     * @param [in] type_name The name of the type.
     * @param [in] data_json JSON representation of the content.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    ReturnCode_t publish_sample(
            std::string topic_name,
            std::string type_name,
            std::string data_json);

    /**
     * @brief Sets the callback to notify the context broker of data reception.
     *
     * @param [in] callback Callback to the contest broker.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void set_data_callback(
            participants::DdsNotification callback)
    {
        cb_writer_.get()->set_data_callback(callback);
    }

    /**
     * @brief Sets the callback to notify the context broker of type reception.
     *
     * @param [in] callback Callback to the contest broker.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void set_type_callback(
            participants::DdsTypeNotification callback)
    {
        cb_writer_.get()->set_type_callback(callback);
    }

protected:

    /**
     * @brief Write to CB.
     *
     * @param [in] msg CBMessage to be added
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    void write_schema(
            const CBMessage& msg,
            const DynamicType::_ref_type& dyn_type);

    /**
     * @brief Write to CB.
     *
     * @param [in] msg CBMessage to be added
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    void write_sample(
            const CBMessage& msg,
            const DynamicType::_ref_type& dyn_type);

    /**
     * @brief Adds a type to known_types_ .
     *
     * @param [in] dyn_type DynamicType containing the type information required.
     * @param [in] type_id TypeIdentifier of the type.
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

    //! Callback function to add a topic to the blocklist
    std::function<void(const std::string&)> add_topic_to_blocklist_callback_;

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
