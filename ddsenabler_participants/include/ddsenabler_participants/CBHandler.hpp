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
 * @file CBHandler.hpp
 */

#pragma once

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <stdexcept>
#include <thread>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/dds/Payload.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddspipe_participants/participant/dynamic_types/ISchemaHandler.hpp>

#include <ddsenabler_participants/CBHandlerConfiguration.hpp>
#include <ddsenabler_participants/CBMessage.hpp>
#include <ddsenabler_participants/CBWriter.hpp>
#include <ddsenabler_participants/library/library_dll.h>

namespace std {
template<>
struct hash<eprosima::fastdds::dds::xtypes::TypeIdentifier>
{
    std::size_t operator ()(
            const eprosima::fastdds::dds::xtypes::TypeIdentifier& k) const
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

    /**
     * @brief Destructor
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    ~CBHandler();

    /**
     * @brief Create and store in \c schemas_ an OMG IDL (.idl format) schema.
     * Any samples following this schema that were received before the schema itself are moved to the memory buffer.
     *
     * @param [in] dyn_type DynamicType containing the type information required to generate the schema.
     * @param [in] type_id TypeIdentifier of the type.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void add_schema(
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const fastdds::dds::xtypes::TypeIdentifier& type_id) override;

    /**
     * @brief Add a data sample, associated to the given \c topic.
     *
     * The sample is added to buffer without schema.
     *
     * @param [in] topic DDS topic associated to this sample.
     * @param [in] data payload data to be added.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void add_data(
            const ddspipe::core::types::DdsTopic& topic,
            ddspipe::core::types::RtpsPayloadData& data) override;


    void set_data_callback(
            participants::DdsNotification callback)
    {
        cb_writer_.get()->set_data_callback(callback);
    }

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
    void write_sample(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    //! Handler configuration
    CBHandlerConfiguration configuration_;

    //! Payload pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! CB writer
    //CBWriter cb_writer_;
    std::unique_ptr<CBWriter> cb_writer_;

    //! Schemas map
    std::unordered_map<fastdds::dds::xtypes::TypeIdentifier, fastdds::dds::DynamicType::_ref_type> schemas_;

    //! Unique sequence number assigned to received messages. It is incremented with every sample added
    unsigned int unique_sequence_number_{0};

    //! Mutex synchronizing access to object's data structures
    std::mutex mtx_;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
