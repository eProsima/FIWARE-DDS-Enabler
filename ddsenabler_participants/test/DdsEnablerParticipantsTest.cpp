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

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>


#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>

#include <CBHandler.hpp>
#include <CBHandlerConfiguration.hpp>
#include <CBMessage.hpp>
#include <CBWriter.hpp>

#include <thread>

#include "types/DDSEnablerTestTypesPubSubTypes.hpp"

using namespace eprosima;
using namespace eprosima::fastdds::dds;
using namespace eprosima::ddsenabler;

DynamicType::_ref_type create_schema(
        ddspipe::core::types::DdsTopic& topic)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(topic.type_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};

    return type_builder->build();
}

class CBWriterTest : public participants::CBWriter
{
public:

    CBWriterTest() = default;
    ~CBWriterTest() = default;

    // Expose protected methods
    using CBWriter::write_data;
    using CBWriter::write_schema;
};

class CBHandlerTest : public participants::CBHandler
{
public:

    CBHandlerTest(
            const participants::CBHandlerConfiguration& config,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool)
        : participants::CBHandler(config, payload_pool)
    {
        current_test_instance_ = this;

        cb_writer_ = std::make_unique<CBWriterTest>();

        // Set the callbacks
        set_data_callback(test_data_callback);
        set_type_callback(test_type_callback);
        set_topic_callback(test_topic_notification_callback);
        set_type_request_callback(test_type_request_callback);


    }

    // Expose protected members
    using participants::CBHandler::schemas_;
    using participants::CBHandler::cb_writer_;
    using participants::CBHandler::unique_sequence_number_;

    // eprosima::ddsenabler::participants::DdsTypeRequest type_req_callback;
    static void test_type_request_callback(
            const char* typeName,
            unsigned char*& serializedTypeInternal,
            uint32_t& serializedTypeInternalSize)
    {
        if(current_test_instance_ == nullptr)
            return;

        current_test_instance_->type_request_called++;
    }

    // eprosima::ddsenabler::participants::DdsNotification data_callback;
    static void test_data_callback(
            const char* topicName,
            const char* json,
            int64_t publishTime)
    {
        if(current_test_instance_ == nullptr)
            return;

        current_test_instance_->data_called_++;
    }

    // eprosima::ddsenabler::participants::DdsTypeNotification data_callback;
    static void test_type_callback(
            const char* typeName,
            const char* serializedType,
            const unsigned char* serializedTypeInternal,
            uint32_t serializedTypeInternalSize,
            const char* dataPlaceholder)
    {
        if(current_test_instance_ == nullptr)
            return;

        current_test_instance_->type_called_++;
    }

    // eprosima::ddsenabler::participants::DdsTopicNotification topic_callback;
    static void test_topic_notification_callback(
            const char* topicName,
            const char* typeName,
            const char* serializedQos)
    {
        if(current_test_instance_ == nullptr)
            return;

        current_test_instance_->topic_called_++;
    }

    uint32_t type_request_called = 0;
    uint32_t data_called_ = 0;
    uint32_t type_called_ = 0;
    uint32_t topic_called_ = 0;


    // Pointer to the current test instance (for use in the static callback)
    static CBHandlerTest* current_test_instance_;
};

CBHandlerTest* CBHandlerTest::current_test_instance_ = nullptr;

struct KnownType
{
    DynamicType::_ref_type dyn_type_;
    TypeSupport type_sup_;
    DataWriter* writer_ = nullptr;
};

bool test_create_publisher(
            KnownType& a_type,
            Topic** topic,
            std::string topic_name_suffix = "topic_name")
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
    topic_name << a_type.type_sup_.get_type_name() << topic_name_suffix;
    *topic = participant->create_topic(topic_name.str(), a_type.type_sup_.get_type_name(), TOPIC_QOS_DEFAULT);
    std::cout << (*topic)->get_name() << std::endl;
    if (*topic == nullptr)
    {
        std::cout << "ERROR DDSEnablerTester: create_topic: " <<
            a_type.type_sup_.get_type_name() << std::endl;
        return false;
    }

    DataWriterQos wqos = publisher->get_default_datawriter_qos();
    a_type.writer_ = publisher->create_datawriter(*topic, wqos);
    if (a_type.writer_ == nullptr)
    {
        std::cout << "ERROR DDSEnablerTester: create_datawriter: " <<
            a_type.type_sup_.get_type_name() << std::endl;
        return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return true;
}

