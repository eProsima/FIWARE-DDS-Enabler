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
 * @file DdsParticipant.hpp
 */

#pragma once

#include <ddspipe_participants/participant/dynamic_types/DynTypesParticipant.hpp>

#include <ddsenabler_participants/library/library_dll.h>

namespace eprosima {
namespace ddsenabler {
namespace participants {

class DdsParticipant : public ddspipe::participants::DynTypesParticipant
{
public:

    DDSENABLER_PARTICIPANTS_DllAPI
    DdsParticipant(
            std::shared_ptr<ddspipe::participants::SimpleParticipantConfiguration> participant_configuration,
            std::shared_ptr<ddspipe::core::PayloadPool> payload_pool,
            std::shared_ptr<ddspipe::core::DiscoveryDatabase> discovery_database);

    DDSENABLER_PARTICIPANTS_DllAPI
    std::shared_ptr<ddspipe::core::IWriter> create_writer(
            const ddspipe::core::ITopic& topic) override;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
