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
 * @file EnablerConfiguration.hpp
 */

#pragma once

#include <nlohmann/json.hpp>

#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/memory/Heritable.hpp>

#include <ddspipe_core/configuration/DdsPipeConfiguration.hpp>
#include <ddspipe_core/configuration/MonitorConfiguration.hpp>
#include <ddspipe_core/types/dds/TopicQoS.hpp>
#include <ddspipe_core/types/topic/dds/DistributedTopic.hpp>
#include <ddspipe_core/types/topic/filter/IFilterTopic.hpp>

#include <ddspipe_participants/configuration/ParticipantConfiguration.hpp>
#include <ddspipe_participants/configuration/SimpleParticipantConfiguration.hpp>

#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlReader.hpp>

#include <ddsenabler_yaml/library/library_dll.h>

namespace eprosima {
namespace ddsenabler {
namespace yaml {

constexpr const int DEFAULT_N_THREADS = 8;

/**
 * @brief Class that encapsulates specific methods to get a full DDS Enabler Configuration from a yaml node.
 *
 * TODO: Add version configuration so it could load different versions
 */
class DDSENABLER_YAML_DllAPI EnablerConfiguration
{
public:

    EnablerConfiguration(
            bool is_json,
            const nlohmann::json& json);

    EnablerConfiguration(
            const Yaml& yml);

    EnablerConfiguration(
            const std::string& file_path);

    virtual bool is_valid(
            utils::Formatter& error_msg) const noexcept;

    // DDS Pipe Configuration
    ddspipe::core::DdsPipeConfiguration ddspipe_configuration;

    // Participants configurations
    std::shared_ptr<ddspipe::participants::SimpleParticipantConfiguration> simple_configuration;
    std::shared_ptr<ddspipe::participants::ParticipantConfiguration> enabler_configuration;

    unsigned int n_threads = DEFAULT_N_THREADS;

    ddspipe::core::types::TopicQoS topic_qos{};

protected:

    YAML::Node json_to_yaml(
            const nlohmann::json& json);

    void load_ddsenabler_configuration_(
            const Yaml& yml);

    void load_enabler_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_specs_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_dds_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_ddsenabler_configuration_from_file_(
            const std::string& file_path);
};

} /* namespace yaml */
} /* namespace ddsenabler */
} /* namespace eprosima */