ddspipe::core::types::DdsTopic new_schema(
    int num_type,
    DynamicType::_ref_type& dynamic_type,
    xtypes::TypeIdentifierPair& type_id_pair)
{
    KnownType k_type;
    switch(num_type)
    {
        case 3:
        {
            k_type.type_sup_.reset(new DDSEnablerTestType3PubSubType());
            break;
        }
        case 2:
        {
            k_type.type_sup_.reset(new DDSEnablerTestType2PubSubType());
            break;
        }
        case 1:
        default:
        {
            k_type.type_sup_.reset(new DDSEnablerTestType1PubSubType());
            break;
        }
    }

    eprosima::fastdds::dds::Topic* topic = nullptr;

    std::string str_topic_name = "topic_name" + std::to_string(num_type);
    test_create_publisher(k_type, &topic, str_topic_name);

    ddspipe::core::types::DdsTopic pipe_topic;
    pipe_topic.m_topic_name = topic->get_name();
    pipe_topic.type_name = topic->get_type_name();

    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        topic->get_type_name(), type_id_pair);
    pipe_topic.type_identifiers = type_id_pair;

    dynamic_type = create_schema(pipe_topic);
    return pipe_topic;
}

TEST(DdsEnablerParticipantsTest, ddsenabler_participants_cb_handler_creation)
{
    // Create Payload Pool
    auto payload_pool_ = std::make_shared<ddspipe::core::FastPayloadPool>();
    ASSERT_NE(payload_pool_, nullptr);

    // Create CB Handler configuration
    participants::CBHandlerConfiguration handler_config;

    // Create CB Handler
    auto cb_handler_ = std::make_shared<participants::CBHandler>(handler_config, payload_pool_);

    ASSERT_NE(cb_handler_, nullptr);
}

TEST(DdsEnablerParticipantsTest, ddsenabler_participants_add_new_schemas)
{
    // Create Payload Pool
    auto payload_pool_ = std::make_shared<ddspipe::core::FastPayloadPool>();
    ASSERT_NE(payload_pool_, nullptr);

    // Create CB Handler configuration
    participants::CBHandlerConfiguration handler_config;

    // Create CB Handler
    auto cb_handler_ = std::make_shared<CBHandlerTest>(handler_config, payload_pool_);
    ASSERT_NE(cb_handler_, nullptr);

    xtypes::TypeIdentifierPair type_id_pair;
    DynamicType::_ref_type dynamic_type;
    new_schema(1, dynamic_type, type_id_pair);

    ASSERT_TRUE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 0);
    cb_handler_->add_schema(dynamic_type, type_id_pair.type_identifier1());
    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);
    ASSERT_EQ(cb_handler_->type_called_, 1);



    xtypes::TypeIdentifierPair type_id_pair2;
    DynamicType::_ref_type dynamic_type2;
    new_schema(2, dynamic_type2, type_id_pair2);

    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);
    cb_handler_->add_schema(dynamic_type2, type_id_pair2.type_identifier1());
    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 2);
    ASSERT_EQ(cb_handler_->type_called_, 2);
}

TEST(DdsEnablerParticipantsTest, ddsenabler_participants_add_same_type_schema)
{
    // Create Payload Pool
    auto payload_pool_ = std::make_shared<ddspipe::core::FastPayloadPool>();
    ASSERT_NE(payload_pool_, nullptr);

    // Create CB Handler configuration
    participants::CBHandlerConfiguration handler_config;

    // Create CB Handler
    auto cb_handler_ = std::make_shared<CBHandlerTest>(handler_config, payload_pool_);
    ASSERT_NE(cb_handler_, nullptr);

    xtypes::TypeIdentifierPair type_id_pair;
    DynamicType::_ref_type dynamic_type;
    new_schema(1, dynamic_type, type_id_pair);

    ASSERT_TRUE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 0);
    cb_handler_->add_schema(dynamic_type, type_id_pair.type_identifier1());
    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);
    ASSERT_EQ(cb_handler_->type_called_, 1);



    xtypes::TypeIdentifierPair type_id_pair2;
    DynamicType::_ref_type dynamic_type2;
    new_schema(1, dynamic_type2, type_id_pair2);

    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);
    cb_handler_->add_schema(dynamic_type2, type_id_pair2.type_identifier1());
    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);
    ASSERT_EQ(cb_handler_->type_called_, 1);
}

