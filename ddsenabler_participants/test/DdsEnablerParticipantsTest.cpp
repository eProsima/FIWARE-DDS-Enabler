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

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
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
    using CBWriter::stored_schemas_;
};

class CBHandlerTest : public participants::CBHandler
{
public:

    // Inherit the constructor from CBHandler
    using participants::CBHandler::CBHandler;

    // Expose protected members
    using participants::CBHandler::schemas_;
    using participants::CBHandler::cb_writer_;
};

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

    // Replace CBWriter with test version
    std::unique_ptr<CBWriterTest> cb_writer_test = std::make_unique<CBWriterTest>();
    ASSERT_NE(cb_writer_test, nullptr);
    cb_handler_->cb_writer_ = std::move(cb_writer_test);

    ddspipe::core::types::DdsTopic topic;
    topic.m_topic_name = "topic1";
    topic.type_name = "type1";
    xtypes::TypeIdentifier type_id;
    xtypes::EquivalenceHash hash = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    type_id.equivalence_hash(hash);
    topic.type_identifiers.type_identifier1(type_id);

    DynamicType::_ref_type dynamic_type;
    dynamic_type = create_schema(topic);
    ASSERT_NE(dynamic_type, nullptr);

    ASSERT_TRUE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 0);
    cb_handler_->add_schema(dynamic_type, type_id);
    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);

    ddspipe::core::types::DdsTopic topic2;
    topic2.m_topic_name = "topic2";
    topic2.type_name = "type2";
    xtypes::TypeIdentifier type_id2;
    xtypes::EquivalenceHash hash2 = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
    type_id2.equivalence_hash(hash2);
    topic2.type_identifiers.type_identifier1(type_id2);

    DynamicType::_ref_type dynamic_type2;
    dynamic_type2 = create_schema(topic2);
    ASSERT_NE(dynamic_type2, nullptr);

    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);
    cb_handler_->add_schema(dynamic_type2, type_id2);
    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 2);
}

TEST(DdsEnablerParticipantsTest, ddsenabler_participants_add_same_type_id_schema)
{
    // Create Payload Pool
    auto payload_pool_ = std::make_shared<ddspipe::core::FastPayloadPool>();
    ASSERT_NE(payload_pool_, nullptr);

    // Create CB Handler configuration
    participants::CBHandlerConfiguration handler_config;

    // Create CB Handler
    auto cb_handler_ = std::make_shared<CBHandlerTest>(handler_config, payload_pool_);
    ASSERT_NE(cb_handler_, nullptr);

    // Replace CBWriter with test version
    std::unique_ptr<CBWriterTest> cb_writer_test = std::make_unique<CBWriterTest>();
    ASSERT_NE(cb_writer_test, nullptr);
    cb_handler_->cb_writer_ = std::move(cb_writer_test);

    ddspipe::core::types::DdsTopic topic;
    topic.m_topic_name = "topic1";
    topic.type_name = "type1";
    xtypes::TypeIdentifier type_id;
    xtypes::EquivalenceHash hash = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    type_id.equivalence_hash(hash);
    topic.type_identifiers.type_identifier1(type_id);

    DynamicType::_ref_type dynamic_type;
    dynamic_type = create_schema(topic);
    ASSERT_NE(dynamic_type, nullptr);

    ASSERT_TRUE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 0);
    cb_handler_->add_schema(dynamic_type, type_id);
    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);

    ddspipe::core::types::DdsTopic topic2;
    topic2.m_topic_name = "topic2";
    topic2.type_name = "type2";
    xtypes::TypeIdentifier type_id2;
    type_id2.equivalence_hash(hash);
    topic2.type_identifiers.type_identifier1(type_id2);

    DynamicType::_ref_type dynamic_type2;
    dynamic_type2 = create_schema(topic2);
    ASSERT_NE(dynamic_type2, nullptr);

    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);
    cb_handler_->add_schema(dynamic_type2, type_id2);
    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);
}

