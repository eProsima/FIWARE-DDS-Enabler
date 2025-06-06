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
 * @file yaml_configuration_tags.hpp
 */

#pragma once

#include <set>
#include <string>

namespace eprosima {
namespace ddsenabler {
namespace yaml {

constexpr const char* ENABLER_DDS_TAG("dds");
constexpr const char* ENABLER_ENABLER_TAG("ddsenabler");
constexpr const char* ENABLER_INITIAL_PUBLISH_WAIT_TAG("initial-publish-wait");

} /* namespace yaml */
} /* namespace ddsenabler */
} /* namespace eprosima */