TEST(DdsEnablerParticipantsTest, ddsenabler_participants_add_data_with_schema)
{
    // Create Payload Pool
    auto payload_pool_ = std::make_shared<ddspipe::core::FastPayloadPool>();
    ASSERT_NE(payload_pool_, nullptr);

    // Create CB Handler configuration
    participants::CBHandlerConfiguration handler_config;

    // Create CB Handler
    auto cb_handler_ = std::make_shared<CBHandlerTest>(handler_config, payload_pool_);
    ASSERT_NE(cb_handler_, nullptr);

    xtypes::TypeIdentifierPair type_id_pair;
    DynamicType::_ref_type dynamic_type;
    ddspipe::core::types::DdsTopic pipe_topic = new_schema(1, dynamic_type, type_id_pair);

    ASSERT_TRUE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 0);
    cb_handler_->add_schema(dynamic_type, type_id_pair.type_identifier1());
    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);
    ASSERT_EQ(cb_handler_->type_called_, 1);

    auto data = std::make_unique<eprosima::ddspipe::core::types::RtpsPayloadData>();
    eprosima::ddspipe::core::types::Payload payload;

    std::string content =
            "{\n"
            "    \"color\": \"RED\",\n"
            "    \"shapesize\": 30,\n"
            "    \"x\": 198,\n"
            "    \"y\": 189\n"
            "}";

    payload.length = static_cast<uint32_t>(content.length());
    payload.max_size = static_cast<uint32_t>(content.length());
    payload.data = new unsigned char[payload.length];
    std::memcpy(payload.data, content.data(), payload.length);

    payload_pool_->get_payload(payload, data->payload);
    payload.data = nullptr;     // Set to nullptr after copy to avoid free on destruction
    data->payload_owner = payload_pool_.get();

    ASSERT_NO_THROW(cb_handler_->add_data(pipe_topic, *data));
    // If the data has been added, the sequence number should be 1
    ASSERT_TRUE(cb_handler_->unique_sequence_number_ == 1);
    ASSERT_EQ(cb_handler_->data_called_, 1);
}

TEST(DdsEnablerParticipantsTest, ddsenabler_participants_add_data_without_schema)
{
    // Create Payload Pool
    auto payload_pool_ = std::make_shared<ddspipe::core::FastPayloadPool>();
    ASSERT_NE(payload_pool_, nullptr);

    // Create CB Handler configuration
    participants::CBHandlerConfiguration handler_config;

    // Create CB Handler
    auto cb_handler_ = std::make_shared<CBHandlerTest>(handler_config, payload_pool_);
    ASSERT_NE(cb_handler_, nullptr);

    xtypes::TypeIdentifierPair type_id_pair;
    DynamicType::_ref_type dynamic_type;
    ddspipe::core::types::DdsTopic pipe_topic = new_schema(1, dynamic_type, type_id_pair);

    auto data = std::make_unique<eprosima::ddspipe::core::types::RtpsPayloadData>();
    eprosima::ddspipe::core::types::Payload payload;

    std::string content =
            "{\n"
            "    \"color\": \"RED\",\n"
            "    \"shapesize\": 30,\n"
            "    \"x\": 198,\n"
            "    \"y\": 189\n"
            "}";

    payload.length = static_cast<uint32_t>(content.length());
    payload.max_size = static_cast<uint32_t>(content.length());
    payload.data = new unsigned char[payload.length];
    std::memcpy(payload.data, content.data(), payload.length);

    payload_pool_->get_payload(payload, data->payload);
    payload.data = nullptr;     // Set to nullptr after copy to avoid free on destruction
    data->payload_owner = payload_pool_.get();

    ASSERT_NO_THROW(cb_handler_->add_data(pipe_topic, *data));
    // As there is no schema associated, the sequence number should still be 0
    ASSERT_TRUE(cb_handler_->unique_sequence_number_ == 0);
    ASSERT_EQ(cb_handler_->data_called_, 0);
}

