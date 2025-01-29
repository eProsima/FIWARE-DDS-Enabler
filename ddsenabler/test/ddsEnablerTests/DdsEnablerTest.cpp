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

#include <gtest/gtest.h>

#include "types/DDSEnablerTestTypesPubSubTypes.hpp"
#include "DDSEnablerTester.hpp"

using namespace ddsenablertester;
using namespace eprosima::ddspipe;
using namespace eprosima::ddsenabler::participants;
using namespace eprosima::fastdds::dds;

// Define static pointer
ddsenablertester::DDSEnablerTester* ddsenablertester::DDSEnablerTester::current_test_instance_ = nullptr;
class DDSEnablerTest : public ddsenablertester::DDSEnablerTester
{
};

TEST_F(DDSEnablerTest, ddsenabler_creation)
{
    ASSERT_NO_THROW(auto enabler = create_ddsenabler());
}

TEST_F(DDSEnablerTest, ddsenabler_reload_configuration)
{
    auto enabler = create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    YAML::Node yml;
    eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);
    eprosima::utils::Formatter error_msg;
    ASSERT_TRUE(configuration.is_valid(error_msg));

    ASSERT_NO_THROW(enabler->reload_configuration(configuration));
}

TEST_F(DDSEnablerTest, send_type1)
{
    ddsenablertester::num_samples_ = 3;

    auto enabler = create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    KnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType1PubSubType());

    ASSERT_TRUE(create_publisher(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), 0);

    // Send data
    ASSERT_TRUE(send_samples(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), num_samples_);
}

TEST_F(DDSEnablerTest, send_many_type1)
{
    ddsenablertester::num_samples_ = 1000;

    auto enabler = create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    KnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType1PubSubType());

    ASSERT_TRUE(create_publisher(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), 0);

    // Send data
    ASSERT_TRUE(send_samples(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), num_samples_);
}

TEST_F(DDSEnablerTest, send_type2)
{
    ddsenablertester::num_samples_ = 3;

    auto enabler = create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    KnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType2PubSubType());

    ASSERT_TRUE(create_publisher(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), 0);

    // Send data
    ASSERT_TRUE(send_samples(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), num_samples_);
}

TEST_F(DDSEnablerTest, send_type3)
{
    ddsenablertester::num_samples_ = 3;

    auto enabler = create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    KnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType3PubSubType());

    ASSERT_TRUE(create_publisher(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), 0);

    // Send data
    ASSERT_TRUE(send_samples(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), num_samples_);

    // Send data
    ASSERT_TRUE(send_samples(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), num_samples_ * 2);
}

TEST_F(DDSEnablerTest, send_type4)
{
    ddsenablertester::num_samples_ = 3;

    auto enabler = create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    KnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType4PubSubType());

    ASSERT_TRUE(create_publisher(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), 0);

    // Send data
    ASSERT_TRUE(send_samples(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), num_samples_);
}

TEST_F(DDSEnablerTest, send_multiple_types)
{
    ddsenablertester::num_samples_ = 3;

    auto enabler = create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    KnownType a_type1;
    a_type1.type_sup_.reset(new DDSEnablerTestType1PubSubType());

    ASSERT_TRUE(create_publisher(a_type1));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), 0);

    // Send data
    ASSERT_TRUE(send_samples(a_type1));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), num_samples_);

    KnownType a_type2;
    a_type2.type_sup_.reset(new DDSEnablerTestType2PubSubType());

    ASSERT_TRUE(create_publisher(a_type2));

    ASSERT_EQ(get_received_types(), 2);
    ASSERT_EQ(get_received_data(), num_samples_);

    // Send data
    ASSERT_TRUE(send_samples(a_type2));

    ASSERT_EQ(get_received_types(), 2);
    ASSERT_EQ(get_received_data(), num_samples_ * 2);

    KnownType a_type3;
    a_type3.type_sup_.reset(new DDSEnablerTestType3PubSubType());

    ASSERT_TRUE(create_publisher(a_type3));

    ASSERT_EQ(get_received_types(), 3);
    ASSERT_EQ(get_received_data(), num_samples_ * 2);

    // Send data
    ASSERT_TRUE(send_samples(a_type3));

    ASSERT_EQ(get_received_types(), 3);
    ASSERT_EQ(get_received_data(), num_samples_ * 3);

    KnownType a_type4;
    a_type4.type_sup_.reset(new DDSEnablerTestType4PubSubType());

    ASSERT_TRUE(create_publisher(a_type4));

    ASSERT_EQ(get_received_types(), 4);
    ASSERT_EQ(get_received_data(), num_samples_ * 3);

    // Send data
    ASSERT_TRUE(send_samples(a_type4));

    ASSERT_EQ(get_received_types(), 4);
    ASSERT_EQ(get_received_data(), num_samples_ * 4);
}

