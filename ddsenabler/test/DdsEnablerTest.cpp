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

#include "resources/types/DDSEnablerTestTypes.hpp"
#include "resources/types/DDSEnablerTestTypesPubSubTypes.hpp"
#include "resources/types/DDSEnablerTestTypesTypeObjectSupport.hpp"

using namespace eprosima::ddspipe;
using namespace eprosima::ddsenabler;
using namespace eprosima::ddsenabler::participants;
using namespace eprosima::fastdds::dds;

namespace ddsEnablerTest {

const unsigned int DOMAIN = 0;

const std::string dds_topic1_name = "DDSEnablerTestTopic1Name";
const std::string dds_topic2_name = "DDSEnablerTestTopic2Name";
const std::string dds_topic3_name = "DDSEnablerTestTopic3Name";
const std::string dds_topic4_name = "DDSEnablerTestTopic4Name";
const std::string dds_type1_name = "DDSEnablerTestType1";
const std::string dds_type2_name = "DDSEnablerTestType2";
const std::string dds_type3_name = "DDSEnablerTestType3";
const std::string dds_type4_name = "DDSEnablerTestType4";

eprosima::fastdds::dds::DomainParticipant* participant_;

eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicType>::ref_type dynamic_type_;

eprosima::fastdds::dds::Publisher* publisher_;
eprosima::fastdds::dds::Topic* topic_;
eprosima::fastdds::dds::DataWriter* writer_;

static int received_types_ = 0;
static int received_data_ = 0;

// Callback to handle type notifications
static void test_type_callback(
        const char* typeName,
        const char* topicName,
        const char* serializedType)
{
    ddsEnablerTest::received_types_++;
}

// Callback to handle data notifications
static void test_data_callback(
        const char* typeName,
        const char* topicName,
        const char* json,
        double publishTime)
{
    ddsEnablerTest::received_data_++;
}

} // ddsEnablerTest

std::unique_ptr<DDSEnabler> create_ddsenabler()
{
    ddsEnablerTest::received_types_ = 0;
    ddsEnablerTest::received_data_ = 0;

    YAML::Node yml;

    eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);

    auto close_handler = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

    auto enabler = std::make_unique<DDSEnabler>(configuration, close_handler);

    return std::make_unique<DDSEnabler>(configuration, close_handler);
}

void create_publisherAA(
        const std::string& participant_name,
        const std::string& type_name,
        const std::string& topic_name,
        eprosima::fastdds::dds::TypeSupport type_support)
{
    // Configure the DomainParticipant
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.name(participant_name);

    // Create the Participant
    ddsEnablerTest::participant_ = DomainParticipantFactory::get_instance()->create_participant(ddsEnablerTest::DOMAIN,
                    pqos);

    // Register the type
    type_support->register_type_object_representation();

    eprosima::fastdds::dds::xtypes::TypeObjectPair dyn_type_objects;
    if (eprosima::fastdds::dds::RETCODE_OK !=
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                type_name, dyn_type_objects))
    {
        return;
    }

    ddsEnablerTest::dynamic_type_ =
            eprosima::fastdds::dds::DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
        dyn_type_objects.complete_type_object)->build();

    // Register the type in the Participant
    ddsEnablerTest::participant_->register_type(type_support);

    // Create the Publisher
    ddsEnablerTest::publisher_ = ddsEnablerTest::participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    // Create the DDS Topic
    ddsEnablerTest::topic_ = ddsEnablerTest::participant_->create_topic(topic_name, type_name, TOPIC_QOS_DEFAULT);

    // Create the DDS DataWriter
    ddsEnablerTest::writer_ = ddsEnablerTest::publisher_->create_datawriter(ddsEnablerTest::topic_,
                    DATAWRITER_QOS_DEFAULT, nullptr);
}

eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type send_sampleAAA()
{
    // Create and initialize new dynamic data

    if (ddsEnablerTest::dynamic_type_ == nullptr)
    {
        return nullptr;
    }

    auto dynamic_data =
            eprosima::fastdds::dds::DynamicDataFactory::get_instance()->create_data(ddsEnablerTest::dynamic_type_);

    ddsEnablerTest::writer_->write(dynamic_data.get());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    return dynamic_data;
}