TEST(DdsEnablerParticipantsTest, ddsenabler_participants_write_schema_first_time)
{
    // Create Payload Pool
    auto payload_pool_ = std::make_shared<ddspipe::core::FastPayloadPool>();
    ASSERT_NE(payload_pool_, nullptr);

    // Create CB Handler configuration
    participants::CBHandlerConfiguration handler_config;

    // Create CB Handler
    auto cb_handler_ = std::make_shared<CBHandlerTest>(handler_config, payload_pool_);
    ASSERT_NE(cb_handler_, nullptr);

    xtypes::TypeIdentifierPair type_id_pair;
    DynamicType::_ref_type dynamic_type;
    ddspipe::core::types::DdsTopic pipe_topic = new_schema(1, dynamic_type, type_id_pair);

    auto data = std::make_unique<eprosima::ddspipe::core::types::RtpsPayloadData>();
    eprosima::ddspipe::core::types::Payload payload;

    std::string content =
            "{\n"
            "    \"color\": \"RED\",\n"
            "    \"shapesize\": 30,\n"
            "    \"x\": 198,\n"
            "    \"y\": 189\n"
            "}";

    payload.length = static_cast<uint32_t>(content.length());
    payload.max_size = static_cast<uint32_t>(content.length());
    payload.data = new unsigned char[payload.length];
    std::memcpy(payload.data, content.data(), payload.length);

    payload_pool_->get_payload(payload, data->payload);
    payload.data = nullptr;     // Set to nullptr after copy to avoid free on destruction
    data->payload_owner = payload_pool_.get();

    participants::CBMessage msg;
    msg.sequence_number = 1;
    msg.publish_time = data->source_timestamp;
    msg.topic = pipe_topic;
    msg.instanceHandle = data->instanceHandle;
    msg.source_guid = data->source_guid;
    payload_pool_->get_payload(data->payload, msg.payload);
    msg.payload_owner = payload_pool_.get();

    cb_handler_->cb_writer_->write_data(msg, dynamic_type);
    // The data will be successfully written as the dynamic type exists and we are bypassing the handler schema check
    ASSERT_EQ(cb_handler_->data_called_, 1);

    xtypes::TypeIdentifierPair type_id_pair2;
    DynamicType::_ref_type dynamic_type2;
    ddspipe::core::types::DdsTopic pipe_topic2 = new_schema(2, dynamic_type2, type_id_pair2);

    auto data2 = std::make_unique<eprosima::ddspipe::core::types::RtpsPayloadData>();
    eprosima::ddspipe::core::types::Payload payload2;

    payload2.length = static_cast<uint32_t>(content.length());
    payload2.max_size = static_cast<uint32_t>(content.length());
    payload2.data = new unsigned char[payload2.length];
    std::memcpy(payload2.data, content.data(), payload2.length);

    payload_pool_->get_payload(payload2, data2->payload);
    payload2.data = nullptr;     // Set to nullptr after copy to avoid free on destruction
    data2->payload_owner = payload_pool_.get();

    participants::CBMessage msg2;
    msg2.sequence_number = 1;
    msg2.publish_time = data2->source_timestamp;
    msg2.topic = pipe_topic2;
    msg2.instanceHandle = data2->instanceHandle;
    msg2.source_guid = data2->source_guid;
    payload_pool_->get_payload(data2->payload, msg2.payload);
    msg2.payload_owner = payload_pool_.get();

    cb_handler_->cb_writer_->write_data(msg2, dynamic_type2);
    ASSERT_EQ(cb_handler_->data_called_, 2);
}

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(Log::Kind::Warning);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
