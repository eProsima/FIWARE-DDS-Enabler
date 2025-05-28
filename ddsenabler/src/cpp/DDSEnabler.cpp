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

#include <cpp_utils/utils.hpp>

#include <ddspipe_core/types/dynamic_types/types.hpp>

#include "ddsenabler/DDSEnabler.hpp"
#include "ddsenabler_participants/RpcUtils.hpp"

#include <nlohmann/json.hpp>

namespace eprosima {
namespace ddsenabler {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::participants::rtps;
using namespace eprosima::ddsenabler::participants;
using namespace eprosima::utils;

DDSEnabler::DDSEnabler(
        const yaml::EnablerConfiguration& configuration,
        std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler)
    : configuration_(configuration)
    , event_handler_(event_handler)
{
    // Load the Enabler's internal topics from a configuration object.
    load_internal_topics_(configuration_);

    // Create Discovery Database
    discovery_database_ = std::make_shared<DiscoveryDatabase>();

    // Create Payload Pool
    payload_pool_ = std::make_shared<FastPayloadPool>();

    // Create Thread Pool
    thread_pool_ = std::make_shared<SlotThreadPool>(configuration_.n_threads);

    // Create CB Handler configuration
    participants::CBHandlerConfiguration handler_config;

    // Create DDS Participant
    dds_participant_ = std::make_shared<DdsParticipant>(
        configuration_.simple_configuration,
        payload_pool_,
        discovery_database_);
    dds_participant_->init();

    // Create CB Handler
    cb_handler_ = std::make_shared<participants::CBHandler>(
        handler_config,
        payload_pool_);

    cb_handler_->set_action_send_get_result_request_callback(
        [this](const std::string& action_name, const UUID& action_id)
        {
            if (this->action_send_get_result_request(action_name, action_id))
                return true;
            this->cancel_action_goal(action_name, action_id);
            return false;
        });

    cb_handler_->set_action_send_send_goal_reply_callback(
        [this](const std::string& action_name, const uint64_t goal_id, bool accepted)
        {
            return this->action_send_send_goal_reply(action_name, goal_id, accepted);
        });

    cb_handler_->set_action_send_result_reply_callback(
        [this](const std::string& action_name, const UUID& goal_id, const std::string& reply_json, const uint64_t request_id)
        {
            return this->action_send_result_reply(action_name, goal_id, reply_json, request_id);
        });

    // Create Enabler Participant
    enabler_participant_ = std::make_shared<EnablerParticipant>(
        configuration_.enabler_configuration,
        payload_pool_,
        discovery_database_,
        cb_handler_);

    // Create Participant Database
    participants_database_ = std::make_shared<ParticipantsDatabase>();

    // Populate Participant Database
    participants_database_->add_participant(
        dds_participant_->id(),
        dds_participant_);

    participants_database_->add_participant(
        enabler_participant_->id(),
        enabler_participant_);

    // Create DDS Pipe
    pipe_ = std::make_unique<DdsPipe>(
        configuration_.ddspipe_configuration,
        discovery_database_,
        payload_pool_,
        participants_database_,
        thread_pool_);
}

void DDSEnabler::set_file_watcher(
        const std::string& file_path)
{
    if (file_path.empty())
    {
        EPROSIMA_LOG_WARNING(DDSENABLER_EXECUTION,
                "Error when stablishing file watcher. Configuration file path is empty.");
        return;
    }

    // Callback will reload configuration and pass it to DdsPipe
    // WARNING: it is needed to pass file_path, as FileWatcher only retrieves file_name
    std::function<void(std::string)> file_watcher_callback =
            [this, file_path]
                (std::string file_name)
            {
                EPROSIMA_LOG_INFO(DDSENABLER_EXECUTION,
                        "FileWatcher notified changes in file " << file_path << ". Reloading configuration");
                try
                {
                    eprosima::ddsenabler::yaml::EnablerConfiguration new_configuration(file_path);
                    auto ret = this->reload_configuration(new_configuration);
                    if(ret == utils::ReturnCode::RETCODE_OK)
                        EPROSIMA_LOG_INFO(DDSENABLER_EXECUTION, "Configuration reloaded successfully");
                    else
                        EPROSIMA_LOG_WARNING(DDSENABLER_EXECUTION, "Reloading internal dds pipe configuration from file " << file_path << " failed");
                }
                catch (const std::exception& e)
                {
                    EPROSIMA_LOG_WARNING(DDSENABLER_EXECUTION,
                            "Error reloading configuration file " << file_path << " with error: " << e.what());
                }
            };

    // Creating FileWatcher event handler
    file_watcher_handler_ = std::make_unique<eprosima::utils::event::FileWatcherHandler>(file_watcher_callback, file_path);
}

utils::ReturnCode DDSEnabler::reload_configuration(
        yaml::EnablerConfiguration& new_configuration)
{
    // Load the Enabler's internal topics from a configuration object.
    load_internal_topics_(new_configuration);

    // Update the Enabler's configuration
    configuration_ = new_configuration;

    return pipe_->reload_configuration(new_configuration.ddspipe_configuration);
}

void DDSEnabler::load_internal_topics_(
        yaml::EnablerConfiguration& configuration)
{
    // Create an internal topic to transmit the dynamic types
    configuration.ddspipe_configuration.builtin_topics.insert(
        utils::Heritable<DdsTopic>::make_heritable(type_object_topic()));

    if (!configuration.ddspipe_configuration.allowlist.empty())
    {
        // The allowlist is not empty. Add the internal topic.
        WildcardDdsFilterTopic internal_topic;
        internal_topic.topic_name.set_value(TYPE_OBJECT_TOPIC_NAME);

        configuration.ddspipe_configuration.allowlist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(internal_topic));
    }
}

bool DDSEnabler::publish(
        const std::string& topic_name,
        const std::string& json)
{
    return enabler_participant_->publish(topic_name, json);
}

bool DDSEnabler::send_service_request(
    const std::string& service_name,
    const std::string& json,
    uint64_t& request_id)
{
    sent_request_id_++;
    if (!enabler_participant_->publish_rpc("rq/" + service_name + "Request", json, sent_request_id_))
        return false;

    request_id = sent_request_id_;
    return true;
}

bool DDSEnabler::announce_service(
    const std::string& service_name)
{
    return enabler_participant_->announce_service(service_name);
}

bool DDSEnabler::revoke_service(
    const std::string& service_name)
{
    return enabler_participant_->revoke_service(service_name);
}

bool DDSEnabler::send_service_reply(
    const std::string& service_name,
    const std::string& json,
    const uint64_t request_id)
{
    return enabler_participant_->publish_rpc("rr/" + service_name + "Reply", json, request_id);
}

bool DDSEnabler::send_action_goal(
    const std::string& action_name,
    const std::string& json,
    UUID& action_id)
{
    std::string goal_json = RpcUtils::make_send_goal_request_json(json, action_id);
    uint64_t goal_request_id = 0;
    std::string goal_request_topic = action_name + "send_goal";

    cb_handler_->store_action_request(
        action_id,
        sent_request_id_+1,
        RpcUtils::ActionType::GOAL);

    if(send_service_request(
            goal_request_topic,
            goal_json,
            goal_request_id))
    {
        return true;
    }

    cb_handler_->erase_action_UUID(action_id);

    EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
            "Failed to send action goal to action " << action_name);
    return false;
}

