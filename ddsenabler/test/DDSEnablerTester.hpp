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

#include <mutex>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include "ddsenabler/dds_enabler_runner.hpp"
#include "ddsenabler_participants/RpcUtils.hpp"

#include <nlohmann/json.hpp>

using namespace eprosima::ddspipe;
using namespace eprosima::ddsenabler;
using namespace eprosima::ddsenabler::participants;
using namespace eprosima::fastdds::dds;

#define TEST_SERVICE_NAME "add_two_ints"
#define TEST_ACTION_NAME "fibonacci/_action/"

#define TEST_FILE_DIRECTORY "/home/eugenio/Documents/enabler_suite/fast_suite/src/FIWARE-DDS-Enabler/ddsenabler/test/test_files/"
#define TEST_SERVICE_FILE "test_service.json"
#define TEST_ACTION_FILE "test_action.json"

namespace ddsenablertester {

struct KnownType
{
    DynamicType::_ref_type dyn_type_;
    TypeSupport type_sup_;
    DataWriter* writer_ = nullptr;
};

const unsigned int DOMAIN_ = 0;
static int num_samples_ =  1;
static int write_delay_ =  10; // Values below 10 might cause flaky results

class DDSEnablerTester : public ::testing::Test
{
public:

    // SetUp method to initialize variables before each test
    void SetUp() override
    {
        std::cout << "Setting up test..." << std::endl;
        received_types_ = 0;
        received_data_ = 0;
        received_reply_ = 0;
        received_request_id_ = 0;
        received_services_ = 0;
        received_services_request_ = 0;
        received_actions_ = 0;
        received_actions_result_ = false;
        actions_feedback_uuid_ = UUID();
        actions_goal_uuid_ = UUID();
        action_goal_requests_.clear();
        action_goal_requests_total_ = 0;
        received_actions_feedback_ = 0;
        received_actions_cancel_request_id_ = 0;
        last_status_ = eprosima::ddsenabler::participants::STATUS_CODE::STATUS_UNKNOWN;
        received_actions_status_updates_ = 0;
        current_test_instance_ = this;  // Set the current instance for callbacks
    }

    // Reset after each test
    void TearDown() override
    {
        std::cout << "Tearing down test..." << std::endl;
        std::cout << "Received types before reset: " << received_types_ << std::endl;
        std::cout << "Received data before reset: " << received_data_ << std::endl;
        std::cout << "Received reply before reset: " << received_reply_ << std::endl;
        std::cout << "Received request before reset: " << received_request_id_ << std::endl;
        std::cout << "Received services before reset: " << received_services_ << std::endl;
        std::cout << "Received services request before reset: " << received_services_request_ << std::endl;
        std::cout << "Received actions before reset: " << received_actions_ << std::endl;
        std::cout << "Received actions result before reset: " << received_actions_result_ << std::endl;
        std::cout << "Received actions feedback before reset: " << received_actions_feedback_ << std::endl;
        std::cout << "Received actions cancel request before reset: " << received_actions_cancel_request_id_ << std::endl;
        std::cout << "Received actions status updates before reset: " << received_actions_status_updates_ << std::endl;
        received_types_ = 0;
        received_data_ = 0;
        received_reply_ = 0;
        received_request_id_ = 0;
        received_services_ = 0;
        received_services_request_ = 0;
        received_actions_ = 0;
        received_actions_result_ = false;
        actions_feedback_uuid_ = UUID();
        actions_goal_uuid_ = UUID();
        action_goal_requests_.clear();
        action_goal_requests_total_ = 0;
        received_actions_feedback_ = 0;
        received_actions_cancel_request_id_ = 0;
        last_status_ = eprosima::ddsenabler::participants::STATUS_CODE::STATUS_UNKNOWN;
        received_actions_status_updates_ = 0;
        current_test_instance_ = nullptr;
    }

    // Create the DDSEnabler and bind the static callbacks
    std::unique_ptr<DDSEnabler> create_ddsenabler()
    {
        YAML::Node yml;

        eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);

