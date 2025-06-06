// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#pragma once

#include <mutex>

#include <ddsenabler/dds_enabler_runner.hpp>
#include <ddsenabler_participants/RpcUtils.hpp>

using namespace eprosima::ddspipe;
using namespace eprosima::ddsenabler;
using namespace eprosima::ddsenabler::participants;
using namespace eprosima::fastdds::dds;

#define TEST_SERVICE_NAME "add_two_ints"
#define TEST_ACTION_NAME "fibonacci/_action/"

#define TEST_FILE_DIRECTORY "/ddsenabler/src/ddsenabler_test_app/test_files/"
#define TEST_SERVICE_FILE "test_service.json"
#define TEST_ACTION_FILE "test_action.json"

namespace tester {

const unsigned int DOMAIN_ = 33;
static int num_samples_ =  1;
static int wait_after_writer_creation_ms_ =  300;
static int write_delay_ms_ =  20;
static int wait_for_ack_ns_ =  1000000000;
static int wait_after_publication_ms_ =  200;

class Tester
{
public:

    // Create the DDSEnabler and bind the static callbacks
    std::shared_ptr<DDSEnabler> create_ddsenabler()
    {
        YAML::Node yml;

        eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);
        configuration.simple_configuration->domain = DOMAIN_;

        CallbackSet callbacks{
            .log = test_log_callback,
            .dds = {
                .type_notification = test_type_notification_callback,
                .topic_notification = test_topic_notification_callback,
                .data_notification = test_data_notification_callback,
                .type_query = test_type_query_callback,
                .topic_query = test_topic_query_callback
            },
            .service = {
                .service_notification = test_service_notification_callback,
                .service_request_notification = test_service_request_notification_callback,
                .service_reply_notification = test_service_reply_notification_callback,
                .service_query = test_service_query_callback
            },
            .action = {
                .action_notification = test_action_notification_callback,
                .action_goal_request_notification = test_action_goal_request_notification_callback,
                .action_feedback_notification = test_action_feedback_notification_callback,
                .action_cancel_request_notification = test_action_cancel_request_notification_callback,
                .action_result_notification = test_action_result_notification_callback,
                .action_status_notification = test_action_status_notification_callback,
                .action_query = test_action_query_callback
            }
        };

        // Create DDS Enabler
        std::shared_ptr<DDSEnabler> enabler;
        bool result = create_dds_enabler(configuration, callbacks, enabler);

        return enabler;
    }

    bool wait_for_service_notification(
        uint16_t expected_services,
        int timeout = 10)
    {
        std::unique_lock<std::mutex> lock(current_test_instance_->service_mutex_);
        if (cv_.wait_for(lock, std::chrono::seconds(timeout), [this, expected_services]()
                {
                    return received_services_ >= expected_services;
                }))
        {
            return true;
        }
        std::cout << "Timeout waiting for service notification" << std::endl;
        return false;
    }

    bool wait_for_service_reply_notification(
            uint64_t& request_id,
            int timeout = 10)
    {
        std::unique_lock<std::mutex> lock(dds_received_mutex_);
        if (cv_.wait_for(lock, std::chrono::seconds(timeout), [this]()
                {
                    return received_reply_ != 0;
                }))
        {
            request_id = received_reply_;
            received_reply_ = 0; // Reset the reply ID after receiving it
            return true;
        }
        std::cout << "Timeout waiting for reply callback" << std::endl;
        return false;
    }