void create_publisher(
        const std::string topic_name,
        const std::string type_name)
{
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.name("TypeIntrospectionExample_Participant_Publisher");

    // Create the Participant
    eprosima::fastdds::dds::DomainParticipant* participant_ =
            DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    // Register the type
    eprosima::fastdds::dds::TypeSupport type(new DDSEnablerTestType2PubSubType());
    type->register_type_object_representation();

    eprosima::fastdds::dds::xtypes::TypeObjectPair dyn_type_objects;
    if (eprosima::fastdds::dds::RETCODE_OK !=
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                type_name,
                dyn_type_objects))
    {
        return;
    }

    ddsEnablerTest::dynamic_type_ =
            eprosima::fastdds::dds::DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
        dyn_type_objects.complete_type_object)->build();

    // Register the type in the Participant
    participant_->register_type(type);

    // Create the Publisher
    eprosima::fastdds::dds::Publisher* publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    // Create the DDS Topic
    eprosima::fastdds::dds::Topic* topic_ = participant_->create_topic(topic_name, type_name,
                    TOPIC_QOS_DEFAULT);

    // Create the DDS DataWriter
    ddsEnablerTest::writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, nullptr);
}

bool send_sample()
{
    if (ddsEnablerTest::dynamic_type_ == nullptr)
    {
        return false;
    }

    // Create data
    auto dynamic_data_ =
            eprosima::fastdds::dds::DynamicDataFactory::get_instance()->create_data(ddsEnablerTest::dynamic_type_);
    if (!dynamic_data_)
    {
        return false;
    }

    std::string test_value;
    dynamic_data_->get_string_value(test_value, dynamic_data_->get_member_id_by_name("value"));

    // Set value
    if (eprosima::fastdds::dds::RETCODE_OK !=
            dynamic_data_->set_string_value(dynamic_data_->get_member_id_by_name("value"), "AAAAA"))
    {
        return false;
    }
    dynamic_data_->get_string_value(test_value, dynamic_data_->get_member_id_by_name("value"));




    // Send message
    if (eprosima::fastdds::dds::RETCODE_OK !=
            ddsEnablerTest::writer_->write(dynamic_data_.get()))
    {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    return true;
}

// TEST(DdsEnablerTests, ddsenabler_creation)
// {
//     ASSERT_NO_THROW(auto enabler = create_ddsenabler());
// }

// TEST(DdsEnablerTests, ddsenabler_reload_configuration)
// {
//     auto enabler = create_ddsenabler();
//     ASSERT_TRUE(enabler != nullptr);

//     YAML::Node yml;
//     eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);
//     eprosima::utils::Formatter error_msg;
//     ASSERT_TRUE(configuration.is_valid(error_msg));

//     ASSERT_NO_THROW(enabler.get()->reload_configuration(configuration));
// }

// TEST(DdsEnablerTests, ddsenabler_send_samples_type1)
// {
//     auto enabler = create_ddsenabler();
//     ASSERT_TRUE(enabler != nullptr);

//     enabler.get()->set_data_callback(ddsEnablerTest::test_data_callback);
//     enabler.get()->set_type_callback(ddsEnablerTest::test_type_callback);

//     ASSERT_NO_THROW(create_publisher(
//                 "DDSEnabler_Test_Participant_Publisher_Type1",
//                 ddsEnablerTest::dds_type1_name,
//                 ddsEnablerTest::dds_topic1_name,
//                 eprosima::fastdds::dds::TypeSupport(new DDSEnablerTestType1PubSubType())));

//     ASSERT_EQ(ddsEnablerTest::received_types_, 0);
//     ASSERT_EQ(ddsEnablerTest::received_data_, 0);

//     // Send data
//     int num_samples = 1;
//     eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type send_data;
//     for (long i = 0; i < num_samples; i++)
//     {
//         send_data = send_sample();
//     }

//     ASSERT_EQ(ddsEnablerTest::received_types_, 1);
//     ASSERT_EQ(ddsEnablerTest::received_data_, num_samples);
// }

TEST(DdsEnablerTests, ddsenabler_send_samples_type2)
{
    auto enabler = create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    enabler.get()->set_data_callback(ddsEnablerTest::test_data_callback);
    enabler.get()->set_type_callback(ddsEnablerTest::test_type_callback);

    // ASSERT_NO_THROW(create_publisher(
    //             "DDSEnabler_Test_Participant_Publisher_Type2",
    //             ddsEnablerTest::dds_type2_name,
    //             ddsEnablerTest::dds_topic2_name,
    //             eprosima::fastdds::dds::TypeSupport(new DDSEnablerTestType2PubSubType())));

    ASSERT_NO_THROW(create_publisher(
                ddsEnablerTest::dds_topic2_name,
                ddsEnablerTest::dds_type2_name));

    ASSERT_EQ(ddsEnablerTest::received_types_, 0);
    ASSERT_EQ(ddsEnablerTest::received_data_, 0);

    // Send data
    int num_samples = 1;
    eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type send_data;
    for (long i = 0; i < num_samples; i++)
    {
        ASSERT_TRUE(send_sample());
    }

    ASSERT_EQ(ddsEnablerTest::received_types_, 1);
    ASSERT_EQ(ddsEnablerTest::received_data_, num_samples);
}

// TEST(DdsEnablerTests, ddsenabler_send_samples_type3)
// {
//     auto enabler = create_ddsenabler();
//     ASSERT_TRUE(enabler != nullptr);

//     enabler.get()->set_data_callback(ddsEnablerTest::test_data_callback);
//     enabler.get()->set_type_callback(ddsEnablerTest::test_type_callback);

//     ASSERT_NO_THROW(create_publisher(
//                 "DDSEnabler_Test_Participant_Publisher_Type3",
//                 ddsEnablerTest::dds_type3_name,
//                 ddsEnablerTest::dds_topic3_name,
//                 eprosima::fastdds::dds::TypeSupport(new DDSEnablerTestType3PubSubType())));

//     ASSERT_EQ(ddsEnablerTest::received_types_, 0);
//     ASSERT_EQ(ddsEnablerTest::received_data_, 0);

//     // Send data
//     int num_samples = 1;
//     eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type send_data;
//     for (long i = 0; i < num_samples; i++)
//     {
//         send_data = send_sample();
//     }

//     ASSERT_EQ(ddsEnablerTest::received_types_, 1);
//     ASSERT_EQ(ddsEnablerTest::received_data_, num_samples);
// }

// TEST(DdsEnablerTests, ddsenabler_send_samples_type4)
// {
//     auto enabler = create_ddsenabler();
//     ASSERT_TRUE(enabler != nullptr);

//     enabler.get()->set_data_callback(ddsEnablerTest::test_data_callback);
//     enabler.get()->set_type_callback(ddsEnablerTest::test_type_callback);

//     ASSERT_NO_THROW(create_publisher(
//                 "DDSEnabler_Test_Participant_Publisher_Type4",
//                 ddsEnablerTest::dds_type4_name,
//                 ddsEnablerTest::dds_topic4_name,
//                 eprosima::fastdds::dds::TypeSupport(new DDSEnablerTestType4PubSubType())));

//     ASSERT_EQ(ddsEnablerTest::received_types_, 0);
//     ASSERT_EQ(ddsEnablerTest::received_data_, 0);

//     // Send data
//     int num_samples = 1;
//     eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type send_data;
//     for (long i = 0; i < num_samples; i++)
//     {
//         send_data = send_sample();
//     }

//     ASSERT_EQ(ddsEnablerTest::received_types_, 1);
//     ASSERT_EQ(ddsEnablerTest::received_data_, num_samples);
// }

// TEST(DdsEnablerTests, ddsenabler_send_samples_multiple_types)
// {
//     auto enabler = create_ddsenabler();
//     ASSERT_TRUE(enabler != nullptr);

//     enabler.get()->set_data_callback(ddsEnablerTest::test_data_callback);
//     enabler.get()->set_type_callback(ddsEnablerTest::test_type_callback);

//     ASSERT_NO_THROW(create_publisher(
//                 "DDSEnabler_Test_Participant_Publisher_Type1",
//                 ddsEnablerTest::dds_type1_name,
//                 ddsEnablerTest::dds_topic1_name,
//                 eprosima::fastdds::dds::TypeSupport(new DDSEnablerTestType1PubSubType())));

//     ASSERT_EQ(ddsEnablerTest::received_types_, 0);
//     ASSERT_EQ(ddsEnablerTest::received_data_, 0);

//     // Send data
//     int num_samples = 1;
//     eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type send_data;
//     for (long i = 0; i < num_samples; i++)
//     {
//         send_data = send_sample();
//     }

//     ASSERT_EQ(ddsEnablerTest::received_types_, 1);
//     ASSERT_EQ(ddsEnablerTest::received_data_, num_samples);

//     ASSERT_NO_THROW(create_publisher(
//                 "DDSEnabler_Test_Participant_Publisher_Type2",
//                 ddsEnablerTest::dds_type2_name,
//                 ddsEnablerTest::dds_topic2_name,
//                 eprosima::fastdds::dds::TypeSupport(new DDSEnablerTestType2PubSubType())));
//     for (long i = 0; i < num_samples; i++)
//     {
//         send_data = send_sample();
//     }
//     ASSERT_EQ(ddsEnablerTest::received_types_, 2);
//     ASSERT_EQ(ddsEnablerTest::received_data_, num_samples * 2);

//     ASSERT_NO_THROW(create_publisher(
//                 "DDSEnabler_Test_Participant_Publisher_Type3",
//                 ddsEnablerTest::dds_type3_name,
//                 ddsEnablerTest::dds_topic3_name,
//                 eprosima::fastdds::dds::TypeSupport(new DDSEnablerTestType3PubSubType())));
//     for (long i = 0; i < num_samples; i++)
//     {
//         send_data = send_sample();
//     }
//     ASSERT_EQ(ddsEnablerTest::received_types_, 3);
//     ASSERT_EQ(ddsEnablerTest::received_data_, num_samples * 3);

//     ASSERT_NO_THROW(create_publisher(
//                 "DDSEnabler_Test_Participant_Publisher_Type4",
//                 ddsEnablerTest::dds_type4_name,
//                 ddsEnablerTest::dds_topic4_name,
//                 eprosima::fastdds::dds::TypeSupport(new DDSEnablerTestType4PubSubType())));
//     for (long i = 0; i < num_samples; i++)
//     {
//         send_data = send_sample();
//     }
//     ASSERT_EQ(ddsEnablerTest::received_types_, 4);
//     ASSERT_EQ(ddsEnablerTest::received_data_, num_samples * 4);
// }

// TEST(DdsEnablerTests, ddsenabler_send_samples_repeated_type)
// {
//     auto enabler = create_ddsenabler();
//     ASSERT_TRUE(enabler != nullptr);

//     enabler.get()->set_data_callback(ddsEnablerTest::test_data_callback);
//     enabler.get()->set_type_callback(ddsEnablerTest::test_type_callback);

//     ASSERT_NO_THROW(create_publisher(
//                 "DDSEnabler_Test_Participant_Publisher_Type1",
//                 ddsEnablerTest::dds_type1_name,
//                 ddsEnablerTest::dds_topic1_name,
//                 eprosima::fastdds::dds::TypeSupport(new DDSEnablerTestType1PubSubType())));

//     ASSERT_EQ(ddsEnablerTest::received_types_, 0);
//     ASSERT_EQ(ddsEnablerTest::received_data_, 0);

//     // Send data
//     int num_samples = 1;
//     eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type send_data;
//     for (long i = 0; i < num_samples; i++)
//     {
//         send_data = send_sample();
//     }

//     ASSERT_EQ(ddsEnablerTest::received_types_, 1);
//     ASSERT_EQ(ddsEnablerTest::received_data_, num_samples);

//     ASSERT_NO_THROW(create_publisher(
//                 "DDSEnabler_Test_Participant_Publisher_Type3",
//                 ddsEnablerTest::dds_type3_name,
//                 ddsEnablerTest::dds_topic3_name,
//                 eprosima::fastdds::dds::TypeSupport(new DDSEnablerTestType3PubSubType())));
//     for (long i = 0; i < num_samples; i++)
//     {
//         send_data = send_sample();
//     }
//     ASSERT_EQ(ddsEnablerTest::received_types_, 2);
//     ASSERT_EQ(ddsEnablerTest::received_data_, num_samples * 2);

//     ASSERT_NO_THROW(create_publisher(
//                 "DDSEnabler_Test_Participant_Publisher_Type1",
//                 ddsEnablerTest::dds_type1_name,
//                 ddsEnablerTest::dds_topic1_name,
//                 eprosima::fastdds::dds::TypeSupport(new DDSEnablerTestType1PubSubType())));
//     for (long i = 0; i < num_samples; i++)
//     {
//         send_data = send_sample();
//     }
//     ASSERT_EQ(ddsEnablerTest::received_types_, 2);
//     ASSERT_EQ(ddsEnablerTest::received_data_, num_samples * 3);
// }

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
