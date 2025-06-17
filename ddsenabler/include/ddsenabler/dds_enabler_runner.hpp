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

#include <ddsenabler_yaml/EnablerConfiguration.hpp>

#include <ddsenabler/CallbackSet.hpp>
#include <ddsenabler/DDSEnabler.hpp>

#include <ddsenabler/library/library_dll.h>

namespace eprosima {
namespace ddsenabler {

/**
 * @brief Create a DDS Enabler instance from a configuration file path.
 *
 * @param [in] configuration_path Path to the configuration file.
 * @param [in] callbacks Set of callbacks to be used by the DDS Enabler.
 * @param [out] enabler Output parameter to hold the created DDS Enabler instance.
 * @return true if the DDS Enabler was created successfully, false otherwise.
 */
DDSENABLER_DllAPI
bool create_dds_enabler(
        const char* configuration_path,
        const CallbackSet& callbacks,
        std::shared_ptr<DDSEnabler>& enabler);
/**
 * @brief Create a DDS Enabler instance from a configuration object.
 *
 * @param [in] configuration DDS Enabler configuration object.
 * @param [in] callbacks Set of callbacks to be used by the DDS Enabler.
 * @param [out] enabler Output parameter to hold the created DDS Enabler instance.
 * @return true if the DDS Enabler was created successfully, false otherwise.
 */
DDSENABLER_DllAPI
bool create_dds_enabler(
        yaml::EnablerConfiguration configuration,
        const CallbackSet& callbacks,
        std::shared_ptr<DDSEnabler>& enabler);

} /* namespace ddsenabler */
} /* namespace eprosima */