    int get_received_request_id()
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->dds_received_mutex_);

            return current_test_instance_->received_request_id_;
        }
        else
        {
            return 0;
        }
    }

    bool wait_for_service_query(
            uint16_t expected_queries,
            int timeout = 10)
    {
        std::unique_lock<std::mutex> lock(service_mutex_);
        if (cv_.wait_for(lock, std::chrono::seconds(timeout), [this, expected_queries]()
                {
                    return received_services_request_ >= expected_queries;
                }))
        {
            return true;
        }
        std::cout << "Timeout waiting for service query" << std::endl;
        return false;
    }

    bool wait_for_service_request(
            const std::string& service_name,
            uint64_t& request_id,
            int timeout = 10)
    {
        std::unique_lock<std::mutex> lock(dds_received_mutex_);
        if (cv_.wait_for(lock, std::chrono::seconds(timeout), [this, service_name]()
                {
                    return received_request_id_ > 0;
                }))
        {
            request_id = received_request_id_;
            received_request_id_ = 0; // Reset the request ID after receiving it
            return true;
        }
        else
        {
            std::cout << "Timeout waiting for request callback" << std::endl;
            return false;
        }
    }

    bool wait_for_action_notification(
        uint16_t expected_actions,
        int timeout = 10)
    {
        std::unique_lock<std::mutex> lock(current_test_instance_->action_mutex_);
        if (cv_.wait_for(lock, std::chrono::seconds(timeout), [this, expected_actions]()
                {
                    return received_actions_ >= expected_actions;
                }))
        {
            return true;
        }
        std::cout << "Timeout waiting for action notification" << std::endl;
        return false;
    }

    bool wait_for_status_update(
            const UUID& action_id,
            eprosima::ddsenabler::participants::STATUS_CODE& status,
            const eprosima::ddsenabler::participants::STATUS_CODE& expected_status,
            int timeout = 10)
    {
        std::unique_lock<std::mutex> lock(dds_received_mutex_);
        if (cv_.wait_for(lock, std::chrono::seconds(timeout), [this, expected_status]()
                {
                    return last_status_ == expected_status;
                }))
        {
            status = last_status_;
            last_status_ = eprosima::ddsenabler::participants::STATUS_CODE::STATUS_UNKNOWN; // Reset the status after receiving it
            return true;
        }

        std::cout << "Timeout waiting for status update" << std::endl;
        status = eprosima::ddsenabler::participants::STATUS_CODE::STATUS_UNKNOWN;
        return false;
    }

    bool wait_for_result(
            UUID& action_id,
            int timeout = 10)
    {
        std::unique_lock<std::mutex> lock(dds_received_mutex_);
        if (cv_.wait_for(lock, std::chrono::seconds(timeout), [this]()
                {
                    return received_actions_result_ == true;
                }))
        {
            action_id = actions_goal_uuid_;
            received_actions_result_ = false;
            return true;
        }
        else
        {
            std::cout << "Timeout waiting for result callback" << std::endl;
            return false;
        }
    }

    bool wait_for_feedback(
            int& feedbacks_prev,
            UUID& action_id,
            int timeout = 10)
    {
        std::unique_lock<std::mutex> lock(dds_received_mutex_);
        if (cv_.wait_for(lock, std::chrono::seconds(timeout), [this, feedbacks_prev]()
                {
                    return received_actions_feedback_ > feedbacks_prev;
                }))
        {
            action_id = actions_feedback_uuid_;
            feedbacks_prev = received_actions_feedback_;
            return true;
        }
        else
        {
            std::cout << "Timeout waiting for feedback callback" << std::endl;
            return false;
        }
    }

    bool wait_for_action_request_notification(
            uint64_t& fibonacci_number,
            UUID& action_id,
            int timeout = 10)
    {
        std::unique_lock<std::mutex> lock(action_mutex_);
        if (cv_.wait_for(lock, std::chrono::seconds(timeout), [this]()
                {
                    return action_goal_requests_.size() > 0;
                }))
        {
            auto request = action_goal_requests_.back();
            action_id = request.first;
            fibonacci_number = request.second;
            action_goal_requests_.pop_back();
            return true;
        }
        else
        {
            std::cout << "Timeout waiting for action request callback" << std::endl;
            return false;
        }
    }

    bool wait_for_action_cancel_request(
            uint64_t& request_id,
            int timeout = 10)
    {
        std::unique_lock<std::mutex> lock(dds_received_mutex_);
        if (cv_.wait_for(lock, std::chrono::seconds(timeout), [this]()
                {
                    return received_actions_cancel_request_id_ != 0;
                }))
        {
            request_id = received_actions_cancel_request_id_;
            received_actions_cancel_request_id_ = 0; // Reset the cancel request after receiving it
            return true;
        }
        else
        {
            std::cout << "Timeout waiting for action cancel request callback" << std::endl;
            return false;
        }
    }

    // eprosima::ddsenabler::participants::DdsDataNotification data_notification;
    static void test_data_notification_callback(
            const char* topic_name,
            const char* json,
            int64_t publish_time)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->dds_received_mutex_);

            current_test_instance_->received_data_++;
            std::cout << "Data callback received: " << topic_name << ", Total data: " <<
                current_test_instance_->received_data_ << std::endl;
        }
    }

    // eprosima::ddsenabler::participants::DdsTypeNotification type_notification;
    static void test_type_notification_callback(
            const char* type_name,
            const char* serialized_type,
            const unsigned char* serialized_type_internal,
            uint32_t serialized_type_internal_size,
            const char* data_placeholder)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->dds_received_mutex_);

            current_test_instance_->received_types_++;
            std::cout << "Type callback received: " << type_name << ", Total types: " <<
                current_test_instance_->received_types_ << std::endl;

            eprosima::ddsenabler::participants::RpcUtils::save_type_to_file(
                TEST_FILE_DIRECTORY,
                type_name,
                serialized_type_internal,
                serialized_type_internal_size);
        }
    }

    // eprosima::ddsenabler::participants::DdsTopicNotification topic_notification
    static void test_topic_notification_callback(
            const char* topic_name,
            const char* type_name,
            const char* serialized_qos)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->dds_received_mutex_);

            current_test_instance_->received_topics_++;
            std::cout << "Topic callback received: " << topic_name << ", Total topics: " <<
                current_test_instance_->received_topics_ << std::endl;
        }
    }

    // eprosima::ddsenabler::participants::DdsTopicQuery topic_query;
    static bool test_topic_query_callback(
            const char* topic_name,
            std::string& type_name,
            std::string& serialized_qos)
    {
        return true;
    }

    // eprosima::ddsenabler::participants::DdsTypeQuery type_query;
    static bool test_type_query_callback(
            const char* type_name,
            std::unique_ptr<const unsigned char []>& serialized_type_internal,
            uint32_t& serialized_type_internal_size)
    {
        return eprosima::ddsenabler::participants::RpcUtils::load_type_from_file(
            TEST_FILE_DIRECTORY,
            type_name,
            serialized_type_internal,
            serialized_type_internal_size);
    }


    //eprosima::ddsenabler::participants::DdsLogFunc log_callback;
    static void test_log_callback(
            const char* fileName,
            int lineNo,
            const char* funcName,
            int category,
            const char* msg)
    {
    }

    // eprosima::ddsenabler::participants::ServiceNotification service_notification_callback;
    static void test_service_notification_callback(
            const char* service_name,
            const char* request_type_name,
            const char* reply_type_name,
            const char* request_serialized_qos,
            const char* reply_serialized_qos)
    {
        if (current_test_instance_ && TEST_SERVICE_NAME == std::string(service_name))
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->service_mutex_);

            current_test_instance_->received_services_++;
            std::cout << "Service callback received: " << service_name << ", Total services: " <<
                current_test_instance_->received_services_ << std::endl;

            std::string service_file = TEST_FILE_DIRECTORY + std::string(TEST_SERVICE_FILE);
            eprosima::ddsenabler::participants::RpcUtils::save_service_to_file(service_name, request_type_name, reply_type_name,
                    request_serialized_qos, reply_serialized_qos, service_file);

            current_test_instance_->cv_.notify_all();
        }
    }

    // eprosima::ddsenabler::participants::ServiceReplyNotification reply_notification_callback;
    static void test_service_reply_notification_callback(
            const char* service_name,
            const char* json,
            uint64_t request_id,
            int64_t publish_time)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->dds_received_mutex_);

            current_test_instance_->received_reply_ = request_id;
            std::cout << "Reply callback received with id: " << request_id << " for service: " << service_name << std::endl;
            current_test_instance_->cv_.notify_all();
        }
    }

    // eprosima::ddsenabler::participants::ServiceRequestNotification request_notification_callback;
    static void test_service_request_notification_callback(
            const char* service_name,
            const char* json,
            uint64_t request_id,
            int64_t publish_time)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->dds_received_mutex_);

            current_test_instance_->received_request_id_ = request_id;
            std::cout << "Request callback received with id: " << request_id << " for service: " << service_name << std::endl;
            current_test_instance_->cv_.notify_all();
        }
    }

    // eprosima::ddsenabler::participants::ServiceQuery type_query_callback;
    static bool test_service_query_callback(
            const char* service_name,
            std::string& request_type_name,
            std::string& request_serialized_qos,
            std::string& reply_type_name,
            std::string& reply_serialized_qos)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->service_mutex_);

            current_test_instance_->received_services_request_++;
            std::cout << "Service type request callback received: " << service_name << ", Total services request: " <<
                current_test_instance_->received_services_request_ << std::endl;

            std::string service_file = TEST_FILE_DIRECTORY + std::string(TEST_SERVICE_FILE);
            if (eprosima::ddsenabler::participants::RpcUtils::load_service_from_file(
                        service_name,
                        request_type_name,
                        reply_type_name,
                        request_serialized_qos,
                        reply_serialized_qos,
                        service_file))
            {
                current_test_instance_->cv_.notify_all();
                return true;
            }
            std::cout << "ERROR Tester: fail to load service from file: " << service_file << std::endl;
        }
        return false;
    }

    // eprosima::ddsenabler::participants::ActionNotification action_notification_callback;
    static void test_action_notification_callback(
            const char* action_name,
            const char* goal_request_action_type,
            const char* goal_reply_action_type,
            const char* cancel_request_action_type,
            const char* cancel_reply_action_type,
            const char* result_request_action_type,
            const char* result_reply_action_type,
            const char* feedback_action_type,
            const char* status_action_type,
            const char* goal_request_action_serialized_qos,
            const char* goal_reply_action_serialized_qos,
            const char* cancel_request_action_serialized_qos,
            const char* cancel_reply_action_serialized_qos,
            const char* result_request_action_serialized_qos,
            const char* result_reply_action_serialized_qos,
            const char* feedback_action_serialized_qos,
            const char* status_action_serialized_qos)
    {
        if (current_test_instance_ && TEST_ACTION_NAME == std::string(action_name))
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);

            current_test_instance_->received_actions_++;
            std::cout << "Action callback received: " << action_name << ", Total actions request: " <<
                current_test_instance_->received_actions_ << std::endl;

            std::string action_file = TEST_FILE_DIRECTORY + std::string(TEST_ACTION_FILE);
            eprosima::ddsenabler::participants::RpcUtils::save_action_to_file(action_name,
                    goal_request_action_type, goal_reply_action_type, cancel_request_action_type, cancel_reply_action_type,
                    result_request_action_type, result_reply_action_type, feedback_action_type, status_action_type,
                    goal_request_action_serialized_qos, goal_reply_action_serialized_qos,
                    cancel_request_action_serialized_qos, cancel_reply_action_serialized_qos,
                    result_request_action_serialized_qos, result_reply_action_serialized_qos,
                    feedback_action_serialized_qos, status_action_serialized_qos, action_file);
            std::cout << "Action file saved: " << action_file << std::endl;

            current_test_instance_->cv_.notify_all();
        }
    }

    // eprosima::ddsenabler::participants::ActionResultNotification result_notification_callback;
    static void test_action_result_notification_callback(
            const char* action_name,
            const char* json,
            const UUID& goal_id,
            int64_t publish_time)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);
            current_test_instance_->actions_goal_uuid_ = goal_id;
            current_test_instance_->received_actions_result_ = true;
            std::cout << "Action result callback received: " << action_name << std::endl;
            current_test_instance_->received_actions_feedback_ = 0;
        }

        current_test_instance_->cv_.notify_all();
    }

    // eprosima::ddsenabler::participants::ActionFeedbackNotification feedback_notification_callback;
    static void test_action_feedback_notification_callback(
            const char* action_name,
            const char* json,
            const UUID& goal_id,
            int64_t publish_time)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);
            current_test_instance_->actions_feedback_uuid_ = goal_id;
            current_test_instance_->received_actions_feedback_++;
            std::cout << "Action feedback callback received with UUID: " << goal_id << ", Total actions feedback: " <<
                current_test_instance_->received_actions_feedback_ << std::endl;
        }

        current_test_instance_->cv_.notify_all();
    }

    // eprosima::ddsenabler::participants::ActionStatusNotification status_notification_callback;
    static void test_action_status_notification_callback(
            const char* action_name,
            const UUID& goal_id,
            eprosima::ddsenabler::participants::STATUS_CODE statusCode,
            const char* statusMessage,
            int64_t publish_time)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);

            current_test_instance_->last_status_ = static_cast<eprosima::ddsenabler::participants::STATUS_CODE>(statusCode);
            current_test_instance_->received_actions_status_updates_++;
            std::cout << "Action status callback received: " << statusCode << ": " << statusMessage << ", Total status updates: " <<
                current_test_instance_->received_actions_status_updates_ << std::endl;

            current_test_instance_->cv_.notify_all();
        }
    }

    // eprosima::ddsenabler::participants::ActionCancelRequestNotification cancel_request_callback;
    static void test_action_cancel_request_notification_callback(
            const char* action_name,
            const UUID& goal_id,
            int64_t timestamp,
            uint64_t request_id,
            int64_t publish_time)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);

            std::cout << "Action cancel request callback received for action: " << action_name
                      << " with UUID: " << goal_id << std::endl;

            current_test_instance_->received_actions_cancel_request_id_ = request_id;

            current_test_instance_->cv_.notify_all();
        }
    }

    // eprosima::ddsenabler::participants::ActionQuery action_query_callback;
    static bool test_action_query_callback(
            const char* action_name,
            std::string& goal_request_action_type,
            std::string& goal_reply_action_type,
            std::string& cancel_request_action_type,
            std::string& cancel_reply_action_type,
            std::string& result_request_action_type,
            std::string& result_reply_action_type,
            std::string& feedback_action_type,
            std::string& status_action_type,
            std::string& goal_request_action_serialized_qos,
            std::string& goal_reply_action_serialized_qos,
            std::string& cancel_request_action_serialized_qos,
            std::string& cancel_reply_action_serialized_qos,
            std::string& result_request_action_serialized_qos,
            std::string& result_reply_action_serialized_qos,
            std::string& feedback_action_serialized_qos,
            std::string& status_action_serialized_qos)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);


            std::string action_file = TEST_FILE_DIRECTORY + std::string(TEST_ACTION_FILE);
            if (eprosima::ddsenabler::participants::RpcUtils::load_action_from_file(
                        action_name, goal_request_action_type, goal_reply_action_type, cancel_request_action_type, cancel_reply_action_type,
                        result_request_action_type, result_reply_action_type, feedback_action_type, status_action_type,
                        goal_request_action_serialized_qos, goal_reply_action_serialized_qos,
                        cancel_request_action_serialized_qos, cancel_reply_action_serialized_qos,
                        result_request_action_serialized_qos, result_reply_action_serialized_qos,
                        feedback_action_serialized_qos, status_action_serialized_qos, action_file))
            {
                return true;
            }
            std::cout << "ERROR Tester: fail to load action from file: " << action_file << std::endl;
        }
        return false;
    }

    static bool test_action_goal_request_notification_callback(
            const char* action_name,
            const char* json,
            const UUID& goal_id,
            int64_t publish_time)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);

            uint64_t fibonacci_number = 5; // TODO removed json parsing for simplicity

            if (current_test_instance_->action_goal_requests_total_ < 3)
            {
                current_test_instance_->action_goal_requests_.push_back(std::make_pair(goal_id, fibonacci_number));
                std::cout << "Action goal request callback received with UUID: " << goal_id << std::endl;
                current_test_instance_->action_goal_requests_total_++;

                // Notify that the action goal request has been received
                current_test_instance_->cv_.notify_all();

                return true;
            }
            else
            {
                std::cout << "Action goal request callback received with UUID: " << goal_id
                          << " but ignoring it as the limit has been reached." << std::endl;
                return false;
            }
        }
        return false;
    }

    int get_received_types()
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->dds_received_mutex_);

            return current_test_instance_->received_types_;
        }
        else
        {
            return 0;
        }
    }

    int get_received_topics()
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->dds_received_mutex_);

            return current_test_instance_->received_topics_;
        }
        else
        {
            return 0;
        }
    }

    int get_received_data()
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->dds_received_mutex_);

            return current_test_instance_->received_data_;
        }
        else
        {
            return 0;
        }
    }

    // Pointer to the current test instance (for use in the static callback)
    static Tester* current_test_instance_;

    // Test-specific received counters
    int received_types_ = 0;
    int received_topics_ = 0;
    int received_data_ = 0;
    int received_reply_ = 0;
    int received_request_id_ = 0;
    int received_services_ = 0;
    int received_services_request_ = 0;
    int received_actions_ = 0;
    bool received_actions_result_ = false;
    UUID actions_feedback_uuid_;
    UUID actions_goal_uuid_;
    std::vector<std::pair<UUID, uint64_t>> action_goal_requests_;
    int action_goal_requests_total_ = 0; // Limit for action goal requests
    int received_actions_feedback_ = 0;
    bool received_actions_cancel_request_id_ = 0;
    eprosima::ddsenabler::participants::STATUS_CODE last_status_;
    int received_actions_status_updates_ = 0;
    // Condition variable for synchronization
    std::condition_variable cv_;

    // Mutex for synchronizing access to all received data;
    std::mutex dds_received_mutex_;
    std::mutex service_mutex_;
    std::mutex action_mutex_;
};

} // namespace tester
