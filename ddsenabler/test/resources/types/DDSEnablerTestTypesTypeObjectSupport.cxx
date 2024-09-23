// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file DDSEnablerTestTypesTypeObjectSupport.cxx
 * Source file containing the implementation to register the TypeObject representation of the described types in the IDL file
 *
 * This file was generated by the tool fastddsgen.
 */

#include "DDSEnablerTestTypesTypeObjectSupport.hpp"

#include <mutex>
#include <string>

#include <fastcdr/xcdr/external.hpp>
#include <fastcdr/xcdr/optional.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

#include "DDSEnablerTestTypes.hpp"


using namespace eprosima::fastdds::dds::xtypes;

// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_DDSEnablerTestType1_type_identifier(
        TypeIdentifierPair& type_ids_DDSEnablerTestType1)
{

    ReturnCode_t return_code_DDSEnablerTestType1 {eprosima::fastdds::dds::RETCODE_OK};
    return_code_DDSEnablerTestType1 =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        "DDSEnablerTestType1", type_ids_DDSEnablerTestType1);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_DDSEnablerTestType1)
    {
        StructTypeFlag struct_flags_DDSEnablerTestType1 = TypeObjectUtils::build_struct_type_flag(eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE,
                false, false);
        QualifiedTypeName type_name_DDSEnablerTestType1 = "DDSEnablerTestType1";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_DDSEnablerTestType1;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_DDSEnablerTestType1;
        CompleteTypeDetail detail_DDSEnablerTestType1 = TypeObjectUtils::build_complete_type_detail(type_ann_builtin_DDSEnablerTestType1, ann_custom_DDSEnablerTestType1, type_name_DDSEnablerTestType1.to_string());
        CompleteStructHeader header_DDSEnablerTestType1;
        header_DDSEnablerTestType1 = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail_DDSEnablerTestType1);
        CompleteStructMemberSeq member_seq_DDSEnablerTestType1;
        {
            TypeIdentifierPair type_ids_value;
            ReturnCode_t return_code_value {eprosima::fastdds::dds::RETCODE_OK};
            return_code_value =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "_int16_t", type_ids_value);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_value)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                        "value Structure member TypeIdentifier unknown to TypeObjectRegistry.");
                return;
            }
            StructMemberFlag member_flags_value = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_value = 0x00000000;
            bool common_value_ec {false};
            CommonStructMember common_value {TypeObjectUtils::build_common_struct_member(member_id_value, member_flags_value, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_value, common_value_ec))};
            if (!common_value_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure value member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_value = "value";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_value;
            ann_custom_DDSEnablerTestType1.reset();
            CompleteMemberDetail detail_value = TypeObjectUtils::build_complete_member_detail(name_value, member_ann_builtin_value, ann_custom_DDSEnablerTestType1);
            CompleteStructMember member_value = TypeObjectUtils::build_complete_struct_member(common_value, detail_value);
            TypeObjectUtils::add_complete_struct_member(member_seq_DDSEnablerTestType1, member_value);
        }
        CompleteStructType struct_type_DDSEnablerTestType1 = TypeObjectUtils::build_complete_struct_type(struct_flags_DDSEnablerTestType1, header_DDSEnablerTestType1, member_seq_DDSEnablerTestType1);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_DDSEnablerTestType1, type_name_DDSEnablerTestType1.to_string(), type_ids_DDSEnablerTestType1))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "DDSEnablerTestType1 already registered in TypeObjectRegistry for a different type.");
        }
    }
}
// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_DDSEnablerTestType2_type_identifier(
        TypeIdentifierPair& type_ids_DDSEnablerTestType2)
{

    ReturnCode_t return_code_DDSEnablerTestType2 {eprosima::fastdds::dds::RETCODE_OK};
    return_code_DDSEnablerTestType2 =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        "DDSEnablerTestType2", type_ids_DDSEnablerTestType2);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_DDSEnablerTestType2)
    {
        StructTypeFlag struct_flags_DDSEnablerTestType2 = TypeObjectUtils::build_struct_type_flag(eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE,
                false, false);
        QualifiedTypeName type_name_DDSEnablerTestType2 = "DDSEnablerTestType2";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_DDSEnablerTestType2;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_DDSEnablerTestType2;
        CompleteTypeDetail detail_DDSEnablerTestType2 = TypeObjectUtils::build_complete_type_detail(type_ann_builtin_DDSEnablerTestType2, ann_custom_DDSEnablerTestType2, type_name_DDSEnablerTestType2.to_string());
        CompleteStructHeader header_DDSEnablerTestType2;
        header_DDSEnablerTestType2 = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail_DDSEnablerTestType2);
        CompleteStructMemberSeq member_seq_DDSEnablerTestType2;
        {
            TypeIdentifierPair type_ids_value;
            ReturnCode_t return_code_value {eprosima::fastdds::dds::RETCODE_OK};
            return_code_value =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "anonymous_string_unbounded", type_ids_value);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_value)
            {
                {
                    SBound bound = 0;
                    StringSTypeDefn string_sdefn = TypeObjectUtils::build_string_s_type_defn(bound);
                    if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                            TypeObjectUtils::build_and_register_s_string_type_identifier(string_sdefn,
                            "anonymous_string_unbounded", type_ids_value))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                            "anonymous_string_unbounded already registered in TypeObjectRegistry for a different type.");
                    }
                }
            }
            StructMemberFlag member_flags_value = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_value = 0x00000000;
            bool common_value_ec {false};
            CommonStructMember common_value {TypeObjectUtils::build_common_struct_member(member_id_value, member_flags_value, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_value, common_value_ec))};
            if (!common_value_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure value member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_value = "value";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_value;
            ann_custom_DDSEnablerTestType2.reset();
            CompleteMemberDetail detail_value = TypeObjectUtils::build_complete_member_detail(name_value, member_ann_builtin_value, ann_custom_DDSEnablerTestType2);
            CompleteStructMember member_value = TypeObjectUtils::build_complete_struct_member(common_value, detail_value);
            TypeObjectUtils::add_complete_struct_member(member_seq_DDSEnablerTestType2, member_value);
        }
        CompleteStructType struct_type_DDSEnablerTestType2 = TypeObjectUtils::build_complete_struct_type(struct_flags_DDSEnablerTestType2, header_DDSEnablerTestType2, member_seq_DDSEnablerTestType2);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_DDSEnablerTestType2, type_name_DDSEnablerTestType2.to_string(), type_ids_DDSEnablerTestType2))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "DDSEnablerTestType2 already registered in TypeObjectRegistry for a different type.");
        }
    }
}
// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_DDSEnablerTestType3_type_identifier(
        TypeIdentifierPair& type_ids_DDSEnablerTestType3)
{

    ReturnCode_t return_code_DDSEnablerTestType3 {eprosima::fastdds::dds::RETCODE_OK};
    return_code_DDSEnablerTestType3 =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        "DDSEnablerTestType3", type_ids_DDSEnablerTestType3);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_DDSEnablerTestType3)
    {
        StructTypeFlag struct_flags_DDSEnablerTestType3 = TypeObjectUtils::build_struct_type_flag(eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE,
                false, false);
        QualifiedTypeName type_name_DDSEnablerTestType3 = "DDSEnablerTestType3";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_DDSEnablerTestType3;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_DDSEnablerTestType3;
        CompleteTypeDetail detail_DDSEnablerTestType3 = TypeObjectUtils::build_complete_type_detail(type_ann_builtin_DDSEnablerTestType3, ann_custom_DDSEnablerTestType3, type_name_DDSEnablerTestType3.to_string());
        CompleteStructHeader header_DDSEnablerTestType3;
        header_DDSEnablerTestType3 = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail_DDSEnablerTestType3);
        CompleteStructMemberSeq member_seq_DDSEnablerTestType3;
        {
            TypeIdentifierPair type_ids_value;
            ReturnCode_t return_code_value {eprosima::fastdds::dds::RETCODE_OK};
            return_code_value =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "anonymous_array_int32_t_10", type_ids_value);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_value)
            {
                return_code_value =
                    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                    "_int32_t", type_ids_value);

                if (eprosima::fastdds::dds::RETCODE_OK != return_code_value)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                            "Array element TypeIdentifier unknown to TypeObjectRegistry.");
                    return;
                }
                bool element_identifier_anonymous_array_int32_t_10_ec {false};
                TypeIdentifier* element_identifier_anonymous_array_int32_t_10 {new TypeIdentifier(TypeObjectUtils::retrieve_complete_type_identifier(type_ids_value, element_identifier_anonymous_array_int32_t_10_ec))};
                if (!element_identifier_anonymous_array_int32_t_10_ec)
                {
                    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Array element TypeIdentifier inconsistent.");
                    return;
                }
                EquivalenceKind equiv_kind_anonymous_array_int32_t_10 = EK_COMPLETE;
                if (TK_NONE == type_ids_value.type_identifier2()._d())
                {
                    equiv_kind_anonymous_array_int32_t_10 = EK_BOTH;
                }
                CollectionElementFlag element_flags_anonymous_array_int32_t_10 = 0;
                PlainCollectionHeader header_anonymous_array_int32_t_10 = TypeObjectUtils::build_plain_collection_header(equiv_kind_anonymous_array_int32_t_10, element_flags_anonymous_array_int32_t_10);
                {
                    SBoundSeq array_bound_seq;
                        TypeObjectUtils::add_array_dimension(array_bound_seq, static_cast<SBound>(10));

                    PlainArraySElemDefn array_sdefn = TypeObjectUtils::build_plain_array_s_elem_defn(header_anonymous_array_int32_t_10, array_bound_seq,
                                eprosima::fastcdr::external<TypeIdentifier>(element_identifier_anonymous_array_int32_t_10));
                    if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                            TypeObjectUtils::build_and_register_s_array_type_identifier(array_sdefn, "anonymous_array_int32_t_10", type_ids_value))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                            "anonymous_array_int32_t_10 already registered in TypeObjectRegistry for a different type.");
                    }
                }
            }
            StructMemberFlag member_flags_value = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_value = 0x00000000;
            bool common_value_ec {false};
            CommonStructMember common_value {TypeObjectUtils::build_common_struct_member(member_id_value, member_flags_value, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_value, common_value_ec))};
            if (!common_value_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure value member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_value = "value";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_value;
            ann_custom_DDSEnablerTestType3.reset();
            CompleteMemberDetail detail_value = TypeObjectUtils::build_complete_member_detail(name_value, member_ann_builtin_value, ann_custom_DDSEnablerTestType3);
            CompleteStructMember member_value = TypeObjectUtils::build_complete_struct_member(common_value, detail_value);
            TypeObjectUtils::add_complete_struct_member(member_seq_DDSEnablerTestType3, member_value);
        }
        CompleteStructType struct_type_DDSEnablerTestType3 = TypeObjectUtils::build_complete_struct_type(struct_flags_DDSEnablerTestType3, header_DDSEnablerTestType3, member_seq_DDSEnablerTestType3);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_DDSEnablerTestType3, type_name_DDSEnablerTestType3.to_string(), type_ids_DDSEnablerTestType3))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "DDSEnablerTestType3 already registered in TypeObjectRegistry for a different type.");
        }
    }
}
// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_DDSEnablerTestType4_type_identifier(
        TypeIdentifierPair& type_ids_DDSEnablerTestType4)
{

    ReturnCode_t return_code_DDSEnablerTestType4 {eprosima::fastdds::dds::RETCODE_OK};
    return_code_DDSEnablerTestType4 =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        "DDSEnablerTestType4", type_ids_DDSEnablerTestType4);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_DDSEnablerTestType4)
    {
        StructTypeFlag struct_flags_DDSEnablerTestType4 = TypeObjectUtils::build_struct_type_flag(eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE,
                false, false);
        QualifiedTypeName type_name_DDSEnablerTestType4 = "DDSEnablerTestType4";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_DDSEnablerTestType4;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_DDSEnablerTestType4;
        CompleteTypeDetail detail_DDSEnablerTestType4 = TypeObjectUtils::build_complete_type_detail(type_ann_builtin_DDSEnablerTestType4, ann_custom_DDSEnablerTestType4, type_name_DDSEnablerTestType4.to_string());
        CompleteStructHeader header_DDSEnablerTestType4;
        header_DDSEnablerTestType4 = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail_DDSEnablerTestType4);
        CompleteStructMemberSeq member_seq_DDSEnablerTestType4;
        {
            TypeIdentifierPair type_ids_value;
            ReturnCode_t return_code_value {eprosima::fastdds::dds::RETCODE_OK};
            return_code_value =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "DDSEnablerTestType1", type_ids_value);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_value)
            {
            ::register_DDSEnablerTestType1_type_identifier(type_ids_value);
            }
            StructMemberFlag member_flags_value = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_value = 0x00000000;
            bool common_value_ec {false};
            CommonStructMember common_value {TypeObjectUtils::build_common_struct_member(member_id_value, member_flags_value, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_value, common_value_ec))};
            if (!common_value_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure value member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_value = "value";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_value;
            ann_custom_DDSEnablerTestType4.reset();
            CompleteMemberDetail detail_value = TypeObjectUtils::build_complete_member_detail(name_value, member_ann_builtin_value, ann_custom_DDSEnablerTestType4);
            CompleteStructMember member_value = TypeObjectUtils::build_complete_struct_member(common_value, detail_value);
            TypeObjectUtils::add_complete_struct_member(member_seq_DDSEnablerTestType4, member_value);
        }
        CompleteStructType struct_type_DDSEnablerTestType4 = TypeObjectUtils::build_complete_struct_type(struct_flags_DDSEnablerTestType4, header_DDSEnablerTestType4, member_seq_DDSEnablerTestType4);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_DDSEnablerTestType4, type_name_DDSEnablerTestType4.to_string(), type_ids_DDSEnablerTestType4))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "DDSEnablerTestType4 already registered in TypeObjectRegistry for a different type.");
        }
    }
}

