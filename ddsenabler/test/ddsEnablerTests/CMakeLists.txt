# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set(TEST_NAME DdsEnablerTest)

file(
    GLOB_RECURSE TEST_SOURCES
    DdsEnablerTest.cpp
    "${PROJECT_SOURCE_DIR}/test/resources/types/*.c*"
)

file(
    GLOB_RECURSE TEST_LIBRARY_SOURCES
    "${PROJECT_SOURCE_DIR}/src/cpp/*.c*"
)

list(FILTER TEST_LIBRARY_SOURCES EXCLUDE REGEX "/main.cpp")

set(TEST_LIST
    ddsenabler_creation
    ddsenabler_reload_configuration
    ddsenabler_send_samples_type1
    ddsenabler_send_samples_type2
    ddsenabler_send_samples_type3
    ddsenabler_send_samples_type4
    ddsenabler_send_samples_multiple_types
    ddsenabler_send_samples_repeated_type
)

set(TEST_NEEDED_SOURCES
)

set(TEST_EXTRA_HEADERS
    ${PROJECT_SOURCE_DIR}/test/resources/types/
)

add_blackbox_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_NEEDED_SOURCES}"
    "${TEST_EXTRA_HEADERS}"
    "${TEST_LIBRARY_SOURCES}"
)

