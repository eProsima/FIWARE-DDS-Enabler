# Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.20)

project(ddsenabler_test_app VERSION 1 LANGUAGES CXX)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr 2 REQUIRED)
endif()

if(NOT fastdds_FOUND)
    find_package(fastdds 3 REQUIRED)
endif()

if(NOT cpp_utils)
    find_package(cpp_utils REQUIRED)
endif()

if(NOT ddspipe_participants)
    find_package(ddspipe_participants REQUIRED)
endif()

if(NOT ddspipe_yaml)
    find_package(ddspipe_yaml REQUIRED)
endif()

if(NOT ddspipe_core)
    find_package(ddspipe_core REQUIRED)
endif()

if(NOT ddsenabler_participants)
    find_package(ddsenabler_participants REQUIRED)
endif()

if(NOT ddsenabler_yaml)
    find_package(ddsenabler_yaml REQUIRED)
endif()

if(NOT ddsenabler)
    find_package(ddsenabler REQUIRED)
endif()


#Check C++11
include(CheckCXXCompilerFlag)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(NOT SUPPORTS_CXX11)
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

# Set CMAKE_BUILD_TYPE to Debug by default.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

message(STATUS "Configuring ddsenabler test_app...")
file(GLOB TEST_APP_SOURCES_CXX "*.cxx")
file(GLOB TEST_APP_SOURCES_CPP "*.cpp")

add_executable(test_app ${TEST_APP_SOURCES_CXX} ${TEST_APP_SOURCES_CPP})
target_compile_definitions(test_app PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    $<$<BOOL:${SHM_TRANSPORT_DEFAULT}>:SHM_TRANSPORT_BUILTIN> # Enable SHM as built-in transport
)

target_link_libraries(test_app
    fastdds
    fastcdr
    ddsenabler
    cpp_utils
    ddspipe_core
    ddspipe_participants
    ddspipe_yaml
    ddsenabler_participants
    ddsenabler_yaml)

