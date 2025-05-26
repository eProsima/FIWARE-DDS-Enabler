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

#include <string>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/logging/StdLogConsumer.hpp>

#include <ddsenabler_participants/CBCallbacks.hpp>
#include <ddsenabler_participants/DDSEnablerLogConsumer.hpp>

#include "ddsenabler/dds_enabler_runner.hpp"

using namespace eprosima::ddspipe;

namespace eprosima {
namespace ddsenabler {

bool create_dds_enabler(
        const char* ddsEnablerConfigFile,
        participants::ddsCallbacks& dds_callbacks,
        participants::serviceCallbacks& service_callbacks,
        participants::actionCallbacks& action_callbacks,
        std::unique_ptr<DDSEnabler>& enabler)
{
    std::string dds_enabler_config_file = "";
    if (ddsEnablerConfigFile != NULL)
    {
        dds_enabler_config_file = ddsEnablerConfigFile;
    }
    // Load configuration from file
    eprosima::ddsenabler::yaml::EnablerConfiguration configuration(dds_enabler_config_file);

    bool ret = create_dds_enabler(
        configuration,
        dds_callbacks,
        service_callbacks,
        action_callbacks,
        enabler);


    if(ret)
    {
        enabler->set_file_watcher(dds_enabler_config_file);
    }

    return ret;
}

bool create_dds_enabler(
        yaml::EnablerConfiguration configuration,
        participants::ddsCallbacks& dds_callbacks,
        participants::serviceCallbacks& service_callbacks,
        participants::actionCallbacks& action_callbacks,
        std::unique_ptr<DDSEnabler>& enabler)
{
    // Encapsulating execution in block to erase all memory correctly before closing process
    try
    {
        // Verify that the configuration is correct
        eprosima::utils::Formatter error_msg;
        if (!configuration.is_valid(error_msg))
        {
            throw eprosima::utils::ConfigurationException(
                      eprosima::utils::Formatter() << "Invalid configuration: " << error_msg);
        }

        // Logging
        if(dds_callbacks.log_callback != nullptr){
            // Disable stdout always
            configuration.ddspipe_configuration.log_configuration.stdout_enable = false;
            const auto log_configuration = configuration.ddspipe_configuration.log_configuration;

            eprosima::utils::Log::ClearConsumers();
            eprosima::utils::Log::SetVerbosity(log_configuration.verbosity);

            // DDS Enabler Log Consumer
            auto* log_consumer = new eprosima::ddsenabler::participants::DDSEnablerLogConsumer(&log_configuration);
            log_consumer->set_log_callback(dds_callbacks.log_callback);

            eprosima::utils::Log::RegisterConsumer(
                std::unique_ptr<eprosima::ddsenabler::participants::DDSEnablerLogConsumer>(log_consumer));

            // Std Log Consumer
            if (log_configuration.stdout_enable)
            {
                eprosima::utils::Log::RegisterConsumer(
                    std::make_unique<eprosima::utils::StdLogConsumer>(&log_configuration));
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
        enabler->set_data_callback(dds_callbacks.data_callback);
        enabler->set_type_callback(dds_callbacks.type_callback);
        enabler->set_topic_callback(dds_callbacks.topic_callback);
        enabler->set_type_request_callback(dds_callbacks.type_req_callback);
        enabler->set_topic_request_callback(dds_callbacks.topic_req_callback);

        enabler->set_service_callback(service_callbacks.service_callback);
        enabler->set_reply_callback(service_callbacks.reply_callback);
        enabler->set_request_callback(service_callbacks.request_callback);
        enabler->set_service_request_callback(service_callbacks.type_req_callback);

        enabler->set_action_callback(action_callbacks.action_callback);
        enabler->set_action_result_callback(action_callbacks.result_callback);
        enabler->set_action_feedback_callback(action_callbacks.feedback_callback);
        enabler->set_action_status_callback(action_callbacks.status_callback);
        enabler->set_action_request_callback(action_callbacks.type_req_callback);
        // TODO rename the notifications when rebased
        enabler->set_action_goal_request_notification_callback(action_callbacks.goal_request_callback);

        EPROSIMA_LOG_INFO(DDSENABLER_EXECUTION,
                "DDS Enabler running.");
    }
    catch (const eprosima::utils::ConfigurationException& e)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
                "Error Loading DDS Enabler Configuration. Error message:\n " << e.what());
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
