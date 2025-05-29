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
 * @file DDSEnabler.hpp
 */

#pragma once

#include <memory>

#include <cpp_utils/event/FileWatcherHandler.hpp>
#include <cpp_utils/event/MultipleEventHandler.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/dynamic/AllowedTopicList.hpp>
#include <ddspipe_core/dynamic/DiscoveryDatabase.hpp>
#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>
#include <ddspipe_core/types/topic/dds/DistributedTopic.hpp>

#include <ddsenabler_participants/CBCallbacks.hpp>
#include <ddsenabler_participants/CBHandler.hpp>
#include <ddsenabler_participants/CBHandlerConfiguration.hpp>
#include <ddsenabler_participants/DdsParticipant.hpp>
#include <ddsenabler_participants/EnablerParticipant.hpp>

#include <ddsenabler_yaml/EnablerConfiguration.hpp>


namespace eprosima {
namespace ddsenabler {

/**
 * Wrapper class that encapsulates all dependencies required to launch DDS Enabler.
 */
class DDSEnabler
{
public:

    /**
     * DDSEnabler constructor by required values and event handler reference.
     *
     * Creates DDSEnabler instance with given configuration.
     *
     * @param configuration: Structure encapsulating all enabler configuration options.
     * @param event_handler: Reference to event handler used for thread synchronization in main application.
     */
    DDSEnabler(
            const yaml::EnablerConfiguration& configuration,
            std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler);


    void set_data_callback(
            participants::DdsNotification callback)
    {
        cb_handler_->set_data_callback(callback);
    }

    void set_type_callback(
            participants::DdsTypeNotification callback)
    {
        cb_handler_->set_type_callback(callback);
    }

    void set_topic_callback(
            participants::DdsTopicNotification callback)
    {
        cb_handler_->set_topic_callback(callback);
    }

    void set_topic_request_callback(
            participants::DdsTopicRequest callback)
    {
        enabler_participant_->set_topic_request_callback(callback);
    }

    void set_type_request_callback(
            participants::DdsTypeRequest callback)
    {
        cb_handler_->set_type_request_callback(callback);
    }

    void set_service_callback(
            participants::ServiceNotification callback)
    {
        cb_handler_->set_service_callback(callback);
    }

    void set_reply_callback(
            participants::ServiceReplyNotification callback)
    {
        cb_handler_->set_reply_callback(callback);
    }

    void set_request_callback(
            participants::ServiceRequestNotification callback)
    {
        cb_handler_->set_request_callback(callback);
    }

    // TODO rename request of service (rq) to distinguish from request to broker
    void set_service_request_callback(
                participants::ServiceTypeRequest callback)
    {
        enabler_participant_->set_service_request_callback(callback);
    }

    void set_action_callback(
            participants::RosActionNotification callback)
    {
        cb_handler_->set_action_callback(callback);
    }

    void set_action_result_callback(
            participants::RosActionResultNotification callback)
    {
        cb_handler_->set_action_result_callback(callback);
    }

    void set_action_feedback_callback(
            participants::RosActionFeedbackNotification callback)
    {
        cb_handler_->set_action_feedback_callback(callback);
    }

    void set_action_status_callback(
            participants::RosActionStatusNotification callback)
    {
        cb_handler_->set_action_status_callback(callback);
    }

    void set_action_request_callback(
            participants::RosActionTypeRequest callback)
    {
        enabler_participant_->set_action_request_callback(callback);
    }

    void set_action_goal_request_notification_callback(
            participants::RosActionGoalRequestNotification callback)
    {
        cb_handler_->set_action_goal_request_notification_callback(callback);
    }

    /**
     * Associate the file watcher to the configuration file and stablish the callback to reload the configuration.
     *
     * @param file_path: The path to the configuration file.
     */
    void set_file_watcher(
            const std::string& file_path);

    /**
     * Reconfigure the Enabler with the new configuration.
     *
     * @param new_configuration: The configuration to replace the previous configuration with.
     *
     * @return \c RETCODE_OK if allowed topics list has been updated correctly
     * @return \c RETCODE_NO_DATA if new allowed topics list is the same as the previous one
     */
    utils::ReturnCode reload_configuration(
            yaml::EnablerConfiguration& new_configuration);

    // TODO
    bool publish(
            const std::string& topic_name,
            const std::string& json);

    /*****************************************/
    /*               SERVICE                 */
    /*****************************************/

    /**
     * @brief Creates a server for the given service.
     *
     * This function announces a service by setting up a server for it.
     * It returns a boolean indicating whether the operation was successful.
     * Failure may occur if there is an issue requesting the data types to CB
     * for the corresponding request and reply topics.
     *
     * @param service_name The name of the service to be announced.
     * @return true if the service was successfully announced, false otherwise.
     */
    bool announce_service(
            const std::string& service_name);

