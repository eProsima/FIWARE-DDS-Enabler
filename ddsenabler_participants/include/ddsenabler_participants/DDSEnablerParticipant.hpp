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
 * @file DDSEnablerParticipant.hpp
 */

#pragma once

#include <ddspipe_participants/participant/dynamic_types/DynTypesParticipant.hpp>

#include <ddspipe_participants/library/library_dll.h>


using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;

namespace eprosima {
namespace ddsenabler {
namespace participants {


class DDSEnablerParticipant : public DynTypesParticipant
{
public:

    DDSPIPE_PARTICIPANTS_DllAPI
    DDSEnablerParticipant(
            std::shared_ptr<SimpleParticipantConfiguration> participant_configuration,
            std::shared_ptr<PayloadPool> payload_pool,
            std::shared_ptr<DiscoveryDatabase> discovery_database,
            eprosima::fastdds::rtps::GuidPrefix_t ddsenabler_publisher_guidPrefix);

    DDSPIPE_PARTICIPANTS_DllAPI
    void on_writer_discovery(
            fastdds::rtps::RTPSParticipant* participant,
            fastdds::rtps::WriterDiscoveryStatus reason,
            const fastdds::rtps::PublicationBuiltinTopicData& info,
            bool& should_be_ignored) override;

protected:

    eprosima::fastdds::rtps::GuidPrefix_t ddsenabler_publisher_guidPrefix_;

};


} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
