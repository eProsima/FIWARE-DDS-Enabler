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

using namespace eprosima::ddspipe;
using namespace eprosima::ddsenabler;
using namespace eprosima::ddsenabler::participants;
using namespace eprosima::fastdds::dds;

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
        received_types_ = 0;
        received_data_ = 0;
        received_reply_ = 0;
        received_request_id_ = 0;
        current_test_instance_ = nullptr;
    }

    // Create the DDSEnabler and bind the static callbacks
    std::unique_ptr<DDSEnabler> create_ddsenabler()
    {
        YAML::Node yml;

        eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);

        // Create DDS Enabler
        std::unique_ptr<DDSEnabler> enabler;
        bool result = create_dds_enabler(configuration, test_data_callback, test_reply_callback, test_request_callback, test_type_callback, test_topic_notification_callback, test_type_request_callback, test_topic_request_callback, test_log_callback, enabler);

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
            int timeout = 10)
    {
        // Asume there is a mtx and a condition variable in test_request_callback
        std::unique_lock<std::mutex> lock(data_received_mutex_);
        if (cv_.wait_for(lock, std::chrono::seconds(timeout), [this, service_name]()
                {
                    return received_request_id_ > 0;
                }))
        {
            auto request_id = received_request_id_;
            received_request_id_ = 0; // Reset the request ID after receiving it
            return request_id;
        }
        else
        {
            std::cout << "Timeout waiting for request callback" << std::endl;
            return 0;
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

    // eprosima::ddsenabler::participants::ServiceReplyNotification reply_callback;
    static void test_reply_callback(
            const char* serviceName,
            const char* json,
            uint64_t requestId,
            int64_t publishTime)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->data_received_mutex_);

            current_test_instance_->received_reply_ = requestId;
            std::cout << "Reply callback received with id: " << requestId << " for service: " << serviceName << std::endl;
        }
    }

    // eprosima::ddsenabler::participants::ServiceRequestNotification request_callback;
    static void test_request_callback(
            const char* serviceName,
            const char* json,
            uint64_t requestId,
            int64_t publishTime)
    {
        if (current_test_instance_)
        {
            std::lock_guard<std::mutex> lock(current_test_instance_->data_received_mutex_);

            current_test_instance_->received_request_id_ = requestId;
            std::cout << "Request callback received with id: " << requestId << " for service: " << serviceName << std::endl;
            current_test_instance_->cv_.notify_all();
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
    static void test_topic_request_callback(
            const char* topicName,
            char*& typeName,
            char*& serializedQos)
    {
    }

    // eprosima::ddsenabler::participants::DdsTypeRequest type_req_callback;
    static void test_type_request_callback(
            const char* typeName,
            unsigned char*& serializedTypeInternal,
            uint32_t& serializedTypeInternalSize)
    {
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
    

    // Pointer to the current test instance (for use in the static callback)
    static DDSEnablerTester* current_test_instance_;

    // Test-specific received counters
    int received_types_ = 0;
    int received_data_ = 0;
    int received_reply_ = 0;
    int received_request_id_ = 0;
    // Condition variable for synchronization
    std::condition_variable cv_;

    // Mutex for synchronizing access to received_types_ and received_data_/received_reply_/received_request_id_
    std::mutex type_received_mutex_;
    std::mutex data_received_mutex_;
};


} // namespace ddsenablertester