    /**
     * @brief Stops the server for the given service.
     *
     * This function revokes the server associated with the specified service.
     * It returns a boolean indicating whether the operation was successful.
     * The operation will fail if the service was not previously announced.
     *
     * @param service_name The name of the service to be stopped.
     * @return true if the service was successfully stopped, false otherwise.
     */
    bool revoke_service(
            const std::string& service_name);

    /**
     * @brief Sends a request to a specified service.
     *
     * Sends a request containing the given JSON data to the specified service.
     * If successful, the request identifier is stored in the provided reference
     * and will match the one received in the reply callback.
     *
     * The function fails if there is no server for the service or if the request is malformed.
     * The request identifier is incremented sequentially across all services.
     *
     * @param service_name The target service name.
     * @param json The JSON-formatted request data.
     * @param request_id Reference to store the unique request identifier.
     * @return true if the request was successfully sent, false otherwise.
     */
    bool send_service_request(
            const std::string& service_name,
            const std::string& json,
            uint64_t& request_id);

    /**
     * @brief Sends a reply to the given service.
     *
     * This function sends a reply to the specified service with the provided JSON data.
     * It returns a boolean indicating whether the operation was successful.
     * Failure may occur if the service was not previously created or if the request ID does not match any request.
     *
     * @param service_name The name of the service to send the reply to.
     * @param json The JSON data to be sent with the reply.
     * @param request_id The unique identifier of the request to which this reply corresponds.
     * @return true if the reply was successfully sent, false otherwise.
     *
     * @note The request_id must coincide with the one received in the request.
     */
    bool send_service_reply(
            const std::string& service_name,
            const std::string& json,
            const uint64_t request_id);

    /*****************************************/
    /*            ACTION SERVER              */
    /*****************************************/

    /**
     * @brief Creates server for the given action.
     *
     * This function announces an action by setting up a server for it.
     * It returns a boolean indicating whether the operation was successful.
     * Failure may occur if there is an issue requesting the data types to CB.
     *
     * @param action_name The name of the action to be announced.
     *
     * @return true if the action was successfully announced, false otherwise.
     */
    bool announce_action(
            const std::string& action_name);

    /**
     * @brief Stops the server for the given action.
     *
     * This function revokes the server associated with the specified action.
     * It returns a boolean indicating whether the operation was successful.
     * Failure may occur if the action was not previously announced.
     *
     * @param action_name The name of the action to be stopped.
     *
     * @return true if the action was successfully stopped, false otherwise.
     */
    bool revoke_action(
            const std::string& action_name);

    /**
     * @brief Send feedback for the specified goal_id action.
     *
     * This function sends feedback for the specified action goal identified by the given goal ID.
     * It returns a boolean indicating whether the operation was successful.
     *
     * @param action_name The name of the action for which the feedback is being sent.
     * @param json The JSON data to be sent as feedback.
     * @param goal_id The unique identifier of the action goal for which the feedback is being sent.
     *
     * @return true if the feedback was successfully sent, false otherwise.
     */
    bool send_action_feedback(
        const char* action_name,
        const char* json,
        const participants::UUID& goal_id);

    /**
     * @brief Send result for the specified goal_id action.
     *
     * This function sends the result for the specified action goal identified by the given goal ID.
     * It returns a boolean indicating whether the operation was successful.
     *
     * @param action_name The name of the action for which the result is being sent.
     * @param goal_id The unique identifier of the action goal for which the result is being sent.
     * @param status_code The status code representing the result of the action.
     * @param json The JSON data to be sent as the result.
     *
     * @return true if the result was successfully sent, false otherwise.
     */
    bool send_action_result(
        const char* action_name,
        const participants::UUID& goal_id,
        const participants::STATUS_CODE& status_code,
        const char* json);

     /**
      * @brief Publishes an update for the status of an action.
      *
      * This function publishes a msg update for the status of an action.
      * It returns a boolean indicating whether the operation was successful.
      * Failure may occur if the action was not previously announced or if the goal ID is not currently active.
      *
      * @param action_name The name of the action for which the status is being updated.
      * @param goal_id The unique identifier of the action goal for which the status is being updated.
      * @param status_code The status code representing the current state of the action.
      *
      * @return true if the status update was successfully sent, false otherwise.
      *
     */
    bool update_action_status(
        const std::string& action_name,
        const participants::UUID& goal_id,
        const participants::STATUS_CODE& status_code);

    /*****************************************/
    /*            ACTION CLIENT              */
    /*****************************************/

