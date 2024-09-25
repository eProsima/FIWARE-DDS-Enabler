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

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include <ddsenabler_yaml/EnablerConfiguration.hpp>
#include <ddsenabler_yaml/yaml_configuration_tags.hpp>

#include "DDSEnabler.hpp"

#include <cpp_utils/utils.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include "../resources/types/DDSEnablerTestTypes.hpp"
#include "../resources/types/DDSEnablerTestTypesPubSubTypes.hpp"
#include "../resources/types/DDSEnablerTestTypesTypeObjectSupport.hpp"

using namespace eprosima::ddspipe;
using namespace eprosima::ddsenabler;
using namespace eprosima::ddsenabler::participants;
using namespace eprosima::fastdds::dds;

// namespace ddsenablertypedtests {

// struct PubKnownType
// {
//     DynamicType::_ref_type dyn_type_;
//     TypeSupport type_sup_;
//     DataWriter* writer_ = nullptr;
// };

// const unsigned int DOMAIN_ = 0;
// static int num_samples_ =  3;
// static int write_delay_ =  500;
// static int received_types_ = 0;
// static int received_data_ = 0;

// // Callback to handle type notifications
// static void test_type_callback(
//         const char* typeName,
//         const char* topicName,
//         const char* serializedType)
// {
//     ddsenablertypedtests::received_types_++;
// }

// // Callback to handle data notifications
// static void test_data_callback(
//         const char* typeName,
//         const char* topicName,
//         const char* json,
//         double publishTime)
// {
//     ddsenablertypedtests::received_data_++;
// }

// std::unique_ptr<DDSEnabler> create_ddsenabler()
// {
//     ddsenablertypedtests::received_types_ = 0;
//     ddsenablertypedtests::received_data_ = 0;

//     YAML::Node yml;

//     eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);

//     auto close_handler = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

//     auto enabler = std::make_unique<DDSEnabler>(configuration, close_handler);

//     enabler.get()->set_data_callback(ddsenablertypedtests::test_data_callback);
//     enabler.get()->set_type_callback(ddsenablertypedtests::test_type_callback);

//     return enabler;
// }

// bool send_samples(
//         ddsenablertypedtests::PubKnownType& a_type)
// {
//     for (long i = 0; i < ddsenablertypedtests::num_samples_; i++)
//     {
//         void* sample = a_type.type_sup_.create_data();
//         if (RETCODE_OK != a_type.writer_->write(sample))
//         {
//             std::cout << "ERROR ddsEnablerTest: fail writing sample " << a_type.type_sup_.get_type_name() << std::endl;
//             return false;
//         }
//         std::this_thread::sleep_for(std::chrono::milliseconds(ddsenablertypedtests::write_delay_));
//         a_type.type_sup_.delete_data(sample);
//     }
//     std::this_thread::sleep_for(std::chrono::milliseconds(ddsenablertypedtests::write_delay_));
//     return true;
// }

// bool create_publisher(
//         ddsenablertypedtests::PubKnownType& a_type)
// {
//     // CREATE THE PARTICIPANT
//     DomainParticipant* participant = DomainParticipantFactory::get_instance()
//                     ->create_participant(0, PARTICIPANT_QOS_DEFAULT);
//     if (participant == nullptr)
//     {
//         std::cout << "ERROR ddsEnablerTest: create_participant" << std::endl;
//         return false;
//     }

//     // REGISTER TYPE
//     a_type.type_sup_.register_type(participant);

//     // CREATE THE PUBLISHER
//     Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
//     if (publisher == nullptr)
//     {
//         std::cout << "ERROR ddsEnablerTest: create_publisher: " << a_type.type_sup_.get_type_name() << std::endl;
//         return false;
//     }

//     // CREATE THE TOPIC
//     std::ostringstream topic_name;
//     topic_name << a_type.type_sup_.get_type_name() << "TopicName";
//     Topic* topic = participant->create_topic(topic_name.str(), a_type.type_sup_.get_type_name(), TOPIC_QOS_DEFAULT);
//     if (topic == nullptr)
//     {
//         std::cout << "ERROR ddsEnablerTest: create_topic: " << a_type.type_sup_.get_type_name() << std::endl;
//         return false;
//     }