bool DDSEnabler::action_send_get_result_request(
    const std::string& action_name,
    const UUID& action_id)
{
    std::string json = "{\"goal_id\": {\"uuid\": [";
    for (size_t i = 0; i < sizeof(action_id); ++i)
    {
        json += std::to_string(action_id[i]);
        if (i != sizeof(action_id) - 1)
        {
            json += ", ";
        }
    }
    json += "]}}";

    std::string get_result_request_topic = action_name + "get_result";
    uint64_t get_result_request_id = 0;

    cb_handler_->store_action_request(
        action_id,
        sent_request_id_+1,
        RpcUtils::ActionType::RESULT);

    if(send_service_request(
            get_result_request_topic,
            json,
            get_result_request_id))
    {
        return true;
    }

    // TODO cancel?

    EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
            "Failed to send action get result request to action " << action_name);
    return false;
}

bool DDSEnabler::cancel_action_goal(
    const std::string& action_name,
    const participants::UUID& goal_id)
{
    if (!cb_handler_->is_UUID_active(goal_id))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
                "Failed to cancel action goal to action " << action_name
                << ": goal id not found.");
        return false;
    }

    // TODO should we check if the goal_id is already in use?
    // Get current time in seconds and nanoseconds
    auto now = std::chrono::system_clock::now();
    auto duration_since_epoch = now.time_since_epoch();
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration_since_epoch).count();
    auto nanosec = std::chrono::duration_cast<std::chrono::nanoseconds>(duration_since_epoch).count() % 1'000'000'000;

    // Create JSON object
    nlohmann::json j;
    j["goal_info"]["goal_id"]["uuid"] = goal_id;
    j["goal_info"]["stamp"]["sec"] = static_cast<int64_t>(sec);
    j["goal_info"]["stamp"]["nanosec"] = static_cast<uint32_t>(nanosec);
    std::string cancel_json = j.dump(4);

    uint64_t cancel_request_id = 0;
    std::string cancel_request_topic = action_name + "cancel_goal";

    cb_handler_->store_action_request(
        goal_id,
        sent_request_id_+1,
        RpcUtils::ActionType::CANCEL);

    if(send_service_request(
            cancel_request_topic,
            cancel_json,
            cancel_request_id))
    {
        return true;
    }

    EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
            "Failed to send action cancel goal to action " << action_name);
    return false;
}