    /**
     * @brief Sends an action goal to the specified action.
     *
     * This function sends an action goal to the specified action with the provided JSON data.
     * It returns a boolean indicating whether the operation was successful.
     * Failure may occur if the action server was not previously created, if the action types are unknown
     * or if the action goal is malformed.
     *
     * The goal_id is a unique identifier for the action goal and is generated by the function.
     *
     * @param action_name The name of the action to send the goal to.
     * @param json The JSON data to be sent with the action goal.
     * @param goal_id Reference to store the unique identifier of the action goal.
     * @return true if the action goal was successfully sent, false otherwise.
     */
    bool send_action_goal(
	    const std::string& action_name,
	    const std::string& json,
            participants::UUID& goal_id);

    /**
     * @brief Cancels an action goal for the specified action.
     *
     * This function cancels an action goal identified by the given goal ID for the specified action.
     * It returns a boolean indicating whether the operation was successful.
     * Failure may occur if the goal ID is not valid or if the action server was not previously created.
     * The success of the operation only indicates that the cancel request was sent, not that it was processed.
     * The actual cancellation of the goal would be notified via a status update.
     *
     * @param action_name The name of the action for which the goal is to be canceled.
     * @param goal_id The unique identifier of the action goal to be canceled.
     *
     * @return true if the cancel action request was successfully canceled, false otherwise.
     */
    bool cancel_action_goal(
            const std::string& action_name,
            const participants::UUID& goal_id);

protected:

    /**
     * @brief Sends a request to get the result of an action.
     *
     * This function sends a request to get the result of an action identified by the given action name and goal ID.
     * It returns a boolean indicating whether the operation was successful.
     *
     * It is invoked via lambda from the cb_writer_ when the send_goal_reply is received and positive.
     *
     * @param action_name The name of the action for which the result is being requested.
     * @param goal_id The unique identifier of the action goal for which the result is being requested.
     *
     * @return true if the request to get the result was successfully sent, false otherwise.
     */
    bool send_action_get_result_request(
        const std::string& action_name,
        const participants::UUID& goal_id);

    /**
     * @brief Sends a reply to an action goal request.
     *
     * This function sends a reply to an action goal request, indicating whether the goal was accepted or not.
     *
     * It is invoked via lambda from the cb_writer_ when request is notified.
     *
     * @param action_name The name of the action for which the goal reply is being sent.
     * @param goal_id The unique identifier of the action goal for which the reply is being sent.
     * @param accepted A boolean indicating whether the goal was accepted (true) or rejected (false).
     */
    void send_action_send_goal_reply(
        const std::string& action_name,
        const uint64_t goal_id,
        bool accepted);

    /**
     * @brief Actually sends the result reply for an action.
     *
     * This function is only called when all the conditions are met to send the result reply.
     *
     * @param action_name The name of the action for which the result reply is being sent.
     * @param goal_id The unique identifier of the action goal for which the result reply is being sent.
     * @param reply_json The JSON data to be sent as the result reply.
     * @param request_id The unique identifier of the request to which this reply corresponds.
     */
    bool send_action_get_result_reply(
        const std::string& action_name,
        const participants::UUID& goal_id,
        const std::string& reply_json,
        const uint64_t request_id);

    /**
     * Load the Enabler's internal topics into a configuration object.
     *
     * @param configuration: The configuration to load the internal topics into.
     */
    void load_internal_topics_(
            yaml::EnablerConfiguration& configuration);

    //! Configuration of the DDS Enabler
    yaml::EnablerConfiguration configuration_;

    //! Payload Pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Discovery Database
    std::shared_ptr<ddspipe::core::DiscoveryDatabase> discovery_database_;

    //! Participants Database
    std::shared_ptr<ddspipe::core::ParticipantsDatabase> participants_database_;

    //! Thread Pool
    std::shared_ptr<utils::SlotThreadPool> thread_pool_;

    //! CB Handler
    std::shared_ptr<eprosima::ddsenabler::participants::CBHandler> cb_handler_;

    //! DDS Participant
    std::shared_ptr<eprosima::ddsenabler::participants::DdsParticipant> dds_participant_;

    //! Enabler Participant
    std::shared_ptr<eprosima::ddsenabler::participants::EnablerParticipant> enabler_participant_;

    //! DDS Pipe
    std::unique_ptr<ddspipe::core::DdsPipe> pipe_;

    //! Reference to event handler used for thread synchronization in main application
    std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler_;

    //! Config File watcher handler
    std::unique_ptr<eprosima::utils::event::FileWatcherHandler> file_watcher_handler_;

    //! Request identifyer for sent requests (incremented by one after each request)
    uint64_t sent_request_id_ = 0;
};

} /* namespace ddsenabler */
} /* namespace eprosima */
