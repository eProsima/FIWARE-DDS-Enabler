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

###################
# Parse Yaml Test #
###################

set(TEST_NAME DdsEnablerYamlTest)

set(TEST_SOURCES
        ${PROJECT_SOURCE_DIR}/test/DdsEnablerYamlTest.cpp
    )

set(TEST_LIST
        get_ddsenabler_correct_configuration_yaml
        get_ddsenabler_incorrect_n_threads_configuration_yaml
        get_ddsenabler_default_values_configuration_yaml
        get_ddsenabler_incorrect_path_configuration_yaml
        get_ddsenabler_correct_configuration_json
        get_ddsenabler_incorrect_n_threads_configuration_json
        get_ddsenabler_default_values_configuration_json
        get_ddsenabler_incorrect_path_configuration_json
        get_ddsenabler_full_configuration_json
    )

set(TEST_EXTRA_LIBRARIES
        yaml-cpp
        ddspipe_core
        ddspipe_yaml
        ddsenabler_yaml
    )

# Specify the JSON resource file
set(TEST_NEEDED_SOURCES 
        ../test/resources/correct_config.json
        ../test/resources/incorrect_threads_config.json
        ../test/resources/default_config.json
        ../test/resources/full_config.json
    )

add_unittest_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_EXTRA_LIBRARIES}"
    "${TEST_NEEDED_SOURCES}")
