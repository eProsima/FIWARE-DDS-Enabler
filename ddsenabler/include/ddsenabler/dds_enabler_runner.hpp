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
 * @file dds_enabler_runner.hpp
 *
 */

#pragma once

#include <string>

#include <cpp_utils/event/FileWatcherHandler.hpp>
#include <cpp_utils/event/PeriodicEventHandler.hpp>
#include <cpp_utils/exception/ConfigurationException.hpp>
#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/logging/BaseLogConfiguration.hpp>
#include <cpp_utils/logging/StdLogConsumer.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/time/time_utils.hpp>

#include <ddsenabler_participants/CBCallbacks.hpp>

#include <ddsenabler_yaml/EnablerConfiguration.hpp>

#include "ddsenabler/DDSEnabler.hpp"

namespace eprosima {
namespace ddsenabler {

bool create_dds_enabler(
        const char* ddsEnablerConfigFile,
        participants::ddsCallbacks& dds_callbacks,
        participants::serviceCallbacks& service_callbacks,
        participants::actionCallbacks& action_callbacks,
        std::unique_ptr<DDSEnabler>& enabler);

bool create_dds_enabler(
        yaml::EnablerConfiguration configuration,
        participants::ddsCallbacks& dds_callbacks,
        participants::serviceCallbacks& service_callbacks,
        participants::actionCallbacks& action_callbacks,
        std::unique_ptr<DDSEnabler>& enabler);

} /* namespace ddsenabler */
} /* namespace eprosima */
