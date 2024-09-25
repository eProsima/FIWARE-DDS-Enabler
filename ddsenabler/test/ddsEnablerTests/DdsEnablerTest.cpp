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

namespace ddsenablertests {

struct PubKnownType
{
    DynamicType::_ref_type dyn_type_;
    TypeSupport type_sup_;
    DataWriter* writer_ = nullptr;
};

const unsigned int DOMAIN_ = 0;
static int num_samples_ =  3;
static int write_delay_ =  100;
static int received_types_ = 0;
static int received_data_ = 0;

// Callback to handle type notifications
static void test_type_callback(
        const char* typeName,
        const char* topicName,
        const char* serializedType)
{
    ddsenablertests::received_types_++;
}

// Callback to handle data notifications
static void test_data_callback(
        const char* typeName,
        const char* topicName,
        const char* json,
        double publishTime)
{
    ddsenablertests::received_data_++;
}

std::unique_ptr<DDSEnabler> create_ddsenabler()
{
    ddsenablertests::received_types_ = 0;
    ddsenablertests::received_data_ = 0;

    YAML::Node yml;

    eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);

    auto close_handler = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

    auto enabler = std::make_unique<DDSEnabler>(configuration, close_handler);

    enabler.get()->set_data_callback(ddsenablertests::test_data_callback);
    enabler.get()->set_type_callback(ddsenablertests::test_type_callback);

    return enabler;
}

bool send_samples(
        ddsenablertests::PubKnownType& a_type)
{
    for (long i = 0; i < ddsenablertests::num_samples_; i++)
    {
        void* sample = a_type.type_sup_.create_data();
        if (RETCODE_OK != a_type.writer_->write(sample))
        {
            std::cout << "ERROR ddsEnablerTest: fail writing sample " << a_type.type_sup_.get_type_name() << std::endl;
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(ddsenablertests::write_delay_));
        a_type.type_sup_.delete_data(sample);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(ddsenablertests::write_delay_));
    return true;
}

bool create_publisher(
        ddsenablertests::PubKnownType& a_type)
{
    // CREATE THE PARTICIPANT
    DomainParticipant* participant = DomainParticipantFactory::get_instance()
                    ->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    if (participant == nullptr)
    {
        std::cout << "ERROR ddsEnablerTest: create_participant" << std::endl;
        return false;
    }

    // REGISTER TYPE
    a_type.type_sup_.register_type(participant);

    // CREATE THE PUBLISHER
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    if (publisher == nullptr)
    {
        std::cout << "ERROR ddsEnablerTest: create_publisher: " << a_type.type_sup_.get_type_name() << std::endl;
        return false;
    }

    // CREATE THE TOPIC
    std::ostringstream topic_name;
    topic_name << a_type.type_sup_.get_type_name() << "TopicName";
    Topic* topic = participant->create_topic(topic_name.str(), a_type.type_sup_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic == nullptr)
    {
        std::cout << "ERROR ddsEnablerTest: create_topic: " << a_type.type_sup_.get_type_name() << std::endl;
        return false;
    }

    // CREATE THE DATAWRITER
    DataWriterQos wqos = publisher->get_default_datawriter_qos();
    wqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

    a_type.writer_ = publisher->create_datawriter(topic, wqos);
    if (a_type.writer_ == nullptr)
    {
        std::cout << "ERROR ddsEnablerTest: create_datawriter: " << a_type.type_sup_.get_type_name() << std::endl;
        return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return true;
}

} // ddsenablertests

TEST(DdsEnablerTests, ddsenabler_creation)
{
    ASSERT_NO_THROW(auto enabler = ddsenablertests::create_ddsenabler());
}

TEST(DdsEnablerTests, ddsenabler_reload_configuration)
{
    auto enabler = ddsenablertests::create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    YAML::Node yml;
    eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);
    eprosima::utils::Formatter error_msg;
    ASSERT_TRUE(configuration.is_valid(error_msg));

    ASSERT_NO_THROW(enabler.get()->reload_configuration(configuration));
}

TEST(DdsEnablerTests, ddsenabler_send_samples_type1)
{
    auto enabler = ddsenablertests::create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    ddsenablertests::PubKnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType1PubSubType());

    ASSERT_TRUE(ddsenablertests::create_publisher(a_type));

    ASSERT_EQ(ddsenablertests::received_types_, 0);
    ASSERT_EQ(ddsenablertests::received_data_, 0);

    // Send data
    ASSERT_TRUE(ddsenablertests::send_samples(a_type));

    ASSERT_EQ(ddsenablertests::received_types_, 1);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_);
}

TEST(DdsEnablerTests, ddsenabler_send_samples_type2)
{
    auto enabler = ddsenablertests::create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    ddsenablertests::PubKnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType2PubSubType());

    ASSERT_TRUE(ddsenablertests::create_publisher(a_type));

    ASSERT_EQ(ddsenablertests::received_types_, 0);
    ASSERT_EQ(ddsenablertests::received_data_, 0);

    // Send data
    ASSERT_TRUE(ddsenablertests::send_samples(a_type));

    ASSERT_EQ(ddsenablertests::received_types_, 1);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_);
}