        // TODO study why it gets stuck in RTPSDomainImpl::createParticipant when setting the domain
        // configuration.simple_configuration->domain = eprosima::ddspipe::core::types::DomainId(49u);
        // configuration.simple_configuration->transport = eprosima::ddspipe::core::types::TransportDescriptors::udp_only;


        eprosima::ddsenabler::participants::ddsCallbacks dds_callbacks;
        dds_callbacks.data_callback = test_data_callback;
        dds_callbacks.type_callback = test_type_callback;
        dds_callbacks.topic_callback = test_topic_notification_callback;
        dds_callbacks.type_req_callback = test_type_request_callback;
        dds_callbacks.topic_req_callback = test_topic_request_callback;
        dds_callbacks.log_callback = test_log_callback;

        eprosima::ddsenabler::participants::serviceCallbacks service_callbacks;
        service_callbacks.service_callback = test_service_callback;
        service_callbacks.reply_callback = test_reply_callback;
        service_callbacks.request_callback = test_request_callback;
        service_callbacks.type_req_callback = test_service_type_request_callback;

        eprosima::ddsenabler::participants::actionCallbacks action_callbacks;
        action_callbacks.action_callback = test_action_callback;
        action_callbacks.result_callback = test_action_result_callback;
        action_callbacks.feedback_callback = test_action_feedback_callback;
        action_callbacks.status_callback = test_action_status_callback;
        action_callbacks.type_req_callback = test_action_type_request_callback;
        action_callbacks.goal_request_callback = test_action_goal_request_notification_callback;
        action_callbacks.cancel_request_callback = test_action_cancel_request_notification_callback;

        // Create DDS Enabler
        std::unique_ptr<DDSEnabler> enabler;
        bool result = create_dds_enabler(configuration, dds_callbacks, service_callbacks, action_callbacks, enabler);

        return enabler;
    }