//     // CREATE THE DATAWRITER
//     DataWriterQos wqos = publisher->get_default_datawriter_qos();
//     wqos.data_sharing().off();
//     // wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
//     // wqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
//     a_type.writer_ = publisher->create_datawriter(topic, wqos);
//     if (a_type.writer_ == nullptr)
//     {
//         std::cout << "ERROR ddsEnablerTest: create_datawriter: " << a_type.type_sup_.get_type_name() << std::endl;
//         return false;
//     }
//     return true;
// }

// } // ddsenablertypedtests


// #define DEFINE_DDSENABLER_TEST(TestName, Type) \
//         TEST(DdsEnablerTypedTest, TestName) \
//         { \
//             auto enabler = ddsenablertypedtests::create_ddsenabler(); \
//             ASSERT_TRUE(enabler != nullptr); \
// \
//             ddsenablertypedtests::PubKnownType a_type; \
//             a_type.type_sup_.reset(new Type()); \
// \
//             ASSERT_TRUE(ddsenablertypedtests::create_publisher(a_type)); \
// \
//             ASSERT_EQ(ddsenablertypedtests::received_types_, 0); \
//             ASSERT_EQ(ddsenablertypedtests::received_data_, 0); \
// \
//             /* Send data */ \
//             ASSERT_TRUE(ddsenablertypedtests::send_samples(a_type)); \
// \
//             ASSERT_EQ(ddsenablertypedtests::received_types_, 1); \
//             ASSERT_EQ(ddsenablertypedtests::received_data_, ddsenablertypedtests::num_samples_); \
//         }


// TEST(DdsEnablerTypedTest, ddsenabler_send_samples_typeT)
// {
//     auto enabler = ddsenablertypedtests::create_ddsenabler();
//     ASSERT_TRUE(enabler != nullptr);

//     ddsenablertypedtests::PubKnownType a_type;
//     a_type.type_sup_.reset(new DDSEnablerTestType1PubSubType());

//     ASSERT_TRUE(ddsenablertypedtests::create_publisher(a_type));

//     ASSERT_EQ(ddsenablertypedtests::received_types_, 0);
//     ASSERT_EQ(ddsenablertypedtests::received_data_, 0);

//     // Send data
//     ASSERT_TRUE(ddsenablertypedtests::send_samples(a_type));

//     ASSERT_EQ(ddsenablertypedtests::received_types_, 1);
//     ASSERT_EQ(ddsenablertypedtests::received_data_, ddsenablertypedtests::num_samples_);
// }


// // Define tests for each DDSEnablerTestTypePubSubType
// DEFINE_DDSENABLER_TEST(ddsenabler_send_samples_type1, DDSEnablerTestType1PubSubType)
// DEFINE_DDSENABLER_TEST(ddsenabler_send_samples_type2, DDSEnablerTestType2PubSubType)
// DEFINE_DDSENABLER_TEST(ddsenabler_send_samples_type3, DDSEnablerTestType3PubSubType)


// int main(
//         int argc,
//         char** argv)
// {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }

namespace ddsenablertypedtests {

struct PubKnownType
{
    DynamicType::_ref_type dyn_type_;
    TypeSupport type_sup_;
    DataWriter* writer_ = nullptr;
};

const unsigned int DOMAIN_ = 0;
static int num_samples_ =  1;
static int write_delay_ =  100;

class DDSEnablerTypedTest : public ::testing::Test
{
protected:

    // Test-specific received counters
    int received_types_ = 0;
    int received_data_ = 0;

    // Pointer to the current test instance (for use in the static callback)
    static DDSEnablerTypedTest* current_test_instance_;

    // Static type callback
    static void test_type_callback(
            const char* typeName,
            const char* topicName,
            const char* serializedType)
    {
        if (current_test_instance_)
        {
            current_test_instance_->received_types_++;
            std::cout << "Type received: " << typeName << ", Total types: " <<
                current_test_instance_->received_types_ << std::endl;
        }
    }

    // Static data callback
    static void test_data_callback(
            const char* typeName,
            const char* topicName,
            const char* json,
            double publishTime)
    {
        if (current_test_instance_)
        {
            current_test_instance_->received_data_++;
            std::cout << "Data received: " << typeName << ", Total data: " <<
                current_test_instance_->received_data_ << std::endl;
        }
    }

    // Create the DDSEnabler and bind the static callbacks
    std::unique_ptr<DDSEnabler> create_ddsenabler()
    {
        YAML::Node yml;

        eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);

        auto close_handler = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

        auto enabler = std::make_unique<DDSEnabler>(configuration, close_handler);

