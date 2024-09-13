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

// #include <cpp_utils/ros2_mangling.hpp>
// #include <cpp_utils/testing/gtest_aux.hpp>

#include <EnablerConfiguration.hpp>
#include <yaml_configuration_tags.hpp>

#include "DDSEnabler.hpp"

#include <cpp_utils/utils.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include "resources/types/HelloWorld.hpp"
#include "resources/types/HelloWorldPubSubTypes.hpp"
#include "resources/types/HelloWorldTypeObjectSupport.hpp"

using namespace eprosima::ddspipe;
using namespace eprosima::ddsenabler;
using namespace eprosima::ddsenabler::participants;
using namespace eprosima::fastdds::dds;


const char* CONFIGURATION_FILE = "src/lib/orionld/dds/ddsModule/ddsenabler/DDS_ENABLER_CONFIGURATION.yaml";

namespace test {

// Publisher


// void data_callback(
//         const CBMessage& msg,
//         nlohmann::ordered_json output)
// {
//     std::ofstream logFile("/tmp/data_callback.txt", std::ios_base::app);     // Use an absolute path
//     if (logFile.is_open())
//     {
//         logFile << "data_callback" << std::endl;
//         logFile.close();
//     }
// }

// void type_callback(
//         const CBMessage& msg,
//         const eprosima::fastdds::dds::DynamicType::_ref_type& dyn_type)
// {
//     std::ofstream logFile("/tmp/type_callback.txt", std::ios_base::app);     // Use an absolute path
//     if (logFile.is_open())
//     {
//         logFile << "type_callback" << std::endl;
//         logFile.close();
//     }
// }

const unsigned int DOMAIN = 0;

const std::string dds_topic_name = "DDSTopicName";
const std::string dds_type_name = "HelloWorld";

eprosima::fastdds::dds::DomainParticipant* participant_;

eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicType>::ref_type dynamic_type_;

eprosima::fastdds::dds::Publisher* publisher_;
eprosima::fastdds::dds::Topic* topic_;
eprosima::fastdds::dds::DataWriter* writer_;

} // test

void create_publisher(
        const std::string topic_name,
        const std::string type_name,
        const unsigned int domain)
{
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.name("DDSEnabler_Test_Participant_Publisher");

    // Create the Participant
    test::participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    // Register the type
    eprosima::fastdds::dds::TypeSupport type(new HelloWorldPubSubType());
    type->register_type_object_representation();

    eprosima::fastdds::dds::xtypes::TypeObjectPair dyn_type_objects;
    if (eprosima::fastdds::dds::RETCODE_OK !=
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                type_name, dyn_type_objects))
    {
        return;
    }

    test::dynamic_type_ = eprosima::fastdds::dds::DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
        dyn_type_objects.complete_type_object)->build();


    eprosima::fastdds::dds::DynamicData::_ref_type dyn_data(eprosima::fastdds::dds::DynamicDataFactory::get_instance()->
                    create_data(test::dynamic_type_));

    std::stringstream ss;
    ss << std::setw(4);
    eprosima::fastdds::dds::json_serialize(dyn_data,
            eprosima::fastdds::dds::DynamicDataJsonFormat::EPROSIMA, ss);

    std::cout << ss.str() << std::endl;

    // Register the type in the Participant
    test::participant_->register_type(type);

    // Create the Publisher
    test::publisher_ = test::participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    // Create the DDS Topic
    test::topic_ = test::participant_->create_topic(topic_name, type_name, TOPIC_QOS_DEFAULT);

    // Create the DDS DataWriter
    test::writer_ = test::publisher_->create_datawriter(test::topic_, DATAWRITER_QOS_DEFAULT, nullptr);
}

eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type send_sample(
        const unsigned int index = 1,
        const unsigned int time_sleep = 100)
{
    // Create and initialize new dynamic data

    if (test::dynamic_type_ == nullptr)
    {
        return nullptr;
    }

    auto dynamic_data_ = eprosima::fastdds::dds::DynamicDataFactory::get_instance()->create_data(test::dynamic_type_);

    // Set index
    dynamic_data_->set_uint32_value(dynamic_data_->get_member_id_by_name("index"), index);
    // Set message
    dynamic_data_->set_string_value(dynamic_data_->get_member_id_by_name("message"), "Test MSG: " + index);
    test::writer_->write(dynamic_data_.get());

    std::this_thread::sleep_for(std::chrono::milliseconds(time_sleep));

    return dynamic_data_;
}

std::unique_ptr<DDSEnabler> create_ddsenabler()
{
    YAML::Node yml;

    eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);

    auto close_handler = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

    return std::make_unique<DDSEnabler>(configuration, close_handler);
}

TEST(DdsEnablerTests, ddsenabler_creation)
{
    ASSERT_NO_THROW(auto enabler = create_ddsenabler());
}

TEST(DdsEnablerTests, ddsenabler_reload_configuration)
{
    auto enabler = create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    YAML::Node yml;
    eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);
    eprosima::utils::Formatter error_msg;
    ASSERT_TRUE(configuration.is_valid(error_msg));

    ASSERT_NO_THROW(enabler.get()->reload_configuration(configuration));
}

TEST(DdsEnablerTests, ddsenabler_send_samples)
{
    auto enabler = create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    ASSERT_NO_THROW(create_publisher(test::dds_topic_name, test::dds_type_name, test::DOMAIN));

    eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type send_data;

    // Send data
    for (unsigned int i = 0; i < 10; i++)
    {
        send_data = send_sample(i);
    }

    ASSERT_EQ(1, 1);
}

TEST(DdsEnablerTests, ddsenabler_FALSE)
{
    ASSERT_EQ(1, 2);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
