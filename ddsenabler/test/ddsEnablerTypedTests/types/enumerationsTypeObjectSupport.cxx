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
 * @file enumerationsTypeObjectSupport.cxx
 * Source file containing the implementation to register the TypeObject representation of the described types in the IDL file
 *
 * This file was generated by the tool fastddsgen.
 */

#include "enumerationsTypeObjectSupport.hpp"

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

#include "enumerations.hpp"

#include "helpers/basic_inner_types.hpp"

using namespace eprosima::fastdds::dds::xtypes;

namespace Test {
void register_InnerEnumHelper_type_identifier(
        TypeIdentifierPair& type_ids_InnerEnumHelper)
{
    ReturnCode_t return_code_InnerEnumHelper {eprosima::fastdds::dds::RETCODE_OK};
    return_code_InnerEnumHelper =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        "Test::InnerEnumHelper", type_ids_InnerEnumHelper);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_InnerEnumHelper)
    {
        EnumTypeFlag enum_flags_InnerEnumHelper = 0;
        BitBound bit_bound_InnerEnumHelper = 32;
        CommonEnumeratedHeader common_InnerEnumHelper = TypeObjectUtils::build_common_enumerated_header(bit_bound_InnerEnumHelper);
        QualifiedTypeName type_name_InnerEnumHelper = "Test::InnerEnumHelper";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_InnerEnumHelper;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_InnerEnumHelper;
        CompleteTypeDetail detail_InnerEnumHelper = TypeObjectUtils::build_complete_type_detail(type_ann_builtin_InnerEnumHelper, ann_custom_InnerEnumHelper, type_name_InnerEnumHelper.to_string());
        CompleteEnumeratedHeader header_InnerEnumHelper = TypeObjectUtils::build_complete_enumerated_header(common_InnerEnumHelper, detail_InnerEnumHelper);
        CompleteEnumeratedLiteralSeq literal_seq_InnerEnumHelper;
        {
            EnumeratedLiteralFlag flags_ENUM_VALUE_1 = TypeObjectUtils::build_enumerated_literal_flag(false);
            CommonEnumeratedLiteral common_ENUM_VALUE_1 = TypeObjectUtils::build_common_enumerated_literal(0, flags_ENUM_VALUE_1);
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_ENUM_VALUE_1;
            ann_custom_InnerEnumHelper.reset();
            MemberName name_ENUM_VALUE_1 = "ENUM_VALUE_1";
            CompleteMemberDetail detail_ENUM_VALUE_1 = TypeObjectUtils::build_complete_member_detail(name_ENUM_VALUE_1, member_ann_builtin_ENUM_VALUE_1, ann_custom_InnerEnumHelper);
            CompleteEnumeratedLiteral literal_ENUM_VALUE_1 = TypeObjectUtils::build_complete_enumerated_literal(common_ENUM_VALUE_1, detail_ENUM_VALUE_1);
            TypeObjectUtils::add_complete_enumerated_literal(literal_seq_InnerEnumHelper, literal_ENUM_VALUE_1);
        }
        {
            EnumeratedLiteralFlag flags_ENUM_VALUE_2 = TypeObjectUtils::build_enumerated_literal_flag(false);
            CommonEnumeratedLiteral common_ENUM_VALUE_2 = TypeObjectUtils::build_common_enumerated_literal(1, flags_ENUM_VALUE_2);
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_ENUM_VALUE_2;
            ann_custom_InnerEnumHelper.reset();
            MemberName name_ENUM_VALUE_2 = "ENUM_VALUE_2";
            CompleteMemberDetail detail_ENUM_VALUE_2 = TypeObjectUtils::build_complete_member_detail(name_ENUM_VALUE_2, member_ann_builtin_ENUM_VALUE_2, ann_custom_InnerEnumHelper);
            CompleteEnumeratedLiteral literal_ENUM_VALUE_2 = TypeObjectUtils::build_complete_enumerated_literal(common_ENUM_VALUE_2, detail_ENUM_VALUE_2);
            TypeObjectUtils::add_complete_enumerated_literal(literal_seq_InnerEnumHelper, literal_ENUM_VALUE_2);
        }
        {
            EnumeratedLiteralFlag flags_ENUM_VALUE_3 = TypeObjectUtils::build_enumerated_literal_flag(false);
            CommonEnumeratedLiteral common_ENUM_VALUE_3 = TypeObjectUtils::build_common_enumerated_literal(2, flags_ENUM_VALUE_3);
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_ENUM_VALUE_3;
            ann_custom_InnerEnumHelper.reset();
            MemberName name_ENUM_VALUE_3 = "ENUM_VALUE_3";
            CompleteMemberDetail detail_ENUM_VALUE_3 = TypeObjectUtils::build_complete_member_detail(name_ENUM_VALUE_3, member_ann_builtin_ENUM_VALUE_3, ann_custom_InnerEnumHelper);
            CompleteEnumeratedLiteral literal_ENUM_VALUE_3 = TypeObjectUtils::build_complete_enumerated_literal(common_ENUM_VALUE_3, detail_ENUM_VALUE_3);
            TypeObjectUtils::add_complete_enumerated_literal(literal_seq_InnerEnumHelper, literal_ENUM_VALUE_3);
        }
        CompleteEnumeratedType enumerated_type_InnerEnumHelper = TypeObjectUtils::build_complete_enumerated_type(enum_flags_InnerEnumHelper, header_InnerEnumHelper,
                literal_seq_InnerEnumHelper);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_enumerated_type_object(enumerated_type_InnerEnumHelper, type_name_InnerEnumHelper.to_string(), type_ids_InnerEnumHelper))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                "Test::InnerEnumHelper already registered in TypeObjectRegistry for a different type.");
        }
    }
}
} // namespace Test
// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_EnumStructure_type_identifier(
        TypeIdentifierPair& type_ids_EnumStructure)
{

    ReturnCode_t return_code_EnumStructure {eprosima::fastdds::dds::RETCODE_OK};
    return_code_EnumStructure =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        "EnumStructure", type_ids_EnumStructure);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_EnumStructure)
    {
        StructTypeFlag struct_flags_EnumStructure = TypeObjectUtils::build_struct_type_flag(eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE,
                false, false);
        QualifiedTypeName type_name_EnumStructure = "EnumStructure";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_EnumStructure;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_EnumStructure;
        CompleteTypeDetail detail_EnumStructure = TypeObjectUtils::build_complete_type_detail(type_ann_builtin_EnumStructure, ann_custom_EnumStructure, type_name_EnumStructure.to_string());
        CompleteStructHeader header_EnumStructure;
        header_EnumStructure = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail_EnumStructure);
        CompleteStructMemberSeq member_seq_EnumStructure;
        {
            TypeIdentifierPair type_ids_var_InnerEnumHelper;
            ReturnCode_t return_code_var_InnerEnumHelper {eprosima::fastdds::dds::RETCODE_OK};
            return_code_var_InnerEnumHelper =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "InnerEnumHelper", type_ids_var_InnerEnumHelper);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_var_InnerEnumHelper)
            {
            ::register_InnerEnumHelper_type_identifier(type_ids_var_InnerEnumHelper);
            }
            StructMemberFlag member_flags_var_InnerEnumHelper = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_var_InnerEnumHelper = 0x00000000;
            bool common_var_InnerEnumHelper_ec {false};
            CommonStructMember common_var_InnerEnumHelper {TypeObjectUtils::build_common_struct_member(member_id_var_InnerEnumHelper, member_flags_var_InnerEnumHelper, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_var_InnerEnumHelper, common_var_InnerEnumHelper_ec))};
            if (!common_var_InnerEnumHelper_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure var_InnerEnumHelper member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_var_InnerEnumHelper = "var_InnerEnumHelper";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_var_InnerEnumHelper;
            ann_custom_EnumStructure.reset();
            CompleteMemberDetail detail_var_InnerEnumHelper = TypeObjectUtils::build_complete_member_detail(name_var_InnerEnumHelper, member_ann_builtin_var_InnerEnumHelper, ann_custom_EnumStructure);
            CompleteStructMember member_var_InnerEnumHelper = TypeObjectUtils::build_complete_struct_member(common_var_InnerEnumHelper, detail_var_InnerEnumHelper);
            TypeObjectUtils::add_complete_struct_member(member_seq_EnumStructure, member_var_InnerEnumHelper);
        }
        {
            TypeIdentifierPair type_ids_var_scoped_InnerEnumHelper;
            ReturnCode_t return_code_var_scoped_InnerEnumHelper {eprosima::fastdds::dds::RETCODE_OK};
            return_code_var_scoped_InnerEnumHelper =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "Test::InnerEnumHelper", type_ids_var_scoped_InnerEnumHelper);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_var_scoped_InnerEnumHelper)
            {
                Test::register_InnerEnumHelper_type_identifier(type_ids_var_scoped_InnerEnumHelper);
            }
            StructMemberFlag member_flags_var_scoped_InnerEnumHelper = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_var_scoped_InnerEnumHelper = 0x00000001;
            bool common_var_scoped_InnerEnumHelper_ec {false};
            CommonStructMember common_var_scoped_InnerEnumHelper {TypeObjectUtils::build_common_struct_member(member_id_var_scoped_InnerEnumHelper, member_flags_var_scoped_InnerEnumHelper, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_var_scoped_InnerEnumHelper, common_var_scoped_InnerEnumHelper_ec))};
            if (!common_var_scoped_InnerEnumHelper_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure var_scoped_InnerEnumHelper member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_var_scoped_InnerEnumHelper = "var_scoped_InnerEnumHelper";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_var_scoped_InnerEnumHelper;
            ann_custom_EnumStructure.reset();
            CompleteMemberDetail detail_var_scoped_InnerEnumHelper = TypeObjectUtils::build_complete_member_detail(name_var_scoped_InnerEnumHelper, member_ann_builtin_var_scoped_InnerEnumHelper, ann_custom_EnumStructure);
            CompleteStructMember member_var_scoped_InnerEnumHelper = TypeObjectUtils::build_complete_struct_member(common_var_scoped_InnerEnumHelper, detail_var_scoped_InnerEnumHelper);
            TypeObjectUtils::add_complete_struct_member(member_seq_EnumStructure, member_var_scoped_InnerEnumHelper);
        }
        CompleteStructType struct_type_EnumStructure = TypeObjectUtils::build_complete_struct_type(struct_flags_EnumStructure, header_EnumStructure, member_seq_EnumStructure);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_EnumStructure, type_name_EnumStructure.to_string(), type_ids_EnumStructure))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "EnumStructure already registered in TypeObjectRegistry for a different type.");
        }
    }
}
// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_BitMaskStructure_type_identifier(
        TypeIdentifierPair& type_ids_BitMaskStructure)
{

    ReturnCode_t return_code_BitMaskStructure {eprosima::fastdds::dds::RETCODE_OK};
    return_code_BitMaskStructure =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        "BitMaskStructure", type_ids_BitMaskStructure);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_BitMaskStructure)
    {
        StructTypeFlag struct_flags_BitMaskStructure = TypeObjectUtils::build_struct_type_flag(eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE,
                false, false);
        QualifiedTypeName type_name_BitMaskStructure = "BitMaskStructure";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_BitMaskStructure;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_BitMaskStructure;
        CompleteTypeDetail detail_BitMaskStructure = TypeObjectUtils::build_complete_type_detail(type_ann_builtin_BitMaskStructure, ann_custom_BitMaskStructure, type_name_BitMaskStructure.to_string());
        CompleteStructHeader header_BitMaskStructure;
        header_BitMaskStructure = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail_BitMaskStructure);
        CompleteStructMemberSeq member_seq_BitMaskStructure;
        {
            TypeIdentifierPair type_ids_var_InnerBitMaskHelper;
            ReturnCode_t return_code_var_InnerBitMaskHelper {eprosima::fastdds::dds::RETCODE_OK};
            return_code_var_InnerBitMaskHelper =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "InnerBitMaskHelper", type_ids_var_InnerBitMaskHelper);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_var_InnerBitMaskHelper)
            {
            ::register_InnerBitMaskHelper_type_identifier(type_ids_var_InnerBitMaskHelper);
            }
            StructMemberFlag member_flags_var_InnerBitMaskHelper = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_var_InnerBitMaskHelper = 0x00000000;
            bool common_var_InnerBitMaskHelper_ec {false};
            CommonStructMember common_var_InnerBitMaskHelper {TypeObjectUtils::build_common_struct_member(member_id_var_InnerBitMaskHelper, member_flags_var_InnerBitMaskHelper, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_var_InnerBitMaskHelper, common_var_InnerBitMaskHelper_ec))};
            if (!common_var_InnerBitMaskHelper_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure var_InnerBitMaskHelper member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_var_InnerBitMaskHelper = "var_InnerBitMaskHelper";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_var_InnerBitMaskHelper;
            ann_custom_BitMaskStructure.reset();
            CompleteMemberDetail detail_var_InnerBitMaskHelper = TypeObjectUtils::build_complete_member_detail(name_var_InnerBitMaskHelper, member_ann_builtin_var_InnerBitMaskHelper, ann_custom_BitMaskStructure);
            CompleteStructMember member_var_InnerBitMaskHelper = TypeObjectUtils::build_complete_struct_member(common_var_InnerBitMaskHelper, detail_var_InnerBitMaskHelper);
            TypeObjectUtils::add_complete_struct_member(member_seq_BitMaskStructure, member_var_InnerBitMaskHelper);
        }
        CompleteStructType struct_type_BitMaskStructure = TypeObjectUtils::build_complete_struct_type(struct_flags_BitMaskStructure, header_BitMaskStructure, member_seq_BitMaskStructure);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_BitMaskStructure, type_name_BitMaskStructure.to_string(), type_ids_BitMaskStructure))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "BitMaskStructure already registered in TypeObjectRegistry for a different type.");
        }
    }
}
// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_BoundedBitMaskStructure_type_identifier(
        TypeIdentifierPair& type_ids_BoundedBitMaskStructure)
{

    ReturnCode_t return_code_BoundedBitMaskStructure {eprosima::fastdds::dds::RETCODE_OK};
    return_code_BoundedBitMaskStructure =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        "BoundedBitMaskStructure", type_ids_BoundedBitMaskStructure);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_BoundedBitMaskStructure)
    {
        StructTypeFlag struct_flags_BoundedBitMaskStructure = TypeObjectUtils::build_struct_type_flag(eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE,
                false, false);
        QualifiedTypeName type_name_BoundedBitMaskStructure = "BoundedBitMaskStructure";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_BoundedBitMaskStructure;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_BoundedBitMaskStructure;
        CompleteTypeDetail detail_BoundedBitMaskStructure = TypeObjectUtils::build_complete_type_detail(type_ann_builtin_BoundedBitMaskStructure, ann_custom_BoundedBitMaskStructure, type_name_BoundedBitMaskStructure.to_string());
        CompleteStructHeader header_BoundedBitMaskStructure;
        header_BoundedBitMaskStructure = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail_BoundedBitMaskStructure);
        CompleteStructMemberSeq member_seq_BoundedBitMaskStructure;
        {
            TypeIdentifierPair type_ids_var_InnerBoundedBitMaskHelper;
            ReturnCode_t return_code_var_InnerBoundedBitMaskHelper {eprosima::fastdds::dds::RETCODE_OK};
            return_code_var_InnerBoundedBitMaskHelper =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "InnerBoundedBitMaskHelper", type_ids_var_InnerBoundedBitMaskHelper);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_var_InnerBoundedBitMaskHelper)
            {
            ::register_InnerBoundedBitMaskHelper_type_identifier(type_ids_var_InnerBoundedBitMaskHelper);
            }
            StructMemberFlag member_flags_var_InnerBoundedBitMaskHelper = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_var_InnerBoundedBitMaskHelper = 0x00000000;
            bool common_var_InnerBoundedBitMaskHelper_ec {false};
            CommonStructMember common_var_InnerBoundedBitMaskHelper {TypeObjectUtils::build_common_struct_member(member_id_var_InnerBoundedBitMaskHelper, member_flags_var_InnerBoundedBitMaskHelper, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_var_InnerBoundedBitMaskHelper, common_var_InnerBoundedBitMaskHelper_ec))};
            if (!common_var_InnerBoundedBitMaskHelper_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure var_InnerBoundedBitMaskHelper member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_var_InnerBoundedBitMaskHelper = "var_InnerBoundedBitMaskHelper";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_var_InnerBoundedBitMaskHelper;
            ann_custom_BoundedBitMaskStructure.reset();
            CompleteMemberDetail detail_var_InnerBoundedBitMaskHelper = TypeObjectUtils::build_complete_member_detail(name_var_InnerBoundedBitMaskHelper, member_ann_builtin_var_InnerBoundedBitMaskHelper, ann_custom_BoundedBitMaskStructure);
            CompleteStructMember member_var_InnerBoundedBitMaskHelper = TypeObjectUtils::build_complete_struct_member(common_var_InnerBoundedBitMaskHelper, detail_var_InnerBoundedBitMaskHelper);
            TypeObjectUtils::add_complete_struct_member(member_seq_BoundedBitMaskStructure, member_var_InnerBoundedBitMaskHelper);
        }
        CompleteStructType struct_type_BoundedBitMaskStructure = TypeObjectUtils::build_complete_struct_type(struct_flags_BoundedBitMaskStructure, header_BoundedBitMaskStructure, member_seq_BoundedBitMaskStructure);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_BoundedBitMaskStructure, type_name_BoundedBitMaskStructure.to_string(), type_ids_BoundedBitMaskStructure))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "BoundedBitMaskStructure already registered in TypeObjectRegistry for a different type.");
        }
    }
}

