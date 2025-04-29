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

#include <filesystem>
#include <math.h>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/types/dynamic_types/types.hpp>

#include "ddsenabler/DDSEnabler.hpp"

namespace eprosima {
namespace ddsenabler {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::participants::rtps;
using namespace eprosima::ddsenabler::participants;
using namespace eprosima::utils;

DDSEnabler::DDSEnabler(
        const yaml::EnablerConfiguration& configuration,
        std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler)
    : configuration_(configuration)
    , event_handler_(event_handler)
{
    // Load the Enabler's internal topics from a configuration object.
    load_internal_topics_(configuration_);

    // Create Discovery Database
    discovery_database_ = std::make_shared<DiscoveryDatabase>();

    // Create Payload Pool
    payload_pool_ = std::make_shared<FastPayloadPool>();

    // Create Thread Pool
    thread_pool_ = std::make_shared<SlotThreadPool>(configuration_.n_threads);

    // Create CB Handler configuration
    participants::CBHandlerConfiguration handler_config;

    // Create DynTypes Participant
    dyn_participant_ = std::make_shared<DynTypesParticipant>(
        configuration_.simple_configuration,
        payload_pool_,
        discovery_database_);
    dyn_participant_->init();

    // Create CB Handler
    cb_handler_ = std::make_shared<participants::CBHandler>(
        handler_config,
        payload_pool_);

    // Create Enabler Participant
    enabler_participant_ = std::make_shared<SchemaParticipant>(
        configuration_.enabler_configuration,
        payload_pool_,
        discovery_database_,
        cb_handler_);

    // Create Participant Database
    participants_database_ = std::make_shared<ParticipantsDatabase>();

    // Populate Participant Database
    participants_database_->add_participant(
        dyn_participant_->id(),
        dyn_participant_);

    participants_database_->add_participant(
        enabler_participant_->id(),
        enabler_participant_);

    // Create DDS Pipe
    pipe_ = std::make_unique<DdsPipe>(
        configuration_.ddspipe_configuration,
        discovery_database_,
        payload_pool_,
        participants_database_,
        thread_pool_);
}

utils::ReturnCode DDSEnabler::reload_configuration(
        yaml::EnablerConfiguration& new_configuration)
{
    // Load the Enabler's internal topics from a configuration object.
    load_internal_topics_(new_configuration);

    // Update the Enabler's configuration
    configuration_ = new_configuration;

    return pipe_->reload_configuration(new_configuration.ddspipe_configuration);
}

void DDSEnabler::load_internal_topics_(
        yaml::EnablerConfiguration& configuration)
{
    // Create an internal topic to transmit the dynamic types
    configuration.ddspipe_configuration.builtin_topics.insert(
        utils::Heritable<DdsTopic>::make_heritable(type_object_topic()));

    if (!configuration.ddspipe_configuration.allowlist.empty())
    {
        // The allowlist is not empty. Add the internal topic.
        WildcardDdsFilterTopic internal_topic;
        internal_topic.topic_name.set_value(TYPE_OBJECT_TOPIC_NAME);

        configuration.ddspipe_configuration.allowlist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(internal_topic));
    }
}

} /* namespace ddsenabler */
} /* namespace eprosima */
