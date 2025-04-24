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
 * @file DdsParticipant.cpp
 */

#include <ddspipe_core/types/dynamic_types/types.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>
#include <ddspipe_participants/writer/auxiliar/BlankWriter.hpp>

#include <ddsenabler_participants/DdsParticipant.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;

DdsParticipant::DdsParticipant(
        std::shared_ptr<SimpleParticipantConfiguration> participant_configuration,
        std::shared_ptr<PayloadPool> payload_pool,
        std::shared_ptr<DiscoveryDatabase> discovery_database)
    : DynTypesParticipant(participant_configuration, payload_pool, discovery_database)
{
}

std::shared_ptr<IWriter> DdsParticipant::create_writer(
        const ITopic& topic)
{
    if (is_type_object_topic(topic))
    {
        return std::make_shared<BlankWriter>();
    }
    return rtps::SimpleParticipant::create_writer(topic);
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
