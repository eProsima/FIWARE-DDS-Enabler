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

using namespace eprosima::ddspipe;

namespace eprosima {
namespace ddsenabler {

std::unique_ptr<eprosima::utils::event::FileWatcherHandler> create_filewatcher(
        const std::unique_ptr<DDSEnabler>& enabler,
        const std::string& file_path)
{
    // Callback will reload configuration and pass it to DdsPipe
    // WARNING: it is needed to pass file_path, as FileWatcher only retrieves file_name
    std::function<void(std::string)> filewatcher_callback =
            [&enabler, &file_path]
            (std::string file_name)
            {
                logInfo(DDSENABLER_EXECUTION,
                        "FileWatcher notified changes in file " << file_path << ". Reloading configuration");
                try
                {
                    eprosima::ddsenabler::yaml::EnablerConfiguration new_configuration(file_path);
                    enabler->reload_configuration(new_configuration);
                }
                catch (const std::exception& e)
                {
                    logWarning(DDSENABLER_EXECUTION,
                            "Error reloading configuration file " << file_path << " with error: " << e.what());
                }
            };

    // Creating FileWatcher event handler
    return std::make_unique<eprosima::utils::event::FileWatcherHandler>(filewatcher_callback, file_path);
}

// std::unique_ptr<eprosima::utils::event::PeriodicEventHandler> create_periodic_handler(
//         const std::unique_ptr<DDSEnabler>& enabler,
//         const std::string& file_path,
//         const eprosima::utils::Duration_ms& reload_time)
// {
//     // Callback will reload configuration and pass it to DdsPipe
//     std::function<void()> periodic_callback =
//             [&enabler, &file_path]
//             ()
//             {
//                 logInfo(DDSENABLER_EXECUTION,
//                         "Periodic Timer raised. Reloading configuration from file " << file_path << ".");
//                 try
//                 {
//                     eprosima::ddsenabler::yaml::EnablerConfiguration new_configuration(file_path);
//                     enabler->reload_configuration(new_configuration);
//                 }
//                 catch (const std::exception& e)
//                 {
//                     logWarning(DDSENABLER_EXECUTION,
//                             "Error reloading configuration file " << file_path << " with error: " << e.what());
//                 }
//             };

//     // Creating periodic handler
//     return std::make_unique<eprosima::utils::event::PeriodicEventHandler>(periodic_callback, reload_time);
// }

int init_dds_enabler(
        const char* ddsEnablerConfigFile)
{
    logInfo(DDSENABLER_EXECUTION,
            "Starting DDS Enabler execution.");

    CONFIGURATION_FILE = ddsEnablerConfigFile;

    // Encapsulating execution in block to erase all memory correctly before closing process
    try
    {
        // Load configuration from YAML
        eprosima::ddsenabler::yaml::EnablerConfiguration configuration(CONFIGURATION_FILE);

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

            // DDS Enabler Log Consumer
            eprosima::utils::Log::RegisterConsumer(
                std::make_unique<eprosima::ddsenabler::participants::DDSEnablerLogConsumer>(&log_configuration));

            // Std Log Consumer
            if (log_configuration.stdout_enable)
            {
                eprosima::utils::Log::RegisterConsumer(
                    std::make_unique<eprosima::utils::StdLogConsumer>(&log_configuration));
            }

            eprosima::utils::Log::SetVerbosity(log_configuration.verbosity);
        }

        // DDS Enabler Initialization

        // Create a multiple event handler that handles all events that make the enabler stop
        auto close_handler = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

        // Start recording right away
        auto enabler = std::make_unique<DDSEnabler>(configuration, close_handler);

        logInfo(DDSENABLER_EXECUTION,
                "DDS Enabler running.");

        // Create File Watcher Handler
        std::unique_ptr<eprosima::utils::event::FileWatcherHandler> file_watcher_handler;
        file_watcher_handler = create_filewatcher(enabler, CONFIGURATION_FILE);

        // // Create Periodic Handler
        // std::unique_ptr<eprosima::utils::event::PeriodicEventHandler> periodic_handler;
        // periodic_handler = create_periodic_handler(enabler, CONFIGURATION_FILE, 10);

        // Wait until signal arrives
        close_handler->wait_for_event();

        logInfo(DDSENABLER_EXECUTION,
                "Stopping DDS Enabler.");

        logInfo(DDSENABLER_EXECUTION,
                "DDS Enabler stopped correctly.");
    }
    catch (const eprosima::utils::ConfigurationException& e)
    {
        logError(DDSENABLER_ERROR,
                "Error Loading DDS Enabler Configuration from file " << CONFIGURATION_FILE <<
                ". Error message:\n " << e.what());
        return -1;
    }
    catch (const eprosima::utils::InitializationException& e)
    {
        logError(DDSENABLER_ERROR,
                "Error Initializing DDS Enabler. Error message:\n " << e.what());
        return -1;
    }

    logInfo(DDSENABLER_EXECUTION,
            "Finishing DDS Enabler execution correctly.");

    // Force print every log before closing
    eprosima::utils::Log::Flush();

    // Delete the consumers before closing
    eprosima::utils::Log::ClearConsumers();

    return 0;
}

} /* namespace ddsenabler */
} /* namespace eprosima */
