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

#include "../ddsEnablerTypedTests/DdsEnablerTypedTestTypeHeaders.hpp"

using namespace eprosima::ddspipe;
using namespace eprosima::ddsenabler;
using namespace eprosima::ddsenabler::participants;
using namespace eprosima::fastdds::dds;

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

#define DEFINE_DDSENABLER_TYPED_TEST(TestName, Type) \
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



// This macros are updated automatically using the update_headers_and_create_cases.py script
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasAlias, AliasAliasPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasArray, AliasArrayPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasBitmask, AliasBitmaskPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasBitset, AliasBitsetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasBool, AliasBoolPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasChar16, AliasChar16PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasChar8, AliasChar8PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasEnum, AliasEnumPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasFloat128, AliasFloat128PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasFloat32, AliasFloat32PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasFloat64, AliasFloat64PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasInt16, AliasInt16PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasInt32, AliasInt32PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasInt64, AliasInt64PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasMap, AliasMapPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasMultiArray, AliasMultiArrayPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasOctet, AliasOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasSequence, AliasSequencePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasString16, AliasString16PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasString8, AliasString8PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasStruct, AliasStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasUInt32, AliasUInt32PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasUInt64, AliasUInt64PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasUint16, AliasUint16PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AliasUnion, AliasUnionPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_BasicAnnotationsStruct, BasicAnnotationsStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_EmptyAnnotatedStruct, EmptyAnnotatedStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableBooleanStruct, AppendableBooleanStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableCharStruct, AppendableCharStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableDoubleStruct, AppendableDoubleStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableEmptyInheritanceStruct, AppendableEmptyInheritanceStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableEmptyStruct, AppendableEmptyStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableExtensibilityInheritance, AppendableExtensibilityInheritancePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableFloatStruct, AppendableFloatStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableInheritanceEmptyStruct, AppendableInheritanceEmptyStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableInheritanceStruct, AppendableInheritanceStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableLongDoubleStruct, AppendableLongDoubleStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableLongLongStruct, AppendableLongLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableLongStruct, AppendableLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableOctetStruct, AppendableOctetStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableShortStruct, AppendableShortStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableULongLongStruct, AppendableULongLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableULongStruct, AppendableULongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableUShortStruct, AppendableUShortStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableUnionStruct, AppendableUnionStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_AppendableWCharStruct, AppendableWCharStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayAlias, ArrayAliasPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayBitMask, ArrayBitMaskPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayBitset, ArrayBitsetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayBoolean, ArrayBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayBoundedString, ArrayBoundedStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayBoundedWString, ArrayBoundedWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayChar, ArrayCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayDouble, ArrayDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayEnum, ArrayEnumPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayFloat, ArrayFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayLong, ArrayLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayLongDouble, ArrayLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayLongLong, ArrayLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMap, ArrayMapPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionAlias, ArrayMultiDimensionAliasPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionBitMask, ArrayMultiDimensionBitMaskPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionBitset, ArrayMultiDimensionBitsetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionBoolean, ArrayMultiDimensionBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionBoundedString, ArrayMultiDimensionBoundedStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionBoundedWString, ArrayMultiDimensionBoundedWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionChar, ArrayMultiDimensionCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionDouble, ArrayMultiDimensionDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionEnum, ArrayMultiDimensionEnumPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionFloat, ArrayMultiDimensionFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsAlias, ArrayMultiDimensionLiteralsAliasPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsBitMask, ArrayMultiDimensionLiteralsBitMaskPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsBitSet, ArrayMultiDimensionLiteralsBitSetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsBoolean, ArrayMultiDimensionLiteralsBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsBoundedString, ArrayMultiDimensionLiteralsBoundedStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsBoundedWString, ArrayMultiDimensionLiteralsBoundedWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsChar, ArrayMultiDimensionLiteralsCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsDouble, ArrayMultiDimensionLiteralsDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsEnum, ArrayMultiDimensionLiteralsEnumPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsFloat, ArrayMultiDimensionLiteralsFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsLong, ArrayMultiDimensionLiteralsLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsLongDouble, ArrayMultiDimensionLiteralsLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsLongLong, ArrayMultiDimensionLiteralsLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsMap, ArrayMultiDimensionLiteralsMapPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsOctet, ArrayMultiDimensionLiteralsOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsSequence, ArrayMultiDimensionLiteralsSequencePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsShort, ArrayMultiDimensionLiteralsShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsString, ArrayMultiDimensionLiteralsStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsStructure, ArrayMultiDimensionLiteralsStructurePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsULong, ArrayMultiDimensionLiteralsULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsULongLong, ArrayMultiDimensionLiteralsULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsUShort, ArrayMultiDimensionLiteralsUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsUnion, ArrayMultiDimensionLiteralsUnionPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsWChar, ArrayMultiDimensionLiteralsWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLiteralsWString, ArrayMultiDimensionLiteralsWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLong, ArrayMultiDimensionLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLongDouble, ArrayMultiDimensionLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionLongLong, ArrayMultiDimensionLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionMap, ArrayMultiDimensionMapPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionOctet, ArrayMultiDimensionOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionSequence, ArrayMultiDimensionSequencePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionShort, ArrayMultiDimensionShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionString, ArrayMultiDimensionStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionStructure, ArrayMultiDimensionStructurePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionULong, ArrayMultiDimensionULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionULongLong, ArrayMultiDimensionULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionUShort, ArrayMultiDimensionUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionUnion, ArrayMultiDimensionUnionPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionWChar, ArrayMultiDimensionWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayMultiDimensionWString, ArrayMultiDimensionWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayOctet, ArrayOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySequence, ArraySequencePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayShort, ArrayShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayShortArray, ArrayShortArrayPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsAlias, ArraySingleDimensionLiteralsAliasPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsBitMask, ArraySingleDimensionLiteralsBitMaskPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsBitset, ArraySingleDimensionLiteralsBitsetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsBoolean, ArraySingleDimensionLiteralsBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsBoundedString, ArraySingleDimensionLiteralsBoundedStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsBoundedWString, ArraySingleDimensionLiteralsBoundedWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsChar, ArraySingleDimensionLiteralsCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsDouble, ArraySingleDimensionLiteralsDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsEnum, ArraySingleDimensionLiteralsEnumPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsFloat, ArraySingleDimensionLiteralsFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsLong, ArraySingleDimensionLiteralsLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsLongDouble, ArraySingleDimensionLiteralsLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsLongLong, ArraySingleDimensionLiteralsLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsMap, ArraySingleDimensionLiteralsMapPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsOctet, ArraySingleDimensionLiteralsOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsSequence, ArraySingleDimensionLiteralsSequencePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsShort, ArraySingleDimensionLiteralsShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsShortArray, ArraySingleDimensionLiteralsShortArrayPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsString, ArraySingleDimensionLiteralsStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsStructure, ArraySingleDimensionLiteralsStructurePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsUnion, ArraySingleDimensionLiteralsUnionPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsUnsignedLong, ArraySingleDimensionLiteralsUnsignedLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsUnsignedLongLong, ArraySingleDimensionLiteralsUnsignedLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsUnsignedShort, ArraySingleDimensionLiteralsUnsignedShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsWChar, ArraySingleDimensionLiteralsWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArraySingleDimensionLiteralsWString, ArraySingleDimensionLiteralsWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayString, ArrayStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayStructure, ArrayStructurePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayUInt8, ArrayUInt8PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayULong, ArrayULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayULongLong, ArrayULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayUShort, ArrayUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayUnion, ArrayUnionPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayWChar, ArrayWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ArrayWString, ArrayWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_BoundedBigArrays, BoundedBigArraysPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_BoundedSmallArrays, BoundedSmallArraysPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_BitsetStruct, BitsetStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ConstsLiteralsStruct, ConstsLiteralsStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_Module2ConstsLiteralsStruct, const_module2::Module2ConstsLiteralsStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ModuleConstsLiteralsStruct, const_module1::ModuleConstsLiteralsStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_BitMaskStructure, BitMaskStructurePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_BoundedBitMaskStructure, BoundedBitMaskStructurePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_EnumStructure, EnumStructurePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalBooleanStruct, FinalBooleanStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalCharStruct, FinalCharStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalDoubleStruct, FinalDoubleStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalEmptyInheritanceStruct, FinalEmptyInheritanceStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalEmptyStruct, FinalEmptyStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalExtensibilityInheritance, FinalExtensibilityInheritancePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalFloatStruct, FinalFloatStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalInheritanceStruct, FinalInheritanceStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalLongDoubleStruct, FinalLongDoubleStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalLongLongStruct, FinalLongLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalLongStruct, FinalLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalOctetStruct, FinalOctetStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalShortStruct, FinalShortStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalULongLongStruct, FinalULongLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalULongStruct, FinalULongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalUShortStruct, FinalUShortStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalUnionStruct, FinalUnionStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FinalWCharStruct, FinalWCharStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_InheritanceEmptyStruct, InheritanceEmptyStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_InnerEmptyStructureHelper, InnerEmptyStructureHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_InnerStructureHelper, InnerStructureHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_BitsetsChildInheritanceStruct, BitsetsChildInheritanceStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_InnerEmptyStructureHelperChild, InnerEmptyStructureHelperChildPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_InnerStructureHelperChild, InnerStructureHelperChildPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_InnerStructureHelperChildChild, InnerStructureHelperChildChildPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_InnerStructureHelperEmptyChild, InnerStructureHelperEmptyChildPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_InnerStructureHelperEmptyChildChild, InnerStructureHelperEmptyChildChildPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructAliasInheritanceStruct, StructAliasInheritanceStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructuresInheritanceStruct, StructuresInheritanceStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_InheritanceKeyedEmptyStruct, InheritanceKeyedEmptyStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedAppendable, KeyedAppendablePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedBooleanStruct, KeyedBooleanStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedCharStruct, KeyedCharStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedDoubleStruct, KeyedDoubleStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedEmptyInheritanceStruct, KeyedEmptyInheritanceStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedEmptyStruct, KeyedEmptyStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedFinal, KeyedFinalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedFloatStruct, KeyedFloatStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedInheritanceStruct, KeyedInheritanceStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedLongDoubleStruct, KeyedLongDoubleStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedLongLongStruct, KeyedLongLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedLongStruct, KeyedLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedMutable, KeyedMutablePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedOctetStruct, KeyedOctetStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedShortStruct, KeyedShortStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedULongLongStruct, KeyedULongLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedULongStruct, KeyedULongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedUShortStruct, KeyedUShortStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_KeyedWCharStruct, KeyedWCharStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_BoundedLargeMap, BoundedLargeMapPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_BoundedSmallMap, BoundedSmallMapPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperBoolean, MapInnerAliasBoundedStringHelperBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperChar, MapInnerAliasBoundedStringHelperCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperDouble, MapInnerAliasBoundedStringHelperDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperFloat, MapInnerAliasBoundedStringHelperFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperInnerAliasArrayHelper, MapInnerAliasBoundedStringHelperInnerAliasArrayHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperInnerAliasBoundedStringHelper, MapInnerAliasBoundedStringHelperInnerAliasBoundedStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperInnerAliasBoundedWStringHelper, MapInnerAliasBoundedStringHelperInnerAliasBoundedWStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperInnerAliasHelper, MapInnerAliasBoundedStringHelperInnerAliasHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperInnerAliasMapHelper, MapInnerAliasBoundedStringHelperInnerAliasMapHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperInnerAliasSequenceHelper, MapInnerAliasBoundedStringHelperInnerAliasSequenceHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperInnerBitMaskHelper, MapInnerAliasBoundedStringHelperInnerBitMaskHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperInnerBitsetHelper, MapInnerAliasBoundedStringHelperInnerBitsetHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperInnerEnumHelper, MapInnerAliasBoundedStringHelperInnerEnumHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperInnerStructureHelper, MapInnerAliasBoundedStringHelperInnerStructureHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperInnerUnionHelper, MapInnerAliasBoundedStringHelperInnerUnionHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperLong, MapInnerAliasBoundedStringHelperLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperLongDouble, MapInnerAliasBoundedStringHelperLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperLongLong, MapInnerAliasBoundedStringHelperLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperOctet, MapInnerAliasBoundedStringHelperOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperShort, MapInnerAliasBoundedStringHelperShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperString, MapInnerAliasBoundedStringHelperStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperULong, MapInnerAliasBoundedStringHelperULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperULongLong, MapInnerAliasBoundedStringHelperULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperUShort, MapInnerAliasBoundedStringHelperUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperWChar, MapInnerAliasBoundedStringHelperWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapInnerAliasBoundedStringHelperWString, MapInnerAliasBoundedStringHelperWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapKeyULongLongValueDouble, MapKeyULongLongValueDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapKeyULongValueLongDouble, MapKeyULongValueLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapKeyULongValueLongLong, MapKeyULongValueLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongBoolean, MapLongBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongChar, MapLongCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongDouble, MapLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongFloat, MapLongFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongInnerAliasArrayHelper, MapLongInnerAliasArrayHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongInnerAliasBoundedStringHelper, MapLongInnerAliasBoundedStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongInnerAliasBoundedWStringHelper, MapLongInnerAliasBoundedWStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongInnerAliasHelper, MapLongInnerAliasHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongInnerAliasMapHelper, MapLongInnerAliasMapHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongInnerAliasSequenceHelper, MapLongInnerAliasSequenceHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongInnerBitMaskHelper, MapLongInnerBitMaskHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongInnerBitsetHelper, MapLongInnerBitsetHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongInnerEnumHelper, MapLongInnerEnumHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongInnerStructureHelper, MapLongInnerStructureHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongInnerUnionHelper, MapLongInnerUnionHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongKeyLongDoubleValue, MapLongKeyLongDoubleValuePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongKeyLongLongValue, MapLongKeyLongLongValuePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLong, MapLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongBoolean, MapLongLongBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongChar, MapLongLongCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongFloat, MapLongLongFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongInnerAliasArrayHelper, MapLongLongInnerAliasArrayHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongInnerAliasBoundedStringHelper, MapLongLongInnerAliasBoundedStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongInnerAliasBoundedWStringHelper, MapLongLongInnerAliasBoundedWStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongInnerAliasHelper, MapLongLongInnerAliasHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongInnerAliasMapHelper, MapLongLongInnerAliasMapHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongInnerAliasSequenceHelper, MapLongLongInnerAliasSequenceHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongInnerBitMaskHelper, MapLongLongInnerBitMaskHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongInnerBitsetHelper, MapLongLongInnerBitsetHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongInnerEnumHelper, MapLongLongInnerEnumHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongInnerStructureHelper, MapLongLongInnerStructureHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongInnerUnionHelper, MapLongLongInnerUnionHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongKeyDoubleValue, MapLongLongKeyDoubleValuePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongKeyLongValue, MapLongLongKeyLongValuePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongLongDouble, MapLongLongLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongLongLong, MapLongLongLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongOctet, MapLongLongOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongShort, MapLongLongShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongString, MapLongLongStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongULong, MapLongLongULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongULongLong, MapLongLongULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongUShort, MapLongLongUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongWChar, MapLongLongWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongLongWString, MapLongLongWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongOctet, MapLongOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongShort, MapLongShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongString, MapLongStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongULong, MapLongULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongULongLong, MapLongULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongUShort, MapLongUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongWChar, MapLongWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapLongWString, MapLongWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortBoolean, MapShortBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortChar, MapShortCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortDouble, MapShortDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortFloat, MapShortFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortInnerAliasArrayHelper, MapShortInnerAliasArrayHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortInnerAliasBoundedStringHelper, MapShortInnerAliasBoundedStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortInnerAliasBoundedWStringHelper, MapShortInnerAliasBoundedWStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortInnerAliasHelper, MapShortInnerAliasHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortInnerAliasMapHelper, MapShortInnerAliasMapHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortInnerAliasSequenceHelper, MapShortInnerAliasSequenceHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortInnerBitMaskHelper, MapShortInnerBitMaskHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortInnerBitsetHelper, MapShortInnerBitsetHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortInnerEnumHelper, MapShortInnerEnumHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortInnerStructureHelper, MapShortInnerStructureHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortInnerUnionHelper, MapShortInnerUnionHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortLong, MapShortLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortLongDouble, MapShortLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortLongLong, MapShortLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortOctet, MapShortOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortShort, MapShortShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortString, MapShortStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortULong, MapShortULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortULongLong, MapShortULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortUShort, MapShortUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortWChar, MapShortWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapShortWString, MapShortWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringBoolean, MapStringBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringChar, MapStringCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringDouble, MapStringDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringFloat, MapStringFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringInnerAliasArrayHelper, MapStringInnerAliasArrayHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringInnerAliasBoundedStringHelper, MapStringInnerAliasBoundedStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringInnerAliasBoundedWStringHelper, MapStringInnerAliasBoundedWStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringInnerAliasHelper, MapStringInnerAliasHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringInnerAliasMapHelper, MapStringInnerAliasMapHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringInnerAliasSequenceHelper, MapStringInnerAliasSequenceHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringInnerBitMaskHelper, MapStringInnerBitMaskHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringInnerBitsetHelper, MapStringInnerBitsetHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringInnerEnumHelper, MapStringInnerEnumHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringInnerStructureHelper, MapStringInnerStructureHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringInnerUnionHelper, MapStringInnerUnionHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringLong, MapStringLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringLongDouble, MapStringLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringLongLong, MapStringLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringOctet, MapStringOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringShort, MapStringShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringString, MapStringStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringULong, MapStringULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringULongLong, MapStringULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringUShort, MapStringUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringWChar, MapStringWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapStringWString, MapStringWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongBoolean, MapULongBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongChar, MapULongCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongDouble, MapULongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongFloat, MapULongFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongInnerAliasArrayHelper, MapULongInnerAliasArrayHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongInnerAliasBoundedStringHelper, MapULongInnerAliasBoundedStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongInnerAliasBoundedWStringHelper, MapULongInnerAliasBoundedWStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongInnerAliasHelper, MapULongInnerAliasHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongInnerAliasMapHelper, MapULongInnerAliasMapHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongInnerAliasSequenceHelper, MapULongInnerAliasSequenceHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongInnerBitMaskHelper, MapULongInnerBitMaskHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongInnerBitsetHelper, MapULongInnerBitsetHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongInnerEnumHelper, MapULongInnerEnumHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongInnerStructureHelper, MapULongInnerStructureHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongInnerUnionHelper, MapULongInnerUnionHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLong, MapULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongBoolean, MapULongLongBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongChar, MapULongLongCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongFloat, MapULongLongFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongInnerAliasArrayHelper, MapULongLongInnerAliasArrayHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongInnerAliasBoundedStringHelper, MapULongLongInnerAliasBoundedStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongInnerAliasBoundedWStringHelper, MapULongLongInnerAliasBoundedWStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongInnerAliasHelper, MapULongLongInnerAliasHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongInnerAliasMapHelper, MapULongLongInnerAliasMapHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongInnerAliasSequenceHelper, MapULongLongInnerAliasSequenceHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongInnerBitMaskHelper, MapULongLongInnerBitMaskHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongInnerBitsetHelper, MapULongLongInnerBitsetHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongInnerEnumHelper, MapULongLongInnerEnumHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongInnerStructureHelper, MapULongLongInnerStructureHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongInnerUnionHelper, MapULongLongInnerUnionHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongLong, MapULongLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongLongDouble, MapULongLongLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongLongLong, MapULongLongLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongOctet, MapULongLongOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongShort, MapULongLongShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongString, MapULongLongStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongULong, MapULongLongULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongULongLong, MapULongLongULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongUShort, MapULongLongUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongWChar, MapULongLongWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongLongWString, MapULongLongWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongOctet, MapULongOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongShort, MapULongShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongString, MapULongStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongULong, MapULongULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongULongLong, MapULongULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongUShort, MapULongUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongWChar, MapULongWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapULongWString, MapULongWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortBoolean, MapUShortBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortChar, MapUShortCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortDouble, MapUShortDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortFloat, MapUShortFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortInnerAliasArrayHelper, MapUShortInnerAliasArrayHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortInnerAliasBoundedStringHelper, MapUShortInnerAliasBoundedStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortInnerAliasBoundedWStringHelper, MapUShortInnerAliasBoundedWStringHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortInnerAliasHelper, MapUShortInnerAliasHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortInnerAliasMapHelper, MapUShortInnerAliasMapHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortInnerAliasSequenceHelper, MapUShortInnerAliasSequenceHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortInnerBitMaskHelper, MapUShortInnerBitMaskHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortInnerBitsetHelper, MapUShortInnerBitsetHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortInnerEnumHelper, MapUShortInnerEnumHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortInnerStructureHelper, MapUShortInnerStructureHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortInnerUnionHelper, MapUShortInnerUnionHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortLong, MapUShortLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortLongDouble, MapUShortLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortLongLong, MapUShortLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortOctet, MapUShortOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortShort, MapUShortShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortString, MapUShortStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortULong, MapUShortULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortULongLong, MapUShortULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortUShort, MapUShortUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortWChar, MapUShortWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MapUShortWString, MapUShortWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableBooleanStruct, MutableBooleanStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableCharStruct, MutableCharStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableDoubleStruct, MutableDoubleStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableEmptyInheritanceStruct, MutableEmptyInheritanceStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableEmptyStruct, MutableEmptyStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableExtensibilityInheritance, MutableExtensibilityInheritancePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableFloatStruct, MutableFloatStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableInheritanceEmptyStruct, MutableInheritanceEmptyStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableInheritanceStruct, MutableInheritanceStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableLongDoubleStruct, MutableLongDoubleStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableLongLongStruct, MutableLongLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableLongStruct, MutableLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableOctetStruct, MutableOctetStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableShortStruct, MutableShortStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableULongLongStruct, MutableULongLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableULongStruct, MutableULongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableUShortStruct, MutableUShortStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableUnionStruct, MutableUnionStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_MutableWCharStruct, MutableWCharStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_InnerStructOptional, InnerStructOptionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_array_short_align_1_optional, array_short_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_array_short_align_2_optional, array_short_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_array_short_align_4_optional, array_short_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_array_short_optional, array_short_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_boolean_align_1_optional, boolean_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_boolean_align_2_optional, boolean_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_boolean_align_4_optional, boolean_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_boolean_optional, boolean_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_char_align_1_optional, char_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_char_align_2_optional, char_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_char_align_4_optional, char_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_char_optional, char_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_double_align_1_optional, double_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_double_align_2_optional, double_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_double_align_4_optional, double_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_double_optional, double_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_float_align_1_optional, float_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_float_align_2_optional, float_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_float_align_4_optional, float_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_float_optional, float_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_long_align_1_optional, long_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_long_align_2_optional, long_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_long_align_4_optional, long_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_long_optional, long_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_longdouble_align_1_optional, longdouble_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_longdouble_align_2_optional, longdouble_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_longdouble_align_4_optional, longdouble_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_longdouble_optional, longdouble_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_longlong_align_1_optional, longlong_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_longlong_align_2_optional, longlong_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_longlong_align_4_optional, longlong_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_longlong_optional, longlong_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_map_short_align_1_optional, map_short_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_map_short_align_2_optional, map_short_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_map_short_align_4_optional, map_short_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_map_short_optional, map_short_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_octet_align_1_optional, octet_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_octet_align_2_optional, octet_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_octet_align_4_optional, octet_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_octet_optional, octet_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_opt_struct_align_1_optional, opt_struct_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_opt_struct_align_2_optional, opt_struct_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_opt_struct_align_4_optional, opt_struct_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_opt_struct_optional, opt_struct_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_sequence_short_align_1_optional, sequence_short_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_sequence_short_align_2_optional, sequence_short_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_sequence_short_align_4_optional, sequence_short_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_sequence_short_optional, sequence_short_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_short_align_1_optional, short_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_short_align_2_optional, short_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_short_align_4_optional, short_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_short_optional, short_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_string_bounded_align_1_optional, string_bounded_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_string_bounded_align_2_optional, string_bounded_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_string_bounded_align_4_optional, string_bounded_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_string_bounded_optional, string_bounded_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_string_unbounded_align_1_optional, string_unbounded_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_string_unbounded_align_2_optional, string_unbounded_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_string_unbounded_align_4_optional, string_unbounded_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_string_unbounded_optional, string_unbounded_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_struct_align_1_optional, struct_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_struct_align_2_optional, struct_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_struct_align_4_optional, struct_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_struct_optional, struct_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ulong_align_1_optional, ulong_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ulong_align_2_optional, ulong_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ulong_align_4_optional, ulong_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ulong_optional, ulong_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ulonglong_align_1_optional, ulonglong_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ulonglong_align_2_optional, ulonglong_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ulonglong_align_4_optional, ulonglong_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ulonglong_optional, ulonglong_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ushort_align_1_optional, ushort_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ushort_align_2_optional, ushort_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ushort_align_4_optional, ushort_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ushort_optional, ushort_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_wchar_align_1_optional, wchar_align_1_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_wchar_align_2_optional, wchar_align_2_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_wchar_align_4_optional, wchar_align_4_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_wchar_optional, wchar_optionalPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_BooleanStruct, BooleanStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_CharStruct, CharStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_DoubleStruct, DoubleStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_FloatStruct, FloatStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_Int16Struct, Int16StructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_Int32Struct, Int32StructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_Int64Struct, Int64StructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_Int8Struct, Int8StructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_LongDoubleStruct, LongDoubleStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_LongLongStruct, LongLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_LongStruct, LongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_OctetStruct, OctetStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ShortStruct, ShortStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ULongLongStruct, ULongLongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_ULongStruct, ULongStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UShortStruct, UShortStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_Uint16Struct, Uint16StructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_Uint32Struct, Uint32StructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_Uint64Struct, Uint64StructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_Uint8Struct, Uint8StructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_WCharStruct, WCharStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_BoundedBigSequences, BoundedBigSequencesPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_BoundedSmallSequences, BoundedSmallSequencesPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceAlias, SequenceAliasPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceBitMask, SequenceBitMaskPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceBitset, SequenceBitsetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceBoolean, SequenceBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceChar, SequenceCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceDouble, SequenceDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceEnum, SequenceEnumPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceFloat, SequenceFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceLong, SequenceLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceLongDouble, SequenceLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceLongLong, SequenceLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceMap, SequenceMapPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceOctet, SequenceOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceSequence, SequenceSequencePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceShort, SequenceShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceShortArray, SequenceShortArrayPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceString, SequenceStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceStringBounded, SequenceStringBoundedPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceStructure, SequenceStructurePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceULong, SequenceULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceULongLong, SequenceULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceUShort, SequenceUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceUnion, SequenceUnionPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceWChar, SequenceWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceWString, SequenceWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SequenceWStringBounded, SequenceWStringBoundedPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_LargeStringStruct, LargeStringStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_LargeWStringStruct, LargeWStringStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SmallStringStruct, SmallStringStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_SmallWStringStruct, SmallWStringStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StringStruct, StringStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_WStringStruct, WStringStructPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructAlias, StructAliasPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructBitMask, StructBitMaskPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructBitset, StructBitsetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructBoolean, StructBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructBoundedString, StructBoundedStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructBoundedWString, StructBoundedWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructChar16, StructChar16PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructChar8, StructChar8PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructDouble, StructDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructEmpty, StructEmptyPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructEnum, StructEnumPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructFloat, StructFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructLong, StructLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructLongDouble, StructLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructLongLong, StructLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructMap, StructMapPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructOctet, StructOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructSequence, StructSequencePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructShort, StructShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructShortArray, StructShortArrayPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructString, StructStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructStructure, StructStructurePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructUnion, StructUnionPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructUnsignedLong, StructUnsignedLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructUnsignedLongLong, StructUnsignedLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructUnsignedShort, StructUnsignedShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_StructWString, StructWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_Structures, StructuresPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_bar, barPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_root, rootPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_root1, root1PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_root2, root2PubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionArray, UnionArrayPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionBoolean, UnionBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionBoundedString, UnionBoundedStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionBoundedWString, UnionBoundedWStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionChar, UnionCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorAlias, UnionDiscriminatorAliasPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorBoolean, UnionDiscriminatorBooleanPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorChar, UnionDiscriminatorCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorEnum, UnionDiscriminatorEnumPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorEnumLabel, UnionDiscriminatorEnumLabelPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorLong, UnionDiscriminatorLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorLongLong, UnionDiscriminatorLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorOctet, UnionDiscriminatorOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorShort, UnionDiscriminatorShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorULong, UnionDiscriminatorULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorULongLong, UnionDiscriminatorULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorUShort, UnionDiscriminatorUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDiscriminatorWChar, UnionDiscriminatorWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionDouble, UnionDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionFloat, UnionFloatPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionInnerAliasHelper, UnionInnerAliasHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionInnerBitMaskHelper, UnionInnerBitMaskHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionInnerBitsetHelper, UnionInnerBitsetHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionInnerEnumHelper, UnionInnerEnumHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionInnerStructureHelper, UnionInnerStructureHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionInnerUnionHelper, UnionInnerUnionHelperPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionLong, UnionLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionLongDouble, UnionLongDoublePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionLongLong, UnionLongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionMap, UnionMapPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionOctet, UnionOctetPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionSequence, UnionSequencePubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionSeveralFields, UnionSeveralFieldsPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionSeveralFieldsWithDefault, UnionSeveralFieldsWithDefaultPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionShort, UnionShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionString, UnionStringPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionULong, UnionULongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionULongLong, UnionULongLongPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionUShort, UnionUShortPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionWChar, UnionWCharPubSubType);
DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_UnionWString, UnionWStringPubSubType);


} // namespace ddsenablertypedtests


int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}