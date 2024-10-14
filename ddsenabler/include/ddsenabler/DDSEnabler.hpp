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
 * @file DDSEnabler.hpp
 */

#pragma once

#include <memory>
#include <set>

#include <cpp_utils/event/MultipleEventHandler.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/dynamic/AllowedTopicList.hpp>
#include <ddspipe_core/dynamic/DiscoveryDatabase.hpp>
#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>
#include <ddspipe_core/types/topic/dds/DistributedTopic.hpp>

#include <ddspipe_participants/participant/dynamic_types/DynTypesParticipant.hpp>
#include <ddspipe_participants/participant/dynamic_types/SchemaParticipant.hpp>

#include <ddsenabler_participants/CBHandler.hpp>
#include <ddsenabler_participants/CBHandlerConfiguration.hpp>
#include <ddsenabler_participants/DDSEnablerParticipant.hpp>

#include <ddsenabler_yaml/EnablerConfiguration.hpp>


namespace eprosima {
namespace ddsenabler {

/**
 * Wrapper class that encapsulates all dependencies required to launch DDS Enabler.
 */
class DDSEnabler
{
public:

    /**
     * @brief DDSEnabler constructor by required values and event handler reference.
     *
     * Creates DDSEnabler instance with given configuration.
     *
     * @param configuration: Structure encapsulating all enabler configuration options.
     * @param event_handler: Reference to event handler used for thread synchronization in main application.
     */
    DDSEnabler(
            const yaml::EnablerConfiguration& configuration,
            std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler);

    /**
     * @brief Sets the callback to notify the context broker of data reception.
     *
     * @param [in] callback Callback to the contest broker.
     */
    void set_data_callback(
            participants::DdsNotification callback);

    /**
     * @brief Sets the callback to notify the context broker of type reception.
     *
     * @param [in] callback Callback to the contest broker.
     */
    void set_type_callback(
            participants::DdsTypeNotification callback);

    /**
     * @brief Reconfigure the Enabler with the new configuration.
     *
     * @param [in] new_configuration: The configuration to replace the previous configuration with.
     *
     * @return \c RETCODE_OK if allowed topics list has been updated correctly.
     * @return \c RETCODE_NO_DATA if new allowed topics list is the same as the previous one.
     */
    utils::ReturnCode reload_configuration(
            yaml::EnablerConfiguration& new_configuration);

    /**
     * @brief Publish data.
     *
     * @param [in] topic_name: The name of the topic.
     * @param [in] type_name: The name of the type.
     * @param [in] data_json: The data to publish in JSON format.
     *
     * @return \c RETCODE_OK if data is published correctly.
     * @return \c RETCODE_PRECONDITION_NOT_MET if type is not known.
     * @return \c RETCODE_ERROR if unable to create writer.
     */
    ReturnCode_t publish_json(
            std::string topic_name,
            std::string type_name,
            std::string data_json);

protected:

    /**
     * @brief Load the Enabler's internal topics into a configuration object.
     *
     * @param [in] configuration: The configuration to load the internal topics into.
     */
    void load_internal_topics_(
            yaml::EnablerConfiguration& configuration);

    //! Configuration of the DDS Enabler
    yaml::EnablerConfiguration configuration_;

    //! Payload Pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Thread Pool
    std::shared_ptr<utils::SlotThreadPool> thread_pool_;

    //! Discovery Database
    std::shared_ptr<ddspipe::core::DiscoveryDatabase> discovery_database_;

    //! Participants Database
    std::shared_ptr<ddspipe::core::ParticipantsDatabase> participants_database_;

    //! CB Handler
    std::shared_ptr<ddsenabler::participants::CBHandler> cb_handler_;

    //! Dynamic Types Participant
    std::shared_ptr<ddspipe::participants::DynTypesParticipant> dyn_participant_;
    std::shared_ptr<ddsenabler::participants::DDSEnablerParticipant> ddsenabler_participant_;

    //! Schema Participant
    std::shared_ptr<ddspipe::participants::SchemaParticipant> enabler_participant_;

    //! DDS Pipe
    std::unique_ptr<ddspipe::core::DdsPipe> pipe_;

    //! Reference to event handler used for thread synchronization in main application
    std::shared_ptr<utils::event::MultipleEventHandler> event_handler_;
};

} /* namespace ddsenabler */
} /* namespace eprosima */
