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
     * DDSEnabler constructor by required values and event handler reference.
     *
     * Creates DDSEnabler instance with given configuration.
     *
     * @param configuration: Structure encapsulating all enabler configuration options.
     * @param event_handler: Reference to event handler used for thread synchronization in main application.
     */
    DDSEnabler(
            const yaml::EnablerConfiguration& configuration,
            std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler);


    void set_data_callback(
            participants::DdsNotification callback)
    {
        cb_handler_.get()->set_data_callback(callback);
    }

    void set_type_callback(
            participants::DdsTypeNotification callback)
    {
        cb_handler_.get()->set_type_callback(callback);
    }

    /**
     * Reconfigure the Enabler with the new configuration.
     *
     * @param new_configuration: The configuration to replace the previous configuration with.
     *
     * @return \c RETCODE_OK if allowed topics list has been updated correctly
     * @return \c RETCODE_NO_DATA if new allowed topics list is the same as the previous one
     */
    utils::ReturnCode reload_configuration(
            yaml::EnablerConfiguration& new_configuration);

    void add_topic_to_blocklist(
            std::string topic);

    void publish_json(
            std::string topic_name,
            std::string type_name,
            std::string data_json)
    {
        cb_handler_.get()->publish_sample(topic_name, type_name, data_json);
    }

    //     bool create_writer(
    //             std::string topic_name,
    //             std::string type_name,
    //             participants::KnownType& a_type)
    //     {
    //         if (RETCODE_OK != a_type.type_sup_.register_type(enabler_participant_))
    //         {
    //             EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER, "Error register_type: " <<
    //                     a_type.type_sup_.get_type_name());
    //             return false;
    //         }

    //         std::ostringstream topic_name;
    //         topic_name << "CBPublisher" << a_type.type_sup_.get_type_name() << "TopicName";
    //         Topic* topic = enabler_participant_->create_topic(topic_name.str(), a_type.type_sup_.get_type_name(),
    //                         TOPIC_QOS_DEFAULT);
    //         if (topic == nullptr)
    //         {
    //             EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER, "Error create_topic: " <<
    //                     a_type.type_sup_.get_type_name());
    //             return false;
    //         }

    //         DataWriterQos wqos = publisher_->get_default_datawriter_qos();
    //         a_type.writer_ = publisher_->create_datawriter(topic, wqos);
    //         if (a_type.writer_ == nullptr)
    //         {
    //             EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER, "Error create_datawriter: " <<
    //                     a_type.type_sup_.get_type_name());
    //             return false;
    //         }

    //         return true;
    //     }

    //     void pipe_publish_json(
    //             std::string topic_name,
    //             std::string type_name,
    //             std::string data_json)
    //     {
    //         auto known_type = cb_handler_.get()->get_known_type(topic_name);


    //         if (known_type.has_value())
    //         {
    //             if (known_type.value().writer_ == nullptr)
    //             {
    //                 create_writer(topic_name, type_name, known_type.value());
    //             }
    //         }

    //         // enabler_participant_->create_writer();

    //         cb_handler_.get()->publish_sample(topic_name, type_name, data_json);
    //     }

protected:

    /**
     * Load the Enabler's internal topics into a configuration object.
     *
     * @param configuration: The configuration to load the internal topics into.
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
    std::shared_ptr<eprosima::ddsenabler::participants::CBHandler> cb_handler_;

    //! Dynamic Types Participant
    std::shared_ptr<eprosima::ddspipe::participants::DynTypesParticipant> dyn_participant_;

    //! Schema Participant
    std::shared_ptr<eprosima::ddspipe::participants::SchemaParticipant> enabler_participant_;

    //! DDS Pipe
    std::unique_ptr<ddspipe::core::DdsPipe> pipe_;

    //! Reference to event handler used for thread synchronization in main application
    std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler_;
};

} /* namespace ddsenabler */
} /* namespace eprosima */