TEST(DdsEnablerParticipantsTest, ddsenabler_participants_add_same_type_name_schema)
{
    // Create Payload Pool
    auto payload_pool_ = std::make_shared<ddspipe::core::FastPayloadPool>();
    ASSERT_NE(payload_pool_, nullptr);

    // Create CB Handler configuration
    participants::CBHandlerConfiguration handler_config;

    // Create CB Handler
    auto cb_handler_ = std::make_shared<CBHandlerTest>(handler_config, payload_pool_);
    ASSERT_NE(cb_handler_, nullptr);

    // Replace CBWriter with test version
    std::unique_ptr<CBWriterTest> cb_writer_test = std::make_unique<CBWriterTest>();
    ASSERT_NE(cb_writer_test, nullptr);
    cb_handler_->cb_writer_ = std::move(cb_writer_test);

    ddspipe::core::types::DdsTopic topic;
    topic.m_topic_name = "topic1";
    topic.type_name = "type1";
    xtypes::TypeIdentifier type_id;
    xtypes::EquivalenceHash hash = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    type_id.equivalence_hash(hash);
    topic.type_identifiers.type_identifier1(type_id);

    DynamicType::_ref_type dynamic_type;
    dynamic_type = create_schema(topic);
    ASSERT_NE(dynamic_type, nullptr);

    ASSERT_TRUE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 0);
    cb_handler_->add_schema(dynamic_type, type_id);
    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);

    ddspipe::core::types::DdsTopic topic2;
    topic2.m_topic_name = "topic2";
    topic2.type_name = "type1";
    xtypes::TypeIdentifier type_id2;
    xtypes::EquivalenceHash hash2 = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
    type_id2.equivalence_hash(hash2);
    topic2.type_identifiers.type_identifier1(type_id2);

    DynamicType::_ref_type dynamic_type2;
    dynamic_type2 = create_schema(topic2);
    ASSERT_NE(dynamic_type2, nullptr);

    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);
    cb_handler_->add_schema(dynamic_type2, type_id2);
    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 2);
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

    // Replace CBWriter with test version
    std::unique_ptr<CBWriterTest> cb_writer_test = std::make_unique<CBWriterTest>();
    ASSERT_NE(cb_writer_test, nullptr);
    cb_handler_->cb_writer_ = std::move(cb_writer_test);

    ddspipe::core::types::DdsTopic topic;
    topic.m_topic_name = "topic1";
    topic.type_name = "type1";
    xtypes::TypeIdentifier type_id;
    xtypes::EquivalenceHash hash = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    type_id.equivalence_hash(hash);
    topic.type_identifiers.type_identifier1(type_id);

    DynamicType::_ref_type dynamic_type;
    dynamic_type = create_schema(topic);
    ASSERT_NE(dynamic_type, nullptr);

    ASSERT_TRUE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 0);
    cb_handler_->add_schema(dynamic_type, type_id);
    ASSERT_FALSE(cb_handler_->schemas_.empty());
    ASSERT_EQ(cb_handler_->schemas_.size(), 1);

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
    payload.data = (unsigned char*)reinterpret_cast<const unsigned char*>(content.data());

    payload_pool_->get_payload(payload, data->payload);
    payload.data = nullptr;     // Set to nullptr after copy to avoid free on destruction
    data->payload_owner = payload_pool_.get();

    ASSERT_NO_THROW(cb_handler_->add_data(topic, *data));
}

