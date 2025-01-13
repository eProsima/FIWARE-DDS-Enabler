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
 * @file constantsCdrAux.hpp
 * This source file contains some definitions of CDR related functions.
 *
 * This file was generated by the tool fastddsgen.
 */

#ifndef FAST_DDS_GENERATED__CONSTANTSCDRAUX_HPP
#define FAST_DDS_GENERATED__CONSTANTSCDRAUX_HPP

#include "constants.hpp"

constexpr uint32_t ConstsLiteralsStruct_max_cdr_typesize {10656UL};
constexpr uint32_t ConstsLiteralsStruct_max_key_cdr_typesize {0UL};





constexpr uint32_t const_module2_Module2ConstsLiteralsStruct_max_cdr_typesize {140UL};
constexpr uint32_t const_module2_Module2ConstsLiteralsStruct_max_key_cdr_typesize {0UL};

constexpr uint32_t const_module1_ModuleConstsLiteralsStruct_max_cdr_typesize {50UL};
constexpr uint32_t const_module1_ModuleConstsLiteralsStruct_max_key_cdr_typesize {0UL};







namespace eprosima {
namespace fastcdr {

class Cdr;
class CdrSizeCalculator;

eProsima_user_DllExport void serialize_key(
        eprosima::fastcdr::Cdr& scdr,
        const const_module1::ModuleConstsLiteralsStruct& data);

eProsima_user_DllExport void serialize_key(
        eprosima::fastcdr::Cdr& scdr,
        const const_module2::Module2ConstsLiteralsStruct& data);

eProsima_user_DllExport void serialize_key(
        eprosima::fastcdr::Cdr& scdr,
        const ConstsLiteralsStruct& data);


} // namespace fastcdr
} // namespace eprosima

#endif // FAST_DDS_GENERATED__CONSTANTSCDRAUX_HPP

