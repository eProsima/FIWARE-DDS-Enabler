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

###############################################################################
# Set settings for project ddsenabler
###############################################################################

set(MODULE_NAME
    ddsenabler)

set(MODULE_NAME_LARGE
    "eProsima DDS Enabler Library")

set(MODULE_SUMMARY
    "C++ application to run a DDS Enabler from a YAML configuration file.")

set(MODULE_FIND_PACKAGES
    yaml-cpp
    fastcdr
    fastdds
    cpp_utils
    ddspipe_core
    ddspipe_participants
    ddspipe_yaml
    ddsenabler_participants
    ddsenabler_yaml)

set(MODULE_DEPENDENCIES
    ${MODULE_FIND_PACKAGES})

set(MODULE_THIRDPARTY_HEADERONLY
    nlohmann-json
    )

set(MODULE_THIRDPARTY_PATH
    "../thirdparty")

set(MODULE_LICENSE_FILE_PATH
    "../LICENSE")

set(MODULE_VERSION_FILE_PATH
    "../VERSION")

set(MODULE_CPP_VERSION
    C++17)