        // Bind the static callbacks (no captures allowed)
        enabler->set_data_callback(test_data_callback);
        enabler->set_type_callback(test_type_callback);

        return enabler;
    }

    // SetUp method to initialize variables before each test
    void SetUp() override
    {
        std::cout << "Setting up test..." << std::endl;
        received_types_ = 0;
        received_data_ = 0;
        current_test_instance_ = this;  // Set the current instance for callbacks
    }

    // Reset the static pointer after each test
    void TearDown() override
    {
        std::cout << "Tearing down test..." << std::endl;
        std::cout << "Received types before reset: " << received_types_ << std::endl;
        std::cout << "Received data before reset: " << received_data_ << std::endl;
        received_types_ = 0;
        received_data_ = 0;
        current_test_instance_ = nullptr;
    }

    // Test helper functions
    bool send_samples(
            ddsenablertypedtests::PubKnownType& a_type)
    {
        std::cout << "Sending samples for type: " << a_type.type_sup_.get_type_name() << std::endl;
        for (long i = 0; i < ddsenablertypedtests::num_samples_; i++)
        {
            void* sample = a_type.type_sup_.create_data();
            if (RETCODE_OK != a_type.writer_->write(sample))
            {
                std::cout << "ERROR DDSEnablerTypedTest: fail writing sample " << a_type.type_sup_.get_type_name() <<
                    std::endl;
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(ddsenablertypedtests::write_delay_));
            a_type.type_sup_.delete_data(sample);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(ddsenablertypedtests::write_delay_));
        return true;
    }

    bool create_publisher(
            ddsenablertypedtests::PubKnownType& a_type)
    {
        DomainParticipant* participant = DomainParticipantFactory::get_instance()
                        ->create_participant(0, PARTICIPANT_QOS_DEFAULT);
        if (participant == nullptr)
        {
            std::cout << "ERROR DDSEnablerTypedTest: create_participant" << std::endl;
            return false;
        }

        a_type.type_sup_.register_type(participant);

        Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
        if (publisher == nullptr)
        {
            std::cout << "ERROR DDSEnablerTypedTest: create_publisher: " << a_type.type_sup_.get_type_name() <<
                std::endl;
            return false;
        }

        std::ostringstream topic_name;
        topic_name << a_type.type_sup_.get_type_name() << "TopicName";
        Topic* topic = participant->create_topic(topic_name.str(), a_type.type_sup_.get_type_name(), TOPIC_QOS_DEFAULT);
        if (topic == nullptr)
        {
            std::cout << "ERROR DDSEnablerTypedTest: create_topic: " << a_type.type_sup_.get_type_name() << std::endl;
            return false;
        }

        DataWriterQos wqos = publisher->get_default_datawriter_qos();
        wqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

        a_type.writer_ = publisher->create_datawriter(topic, wqos);
        if (a_type.writer_ == nullptr)
        {
            std::cout << "ERROR DDSEnablerTypedTest: create_datawriter: " << a_type.type_sup_.get_type_name() <<
                std::endl;
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return true;
    }

};

// Define static pointer
DDSEnablerTypedTest* DDSEnablerTypedTest::current_test_instance_ = nullptr;

#define DEFINE_DDSENABLER_TEST(TestName, Type) \
        TEST_F(DDSEnablerTypedTest, TestName) \
        { \
            auto enabler = create_ddsenabler(); \
            ASSERT_TRUE(enabler != nullptr); \
\
            ddsenablertypedtests::PubKnownType a_type; \
            a_type.type_sup_.reset(new Type()); \
\
            ASSERT_TRUE(create_publisher(a_type)); \
\
            ASSERT_EQ(received_types_, 0); \
            ASSERT_EQ(received_data_, 0); \
\
            /* Send data */ \
            ASSERT_TRUE(send_samples(a_type)); \
\
            ASSERT_EQ(received_types_, 1); \
            ASSERT_EQ(received_data_, ddsenablertypedtests::num_samples_); \
        }


// Define tests using the macro
DEFINE_DDSENABLER_TEST(ddsenabler_send_samples_type1, DDSEnablerTestType1PubSubType)
DEFINE_DDSENABLER_TEST(ddsenabler_send_samples_type2, DDSEnablerTestType2PubSubType)
DEFINE_DDSENABLER_TEST(ddsenabler_send_samples_type3, DDSEnablerTestType3PubSubType)
// Add more types as needed

} // namespace ddsenablertypedtests

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