TEST(DdsEnablerParticipantsTest, ddsenabler_participants_write_schema_first_time)
{
    // Create Payload Pool
    auto payload_pool_ = std::make_shared<ddspipe::core::FastPayloadPool>();
    ASSERT_NE(payload_pool_, nullptr);

    std::unique_ptr<CBWriterTest> cb_writer_test = std::make_unique<CBWriterTest>();
    ASSERT_NE(cb_writer_test, nullptr);

    ddspipe::core::types::DdsTopic topic;
    topic.m_topic_name = "topic1";
    topic.type_name = "type1";
    xtypes::TypeIdentifier type_id;
    xtypes::EquivalenceHash hash = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    type_id.equivalence_hash(hash);
    topic.type_identifiers.type_identifier1(type_id);

    DynamicType::_ref_type dynamic_type;
    dynamic_type = create_schema(topic);
    ASSERT_NE(dynamic_type, nullptr);

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
    payload.data = (unsigned char*)reinterpret_cast<const unsigned char*>(content.data());

    payload_pool_->get_payload(payload, data->payload);
    payload.data = nullptr;     // Set to nullptr after copy to avoid free on destruction
    data->payload_owner = payload_pool_.get();

    participants::CBMessage msg;
    msg.sequence_number = 1;
    msg.publish_time = data->source_timestamp;
    msg.topic = topic;
    msg.instanceHandle = data->instanceHandle;
    msg.source_guid = data->source_guid;
    payload_pool_->get_payload(data->payload, msg.payload);
    msg.payload_owner = payload_pool_.get();

    ASSERT_TRUE(cb_writer_test->stored_schemas_.empty());
    ASSERT_EQ(cb_writer_test->stored_schemas_.size(), 0);
    cb_writer_test->write_schema(msg, dynamic_type);
    ASSERT_FALSE(cb_writer_test->stored_schemas_.empty());
    ASSERT_EQ(cb_writer_test->stored_schemas_.size(), 1);

    ddspipe::core::types::DdsTopic topic2;
    topic2.m_topic_name = "topic2";
    topic2.type_name = "type2";
    xtypes::TypeIdentifier type_id2;
    xtypes::EquivalenceHash hash2 = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
    type_id2.equivalence_hash(hash2);
    topic2.type_identifiers.type_identifier1(type_id2);

    DynamicType::_ref_type dynamic_type2;
    dynamic_type2 = create_schema(topic2);
    ASSERT_NE(dynamic_type2, nullptr);

    auto data2 = std::make_unique<eprosima::ddspipe::core::types::RtpsPayloadData>();
    eprosima::ddspipe::core::types::Payload payload2;

    payload2.length = static_cast<uint32_t>(content.length());
    payload2.max_size = static_cast<uint32_t>(content.length());
    payload2.data = (unsigned char*)reinterpret_cast<const unsigned char*>(content.data());

    payload_pool_->get_payload(payload2, data2->payload);
    payload2.data = nullptr;     // Set to nullptr after copy to avoid free on destruction
    data2->payload_owner = payload_pool_.get();

    participants::CBMessage msg2;
    msg2.sequence_number = 1;
    msg2.publish_time = data2->source_timestamp;
    msg2.topic = topic2;
    msg2.instanceHandle = data2->instanceHandle;
    msg2.source_guid = data2->source_guid;
    payload_pool_->get_payload(data2->payload, msg2.payload);
    msg2.payload_owner = payload_pool_.get();

    ASSERT_FALSE(cb_writer_test->stored_schemas_.empty());
    ASSERT_EQ(cb_writer_test->stored_schemas_.size(), 1);
    cb_writer_test->write_schema(msg2, dynamic_type2);
    ASSERT_FALSE(cb_writer_test->stored_schemas_.empty());
    ASSERT_EQ(cb_writer_test->stored_schemas_.size(), 2);
}

TEST(DdsEnablerParticipantsTest, ddsenabler_participants_write_schema_repeated)
{
    // Create Payload Pool
    auto payload_pool_ = std::make_shared<ddspipe::core::FastPayloadPool>();
    ASSERT_NE(payload_pool_, nullptr);

    std::unique_ptr<CBWriterTest> cb_writer_test = std::make_unique<CBWriterTest>();
    ASSERT_NE(cb_writer_test, nullptr);

    ddspipe::core::types::DdsTopic topic;
    topic.m_topic_name = "topic1";
    topic.type_name = "type1";
    xtypes::TypeIdentifier type_id;
    xtypes::EquivalenceHash hash = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    type_id.equivalence_hash(hash);
    topic.type_identifiers.type_identifier1(type_id);

    DynamicType::_ref_type dynamic_type;
    dynamic_type = create_schema(topic);
    ASSERT_NE(dynamic_type, nullptr);

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
    payload.data = (unsigned char*)reinterpret_cast<const unsigned char*>(content.data());

    payload_pool_->get_payload(payload, data->payload);
    payload.data = nullptr;     // Set to nullptr after copy to avoid free on destruction
    data->payload_owner = payload_pool_.get();

    participants::CBMessage msg;
    msg.sequence_number = 1;
    msg.publish_time = data->source_timestamp;
    msg.topic = topic;
    msg.instanceHandle = data->instanceHandle;
    msg.source_guid = data->source_guid;
    payload_pool_->get_payload(data->payload, msg.payload);
    msg.payload_owner = payload_pool_.get();

    ASSERT_TRUE(cb_writer_test->stored_schemas_.empty());
    ASSERT_EQ(cb_writer_test->stored_schemas_.size(), 0);
    cb_writer_test->write_schema(msg, dynamic_type);
    ASSERT_FALSE(cb_writer_test->stored_schemas_.empty());
    ASSERT_EQ(cb_writer_test->stored_schemas_.size(), 1);

    cb_writer_test->write_schema(msg, dynamic_type);
    ASSERT_FALSE(cb_writer_test->stored_schemas_.empty());
    ASSERT_EQ(cb_writer_test->stored_schemas_.size(), 1);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
