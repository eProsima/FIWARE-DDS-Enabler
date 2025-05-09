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
 * @file dds_enabler_runner.cpp
 *
 */

#include <fstream>
#include <string>

#include "fastdds/dds/log/FileConsumer.hpp"

#include "ddsenabler/dds_enabler_runner.hpp"

#include <cpp_utils/Log.hpp>

using namespace eprosima::ddspipe;

namespace eprosima {
namespace ddsenabler {

bool create_dds_enabler(
        const char* ddsEnablerConfigFile,
        participants::DdsNotification data_callback,
        participants::DdsTypeNotification type_callback,
        participants::DdsLogFunc log_callback,
        std::unique_ptr<DDSEnabler>& enabler)
{
    std::string dds_enabler_config_file = "";
    if (ddsEnablerConfigFile != NULL)
    {
        dds_enabler_config_file = ddsEnablerConfigFile;
    }

    // Encapsulating execution in block to erase all memory correctly before closing process
    try
    {
        // Load configuration from file
        eprosima::ddsenabler::yaml::EnablerConfiguration configuration(dds_enabler_config_file);

        // Verify that the configuration is correct
        eprosima::utils::Formatter error_msg;
        if (!configuration.is_valid(error_msg))
        {
            throw eprosima::utils::ConfigurationException(
                      eprosima::utils::Formatter() << "Invalid configuration: " << error_msg);
        }

        // Logging
        {
            const auto log_configuration = configuration.ddspipe_configuration.log_configuration;

            eprosima::utils::Log::ClearConsumers();
            eprosima::utils::Log::SetVerbosity(log_configuration.verbosity);

            if (log_callback)
            {
                // User callback Log Consumer
                auto* log_consumer = new eprosima::ddsenabler::participants::DDSEnablerLogConsumer(&log_configuration);
                log_consumer->set_log_callback(log_callback);

                eprosima::utils::Log::RegisterConsumer(
                    std::unique_ptr<eprosima::ddsenabler::participants::DDSEnablerLogConsumer>(log_consumer));
            }

            // Std Log Consumer
            if (log_configuration.stdout_enable)
            {
                eprosima::utils::Log::RegisterConsumer(
                    std::make_unique<eprosima::utils::StdLogConsumer>(&log_configuration));
            }

            // DDS Log Consumer
            if (log_configuration.publish.enable)
            {
                eprosima::utils::Log::RegisterConsumer(
                    std::make_unique<eprosima::ddspipe::core::DdsLogConsumer>(&log_configuration));
            }
        }

        // DDS Enabler Initialization
        EPROSIMA_LOG_INFO(DDSENABLER_EXECUTION,
                "Starting DDS Enabler execution.");

        // Create a multiple event handler that handles all events that make the enabler stop
        auto close_handler = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

        // Create DDSEnabler and set the context broker callbacks
        enabler.reset(new DDSEnabler(configuration, close_handler));

        // TODO: avoid setting callback after having created "enabled" enabler (e.g. pass and set in construction)
        enabler->set_data_callback(data_callback);
        enabler->set_type_callback(type_callback);

        // Set the file watcher to reload the configuration if the file changes
        if (!enabler->set_file_watcher(dds_enabler_config_file))
        {
            EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
                    "Failed to set file watcher.");
            return false;
        }

        EPROSIMA_LOG_INFO(DDSENABLER_EXECUTION,
                "DDS Enabler running.");
    }
    catch (const eprosima::utils::ConfigurationException& e)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
                "Error Loading DDS Enabler Configuration from file " << dds_enabler_config_file <<
                ". Error message:\n " << e.what());
        return false;
    }
    catch (const eprosima::utils::InitializationException& e)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
                "Error Initializing DDS Enabler. Error message:\n " << e.what());
        return false;
    }

    return true;
}

} /* namespace ddsenabler */
} /* namespace eprosima */
