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
 * @file intDDSEnabler.hpp
 *
 */

#pragma once

#include <cpp_utils/event/FileWatcherHandler.hpp>
#include <cpp_utils/event/MultipleEventHandler.hpp>
#include <cpp_utils/event/PeriodicEventHandler.hpp>
#include <cpp_utils/event/SignalEventHandler.hpp>
#include <cpp_utils/exception/ConfigurationException.hpp>
#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/logging/BaseLogConfiguration.hpp>
#include <cpp_utils/logging/StdLogConsumer.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/types/Fuzzy.hpp>
#include <cpp_utils/utils.hpp>

#include <ddsenabler_yaml/EnablerConfiguration.hpp>
#include <ddsenabler_participants/DDSEnablerLogConsumer.hpp>

#include "ddsenabler/DDSEnabler.hpp"

using namespace eprosima::ddspipe;
using namespace eprosima::ddsenabler;

namespace eprosima {
namespace ddsenabler {

std::unique_ptr<eprosima::utils::event::FileWatcherHandler> create_filewatcher(
        const std::unique_ptr<DDSEnabler>& enabler,
        const std::string& file_path);

std::unique_ptr<eprosima::utils::event::PeriodicEventHandler> create_periodic_handler(
        const std::unique_ptr<DDSEnabler>& enabler,
        const std::string& file_path,
        const eprosima::utils::Duration_ms& reload_time);

int init_dds_enabler(
        const char* ddsEnablerConfigFile,
        participants::DdsNotification data_callback,
        participants::DdsTypeNotification type_callback,
        participants::DdsLogFunc log_callback);


} /* namespace ddsenabler */
} /* namespace eprosima */