TEST(DdsEnablerTests, ddsenabler_send_samples_type3)
{
    auto enabler = ddsenablertests::create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    ddsenablertests::PubKnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType3PubSubType());

    ASSERT_TRUE(ddsenablertests::create_publisher(a_type));

    ASSERT_EQ(ddsenablertests::received_types_, 0);
    ASSERT_EQ(ddsenablertests::received_data_, 0);

    // Send data
    ASSERT_TRUE(ddsenablertests::send_samples(a_type));

    ASSERT_EQ(ddsenablertests::received_types_, 1);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_);
}

TEST(DdsEnablerTests, ddsenabler_send_samples_type4)
{
    auto enabler = ddsenablertests::create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    ddsenablertests::PubKnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType4PubSubType());

    ASSERT_TRUE(ddsenablertests::create_publisher(a_type));

    ASSERT_EQ(ddsenablertests::received_types_, 0);
    ASSERT_EQ(ddsenablertests::received_data_, 0);

    // Send data
    ASSERT_TRUE(ddsenablertests::send_samples(a_type));

    ASSERT_EQ(ddsenablertests::received_types_, 1);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_);
}

TEST(DdsEnablerTests, ddsenabler_send_samples_multiple_types)
{
    auto enabler = ddsenablertests::create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    enabler.get()->set_data_callback(ddsenablertests::test_data_callback);
    enabler.get()->set_type_callback(ddsenablertests::test_type_callback);

    ddsenablertests::PubKnownType a_type1;
    a_type1.type_sup_.reset(new DDSEnablerTestType1PubSubType());

    ASSERT_TRUE(ddsenablertests::create_publisher(a_type1));

    ASSERT_EQ(ddsenablertests::received_types_, 0);
    ASSERT_EQ(ddsenablertests::received_data_, 0);

    // Send data
    ASSERT_TRUE(ddsenablertests::send_samples(a_type1));

    ASSERT_EQ(ddsenablertests::received_types_, 1);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_);

    ddsenablertests::PubKnownType a_type2;
    a_type2.type_sup_.reset(new DDSEnablerTestType2PubSubType());

    ASSERT_TRUE(ddsenablertests::create_publisher(a_type2));

    ASSERT_EQ(ddsenablertests::received_types_, 1);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_);

    // Send data
    ASSERT_TRUE(ddsenablertests::send_samples(a_type2));

    ASSERT_EQ(ddsenablertests::received_types_, 2);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_ * 2);

    ddsenablertests::PubKnownType a_type3;
    a_type3.type_sup_.reset(new DDSEnablerTestType3PubSubType());

    ASSERT_TRUE(ddsenablertests::create_publisher(a_type3));

    ASSERT_EQ(ddsenablertests::received_types_, 2);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_ * 2);

    // Send data
    ASSERT_TRUE(ddsenablertests::send_samples(a_type3));

    ASSERT_EQ(ddsenablertests::received_types_, 3);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_ * 3);
}

TEST(DdsEnablerTests, ddsenabler_send_samples_repeated_type)
{
    auto enabler = ddsenablertests::create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    enabler.get()->set_data_callback(ddsenablertests::test_data_callback);
    enabler.get()->set_type_callback(ddsenablertests::test_type_callback);

    ddsenablertests::PubKnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType1PubSubType());

    ASSERT_TRUE(ddsenablertests::create_publisher(a_type));

    ASSERT_EQ(ddsenablertests::received_types_, 0);
    ASSERT_EQ(ddsenablertests::received_data_, 0);

    // Send data
    ASSERT_TRUE(ddsenablertests::send_samples(a_type));

    ASSERT_EQ(ddsenablertests::received_types_, 1);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_);

    ddsenablertests::PubKnownType another_type;
    another_type.type_sup_.reset(new DDSEnablerTestType2PubSubType());

    ASSERT_TRUE(ddsenablertests::create_publisher(another_type));

    // Send data
    ASSERT_TRUE(ddsenablertests::send_samples(another_type));

    ASSERT_EQ(ddsenablertests::received_types_, 2);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_ * 2);

    ddsenablertests::PubKnownType same_type;
    same_type.type_sup_.reset(new DDSEnablerTestType1PubSubType());

    ASSERT_TRUE(ddsenablertests::create_publisher(same_type));

    ASSERT_EQ(ddsenablertests::received_types_, 2);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_ * 2);

    // Send data
    ASSERT_TRUE(ddsenablertests::send_samples(same_type));

    ASSERT_EQ(ddsenablertests::received_types_, 2);
    ASSERT_EQ(ddsenablertests::received_data_, ddsenablertests::num_samples_ * 3);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}