bool DDSEnabler::announce_action(
    const std::string& action_name)
{
    return enabler_participant_->announce_action(action_name);
}

bool DDSEnabler::revoke_action(
    const std::string& action_name)
{
    return enabler_participant_->revoke_action(action_name);
}

void DDSEnabler::action_send_send_goal_reply(
    const std::string& action_name,
    const uint64_t goal_id,
    bool accepted)
{
    // Get current time in seconds and nanoseconds
    auto now = std::chrono::system_clock::now();
    auto duration_since_epoch = now.time_since_epoch();
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration_since_epoch).count();
    auto nanosec = std::chrono::duration_cast<std::chrono::nanoseconds>(duration_since_epoch).count() % 1'000'000'000;

    // Create JSON object
    nlohmann::json j;
    j["accepted"] = accepted;
    j["stamp"]["sec"] = static_cast<int64_t>(sec);
    j["stamp"]["nanosec"] = static_cast<uint32_t>(nanosec);
    std::string reply_json = j.dump(4);

    if (!send_service_reply(
            action_name + "send_goal",
            reply_json,
            goal_id))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
                "Failed to send action goal reply to action " << action_name
                << ": goal id not found.");
    }
    return;
}

bool DDSEnabler::action_send_result(
    const char* action_name,
    const participants::UUID& goal_id,
    const participants::STATUS_CODE& status_code,
    const char* json)
{
    // Create JSON object
    nlohmann::json j;
    j["status"] = status_code;
    j["result"] = nlohmann::json::parse(json);
    std::string reply_json = j.dump(4);

    return cb_handler_->store_action_result(goal_id, reply_json, action_name);
}

bool DDSEnabler::action_send_result_reply(
    const std::string& action_name,
    const participants::UUID& goal_id,
    const std::string& reply_json,
    const uint64_t request_id)

{
    std::string result_topic = action_name + "get_result";

    if (send_service_reply(
        result_topic,
        reply_json,
        request_id))
    {
        cb_handler_->erase_action_UUID(goal_id);
        return true;
    }

    return false;
}

bool DDSEnabler::action_send_feedback(
    const char* action_name,
    const char* json,
    const participants::UUID& goal_id)
{
    if (!cb_handler_->is_UUID_active(goal_id))
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_EXECUTION,
                "Failed to send action feedback to action " << action_name
                << ": goal id not found.");
        return false;
    }

    // Create JSON object
    nlohmann::json j;
    j["goal_id"]["uuid"] = goal_id;
    j["feedback"] = nlohmann::json::parse(json);
    std::string feedback_json = j.dump(4);
    std::string feedback_topic = "rt/" + std::string(action_name) + "feedback";
    std::cout << " FEEDBACK " << feedback_json << std::endl;
    return enabler_participant_->publish(feedback_topic, feedback_json);
}


} /* namespace ddsenabler */
} /* namespace eprosima */