TEST_F(DDSEnablerTest, send_repeated_type)
{
    ddsenablertester::num_samples_ = 3;

    auto enabler = create_ddsenabler();
    ASSERT_TRUE(enabler != nullptr);

    KnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType1PubSubType());

    ASSERT_TRUE(create_publisher(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), 0);

    // Send data
    ASSERT_TRUE(send_samples(a_type));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), num_samples_);

    KnownType another_type;
    another_type.type_sup_.reset(new DDSEnablerTestType2PubSubType());

    ASSERT_TRUE(create_publisher(another_type));

    // Send data
    ASSERT_TRUE(send_samples(another_type));

    ASSERT_EQ(get_received_types(), 2);
    ASSERT_EQ(get_received_data(), num_samples_ * 2);

    KnownType same_type;
    same_type.type_sup_.reset(new DDSEnablerTestType1PubSubType());

    ASSERT_TRUE(create_publisher(same_type));

    ASSERT_EQ(get_received_types(), 2);
    ASSERT_EQ(get_received_data(), num_samples_ * 2);

    // Send data
    ASSERT_TRUE(send_samples(same_type));

    ASSERT_EQ(get_received_types(), 2);
    ASSERT_EQ(get_received_data(), num_samples_ * 3);
}

TEST_F(DDSEnablerTest, send_history_bigger_than_writer)
{
    ddsenablertester::num_samples_ = 5;
    int history_depth = 3;

    KnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType1PubSubType());

    ASSERT_TRUE(create_publisher_w_history(a_type, history_depth));
    // Send data
    ASSERT_TRUE(send_samples(a_type));

    auto enabler = create_ddsenabler_w_history();
    ASSERT_TRUE(enabler != nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(history_depth * 100));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), history_depth);
}

TEST_F(DDSEnablerTest, send_history_smaller_than_writer)
{
    ddsenablertester::num_samples_ = 20;
    int history_depth = 15;

    KnownType a_type;
    a_type.type_sup_.reset(new DDSEnablerTestType1PubSubType());

    ASSERT_TRUE(create_publisher_w_history(a_type, history_depth));
    // Send data
    ASSERT_TRUE(send_samples(a_type));

    auto enabler = create_ddsenabler_w_history();
    ASSERT_TRUE(enabler != nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(history_depth * 100));

    ASSERT_EQ(get_received_types(), 1);
    ASSERT_EQ(get_received_data(), history_depth);
}

TEST_F(DDSEnablerTest, send_history_multiple_types)
{
    ddsenablertester::num_samples_ = 5;
    int types = 4;
    int history_depth = 3;

    KnownType a_type1;
    a_type1.type_sup_.reset(new DDSEnablerTestType1PubSubType());

    ASSERT_TRUE(create_publisher_w_history(a_type1, history_depth));
    // Send data
    ASSERT_TRUE(send_samples(a_type1));

    KnownType a_type2;
    a_type2.type_sup_.reset(new DDSEnablerTestType2PubSubType());

    ASSERT_TRUE(create_publisher_w_history(a_type2, history_depth));
    // Send data
    ASSERT_TRUE(send_samples(a_type2));

    KnownType a_type3;
    a_type3.type_sup_.reset(new DDSEnablerTestType3PubSubType());

    ASSERT_TRUE(create_publisher_w_history(a_type3, history_depth));
    // Send data
    ASSERT_TRUE(send_samples(a_type3));

    KnownType a_type4;
    a_type4.type_sup_.reset(new DDSEnablerTestType4PubSubType());

    ASSERT_TRUE(create_publisher_w_history(a_type4, history_depth));
    // Send data
    ASSERT_TRUE(send_samples(a_type4));

    auto enabler = create_ddsenabler_w_history();
    ASSERT_TRUE(enabler != nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(types * history_depth * 100));

    ASSERT_EQ(get_received_types(), types);
    ASSERT_EQ(get_received_data(), types * history_depth);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}