    // Create the DDSEnabler and bind the static callbacks
    std::unique_ptr<DDSEnabler> create_ddsenabler_w_history()
    {
        const char* yml_str =
                R"(
                topics:
                  name: "*"
                  qos:
                    durability: TRANSIENT_LOCAL
                    history-depth: 10
            )";
        eprosima::Yaml yml = YAML::Load(yml_str);
        eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);

        eprosima::utils::Formatter error_msg;
        if (!configuration.is_valid(error_msg))
        {
            return nullptr;
        }

        auto close_handler = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

        auto enabler = std::make_unique<DDSEnabler>(configuration, close_handler);

        // Bind the static callbacks (no captures allowed)
        enabler->set_data_callback(test_data_callback);
        enabler->set_type_callback(test_type_callback);

        return enabler;
    }

    bool create_publisher(
            KnownType& a_type)
    {
        DomainParticipant* participant = DomainParticipantFactory::get_instance()
                        ->create_participant(0, PARTICIPANT_QOS_DEFAULT);
        if (participant == nullptr)
        {
            std::cout << "ERROR DDSEnablerTester: create_participant" << std::endl;
            return false;
        }

        if (RETCODE_OK != a_type.type_sup_.register_type(participant))
        {
            std::cout << "ERROR DDSEnablerTester: fail to register type: " <<
                a_type.type_sup_.get_type_name() << std::endl;
            return false;
        }

        Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
        if (publisher == nullptr)
        {
            std::cout << "ERROR DDSEnablerTester: create_publisher: " <<
                a_type.type_sup_.get_type_name() << std::endl;
            return false;
        }

        std::ostringstream topic_name;
        topic_name << a_type.type_sup_.get_type_name() << "TopicName";
        Topic* topic = participant->create_topic(topic_name.str(), a_type.type_sup_.get_type_name(), TOPIC_QOS_DEFAULT);
        if (topic == nullptr)
        {
            std::cout << "ERROR DDSEnablerTester: create_topic: " <<
                a_type.type_sup_.get_type_name() << std::endl;
            return false;
        }

        DataWriterQos wqos = publisher->get_default_datawriter_qos();
        a_type.writer_ = publisher->create_datawriter(topic, wqos);
        if (a_type.writer_ == nullptr)
        {
            std::cout << "ERROR DDSEnablerTester: create_datawriter: " <<
                a_type.type_sup_.get_type_name() << std::endl;
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return true;
    }

    bool create_publisher_w_history(
            KnownType& a_type,
            int history_depth)
    {
        DomainParticipant* participant = DomainParticipantFactory::get_instance()
                        ->create_participant(0, PARTICIPANT_QOS_DEFAULT);
        if (participant == nullptr)
        {
            std::cout << "ERROR DDSEnablerTester: create_participant" << std::endl;
            return false;
        }

        if (RETCODE_OK != a_type.type_sup_.register_type(participant))
        {
            std::cout << "ERROR DDSEnablerTester: fail to register type: " <<
                a_type.type_sup_.get_type_name() << std::endl;
            return false;
        }

        Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
        if (publisher == nullptr)
        {
            std::cout << "ERROR DDSEnablerTester: create_publisher: " <<
                a_type.type_sup_.get_type_name() << std::endl;
            return false;
        }

        std::ostringstream topic_name;
        topic_name << a_type.type_sup_.get_type_name() << "TopicName";
        Topic* topic = participant->create_topic(topic_name.str(), a_type.type_sup_.get_type_name(), TOPIC_QOS_DEFAULT);
        if (topic == nullptr)
        {
            std::cout << "ERROR DDSEnablerTester: create_topic: " <<
                a_type.type_sup_.get_type_name() << std::endl;
            return false;
        }

        DataWriterQos wqos = publisher->get_default_datawriter_qos();
        wqos.history().depth = history_depth;
        a_type.writer_ = publisher->create_datawriter(topic, wqos);
        if (a_type.writer_ == nullptr)
        {
            std::cout << "ERROR DDSEnablerTester: create_datawriter: " <<
                a_type.type_sup_.get_type_name() << std::endl;
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return true;
    }

    bool send_samples(
            KnownType& a_type)
    {
        std::cout << "Sending samples for type: " << a_type.type_sup_.get_type_name() << std::endl;
        for (long i = 0; i < ddsenablertester::num_samples_; i++)
        {
            void* sample = a_type.type_sup_.create_data();
            if (RETCODE_OK != a_type.writer_->write(sample))
            {
                std::cout << "ERROR DDSEnablerTester: fail writing sample: " <<
                    a_type.type_sup_.get_type_name() << std::endl;
                return false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(write_delay_));
            a_type.type_sup_.delete_data(sample);
        }

        if (RETCODE_OK != a_type.writer_->wait_for_acknowledgments(Duration_t(0, 1000000000)))
        {
            std::cout << "ERROR DDSEnablerTester: fail waiting for acknowledgments: " <<
                a_type.type_sup_.get_type_name() << std::endl;
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(write_delay_ * 10));

        return true;
    }

    uint64_t wait_for_request(
            const std::string& service_name,
            uint64_t& request_id,
            int timeout = 10)
    {
        std::unique_lock<std::mutex> lock(data_received_mutex_);
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

    bool wait_for_status_update(
            const UUID& action_id,
            eprosima::ddsenabler::participants::STATUS_CODE& status,
            const eprosima::ddsenabler::participants::STATUS_CODE& expected_status,
            int timeout = 10)
    {
        std::unique_lock<std::mutex> lock(data_received_mutex_);
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
        std::unique_lock<std::mutex> lock(data_received_mutex_);
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
        std::unique_lock<std::mutex> lock(data_received_mutex_);
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
        std::unique_lock<std::mutex> lock(data_received_mutex_);
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

    // eprosima::ddsenabler::participants::DdsNotification data_callback;
    static void test_data_callback(
            const char* topicName,
            const char* json,
            int64_t publishTime)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->data_received_mutex_);

            current_test_instance_->received_data_++;
            std::cout << "Data callback received: " << topicName << ", Total data: " <<
                current_test_instance_->received_data_ << std::endl;
        }
    }

    // eprosima::ddsenabler::participants::DdsTypeNotification data_callback;
    static void test_type_callback(
            const char* typeName,
            const char* serializedType,
            const unsigned char* serializedTypeInternal,
            uint32_t serializedTypeInternalSize,
            const char* dataPlaceholder)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->type_received_mutex_);

            current_test_instance_->received_types_++;
            std::cout << "Type callback received: " << typeName << ", Total types: " <<
                current_test_instance_->received_types_ << std::endl;

            // eprosima::ddsenabler::participants::RpcUtils::save_type_to_file(
            //     TEST_FILE_DIRECTORY,
            //     typeName,
            //     serializedTypeInternal,
            //     serializedTypeInternalSize);
        }
    }

    // eprosima::ddsenabler::participants::DdsTopicNotification topic_callback;
    static void test_topic_notification_callback(
            const char* topicName,
            const char* typeName,
            const char* serializedQos)
    {
    }

    // eprosima::ddsenabler::participants::DdsTopicRequest topic_req_callback;
    static bool test_topic_request_callback(
            const char* topicName,
            char*& typeName,
            char*& serializedQos)
    {
        return false;
    }

    // eprosima::ddsenabler::participants::DdsTypeQuery type_req_callback;
    static bool test_type_request_callback(
            const char* typeName,
            unsigned char*& serializedTypeInternal,
            uint32_t& serializedTypeInternalSize)
    {
        return eprosima::ddsenabler::participants::RpcUtils::load_type_from_file(
            TEST_FILE_DIRECTORY,
            typeName,
            serializedTypeInternal,
            serializedTypeInternalSize);
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

    // eprosima::ddsenabler::participants::ServiceNotification service_callback;
    static void test_service_callback(
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

            // std::string service_file = TEST_FILE_DIRECTORY + std::string(TEST_SERVICE_FILE);
            // eprosima::ddsenabler::participants::RpcUtils::save_service_to_file(service_name, request_type_name, reply_type_name,
            //         request_serialized_qos, reply_serialized_qos, service_file);
        }
    }

    // eprosima::ddsenabler::participants::ServiceReplyNotification reply_callback;
    static void test_reply_callback(
            const char* service_name,
            const char* json,
            uint64_t request_id,
            int64_t publishTime)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->data_received_mutex_);

            current_test_instance_->received_reply_ = request_id;
            std::cout << "Reply callback received with id: " << request_id << " for service: " << service_name << std::endl;
        }
    }

    // eprosima::ddsenabler::participants::ServiceRequestNotification request_callback;
    static void test_request_callback(
            const char* service_name,
            const char* json,
            uint64_t request_id,
            int64_t publishTime)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->data_received_mutex_);

            current_test_instance_->received_request_id_ = request_id;
            std::cout << "Request callback received with id: " << request_id << " for service: " << service_name << std::endl;
            current_test_instance_->cv_.notify_all();
        }
    }

    // eprosima::ddsenabler::participants::ServiceTypeQuery type_req_callback;
    static bool test_service_type_request_callback(
            const char* service_name,
            char*& request_type_name,
            char*& request_serialized_qos,
            char*& reply_type_name,
            char*& reply_serialized_qos)
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
                return true;
            }
            std::cout << "ERROR DDSEnablerTester: fail to load service from file: " << service_file << std::endl;
        }
        return false;
    }

    // eprosima::ddsenabler::participants::ActionNotification action_callback;
    static void test_action_callback(
            const char* actionName,
            const char* goalRequestActionType,
            const char* goalReplyActionType,
            const char* cancelRequestActionType,
            const char* cancelReplyActionType,
            const char* resultRequestActionType,
            const char* resultReplyActionType,
            const char* feedbackActionType,
            const char* statusActionType,
            const char* goalRequestActionSerializedQos,
            const char* goalReplyActionSerializedQos,
            const char* cancelRequestActionSerializedQos,
            const char* cancelReplyActionSerializedQos,
            const char* resultRequestActionSerializedQos,
            const char* resultReplyActionSerializedQos,
            const char* feedbackActionSerializedQos,
            const char* statusActionSerializedQos)
    {
        if (current_test_instance_ && TEST_ACTION_NAME == std::string(actionName))
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);

            current_test_instance_->received_actions_++;
            std::cout << "Action callback received: " << actionName << ", Total actions request: " <<
                current_test_instance_->received_actions_ << std::endl;

            std::string action_file = TEST_FILE_DIRECTORY + std::string(TEST_ACTION_FILE);
            eprosima::ddsenabler::participants::RpcUtils::save_action_to_file(actionName,
                    goalRequestActionType, goalReplyActionType, cancelRequestActionType, cancelReplyActionType,
                    resultRequestActionType, resultReplyActionType, feedbackActionType, statusActionType,
                    goalRequestActionSerializedQos, goalReplyActionSerializedQos,
                    cancelRequestActionSerializedQos, cancelReplyActionSerializedQos,
                    resultRequestActionSerializedQos, resultReplyActionSerializedQos,
                    feedbackActionSerializedQos, statusActionSerializedQos, action_file);
            std::cout << "Action file saved: " << action_file << std::endl;
        }
    }

    // eprosima::ddsenabler::participants::ActionResultNotification result_callback;
    static void test_action_result_callback(
            const char* actionName,
            const char* json,
            const UUID& goalId,
            int64_t publishTime)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);
            current_test_instance_->actions_goal_uuid_ = goalId;
            current_test_instance_->received_actions_result_ = true;
            std::cout << "Action result callback received: " << actionName << std::endl;
            current_test_instance_->received_actions_feedback_ = 0;
        }

        current_test_instance_->cv_.notify_all();
    }

    // eprosima::ddsenabler::participants::ActionFeedbackNotification feedback_callback;
    static void test_action_feedback_callback(
            const char* actionName,
            const char* json,
            const UUID& goalId,
            int64_t publishTime)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);
            current_test_instance_->actions_feedback_uuid_ = goalId;
            current_test_instance_->received_actions_feedback_++;
            std::cout << "Action feedback callback received with UUID: " << goalId << ", Total actions feedback: " <<
                current_test_instance_->received_actions_feedback_ << std::endl;
        }

        current_test_instance_->cv_.notify_all();
    }

    // eprosima::ddsenabler::participants::ActionStatusNotification status_callback;
    static void test_action_status_callback(
            const char* actionName,
            const UUID& goalId,
            eprosima::ddsenabler::participants::STATUS_CODE statusCode,
            const char* statusMessage,
            int64_t publishTime)
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
            const char* actionName,
            const UUID& goalId,
            int64_t timestamp,
            uint64_t request_id,
            int64_t publishTime)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);

            std::cout << "Action cancel request callback received for action: " << actionName
                      << " with UUID: " << goalId << std::endl;

            current_test_instance_->received_actions_cancel_request_id_ = request_id;

            current_test_instance_->cv_.notify_all();
        }
    }

    // eprosima::ddsenabler::participants::ActionTypeQuery action_type_req_callback;
    static bool test_action_type_request_callback(
            const char* actionName,
            char*& goalRequestActionType,
            char*& goalReplyActionType,
            char*& cancelRequestActionType,
            char*& cancelReplyActionType,
            char*& resultRequestActionType,
            char*& resultReplyActionType,
            char*& feedbackActionType,
            char*& statusActionType,
            char*& goalRequestActionSerializedQos,
            char*& goalReplyActionSerializedQos,
            char*& cancelRequestActionSerializedQos,
            char*& cancelReplyActionSerializedQos,
            char*& resultRequestActionSerializedQos,
            char*& resultReplyActionSerializedQos,
            char*& feedbackActionSerializedQos,
            char*& statusActionSerializedQos)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);


            std::string action_file = TEST_FILE_DIRECTORY + std::string(TEST_ACTION_FILE);
            if (eprosima::ddsenabler::participants::RpcUtils::load_action_from_file(
                        actionName, goalRequestActionType, goalReplyActionType, cancelRequestActionType, cancelReplyActionType,
                        resultRequestActionType, resultReplyActionType, feedbackActionType, statusActionType,
                        goalRequestActionSerializedQos, goalReplyActionSerializedQos,
                        cancelRequestActionSerializedQos, cancelReplyActionSerializedQos,
                        resultRequestActionSerializedQos, resultReplyActionSerializedQos,
                        feedbackActionSerializedQos, statusActionSerializedQos, action_file))
            {
                return true;
            }
            std::cout << "ERROR DDSEnablerTester: fail to load action from file: " << action_file << std::endl;
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

            nlohmann::json json_data = nlohmann::json::parse(json);

            uint64_t fibonacci_number = 5;
            const auto& data = json_data["rq/fibonacci/_action/send_goalRequest"]["data"];
            if (!data.empty()) {
                const auto& first_entry = data.begin().value();
                if (first_entry.contains("goal") && first_entry["goal"].contains("order")) {
                    fibonacci_number = first_entry["goal"]["order"].get<int>();
                }
            }

            if (current_test_instance_->action_goal_requests_total_ < 3)
            {
                current_test_instance_->action_goal_requests_.push_back(std::make_pair(goal_id, fibonacci_number));
                std::cout << "Action goal request callback received with UUID: " << goal_id << " and order: " << fibonacci_number << std::endl;
                current_test_instance_->action_goal_requests_total_++;

                // Notify that the action goal request has been received
                current_test_instance_->cv_.notify_all();

                return true;
            }
            else
            {
                std::cout << "Action goal request callback received with UUID: " << goal_id << " and order: " << fibonacci_number
                          << ", but ignoring it as the limit has been reached." << std::endl;
                return false;
            }
        }
        return false;
    }

    int get_received_types()
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->type_received_mutex_);

            return current_test_instance_->received_types_;
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
            std::lock_guard<std::mutex> lock(current_test_instance_->data_received_mutex_);

            return current_test_instance_->received_data_;
        }
        else
        {
            return 0;
        }
    }

    int get_received_reply()
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->data_received_mutex_);

            return current_test_instance_->received_reply_;
        }
        else
        {
            return 0;
        }
    }

    int get_received_request_id()
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->data_received_mutex_);

            return current_test_instance_->received_request_id_;
        }
        else
        {
            return 0;
        }
    }

    int get_received_services()
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->service_mutex_);

            return current_test_instance_->received_services_;
        }
        else
        {
            return 0;
        }
    }

    int get_received_services_request()
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->service_mutex_);

            return current_test_instance_->received_services_request_;
        }
        else
        {
            return 0;
        }
    }

    int get_received_actions()
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);

            return current_test_instance_->received_actions_;
        }
        else
        {
            return 0;
        }
    }

    int get_received_actions_feedback(UUID& feedback_id)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);
            feedback_id = current_test_instance_->actions_feedback_uuid_;
            return current_test_instance_->received_actions_feedback_;
        }
        else
        {
            return 0;
        }
    }

    int get_received_actions_status_updates(eprosima::ddsenabler::participants::STATUS_CODE& status)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->action_mutex_);
            status = current_test_instance_->last_status_;
            return current_test_instance_->received_actions_status_updates_;
        }
        else
        {
            return 0;
        }
    }


    // Pointer to the current test instance (for use in the static callback)
    static DDSEnablerTester* current_test_instance_;

    // Test-specific received counters
    int received_types_ = 0;
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

    // Mutex for synchronizing access to received_types_ and received_data_/received_reply_/received_request_id_
    std::mutex type_received_mutex_;
    std::mutex data_received_mutex_;
    std::mutex service_mutex_;
    std::mutex action_mutex_;
};

} // namespace ddsenablertester
