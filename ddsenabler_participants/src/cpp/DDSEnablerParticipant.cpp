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
 * @file DDSEnablerParticipant.cpp
 */

#include <ddsenabler_participants/DDSEnablerParticipant.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

using namespace eprosima::fastdds::dds;
using namespace eprosima::ddspipe::core::types;

DDSEnablerParticipant::DDSEnablerParticipant(
        std::shared_ptr<SimpleParticipantConfiguration> participant_configuration,
        std::shared_ptr<PayloadPool> payload_pool,
        std::shared_ptr<DiscoveryDatabase> discovery_database,
        fastdds::rtps::GuidPrefix_t ddsenabler_publisher_guidPrefix)
    : DynTypesParticipant(
        participant_configuration,
        payload_pool,
        discovery_database)
{
    ddsenabler_publisher_guidPrefix_ = ddsenabler_publisher_guidPrefix;
}

void DDSEnablerParticipant::on_writer_discovery(
        fastdds::rtps::RTPSParticipant* participant,
        fastdds::rtps::WriterDiscoveryStatus reason,
        const fastdds::rtps::PublicationBuiltinTopicData& info,
        bool& should_be_ignored)
{
    if (info.guid.guidPrefix == ddsenabler_publisher_guidPrefix_)
    {
        // Ignore writers from DDSEnablerPublisher participant
        return;
    }

    if (info.guid.guidPrefix != participant->getGuid().guidPrefix)
    {
        // Get type information
        const auto type_info = info.type_information.type_information;
        const auto type_name = info.type_name.to_string();

        rtps::CommonParticipant::on_writer_discovery(participant, reason, info, should_be_ignored);

        notify_type_discovered_(type_info, type_name);
    }
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
