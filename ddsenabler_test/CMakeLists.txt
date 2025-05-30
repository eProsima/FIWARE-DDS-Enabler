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

###############################################################################
# CMake build rules for DDS Enabler Submodule
###############################################################################
cmake_minimum_required(VERSION 3.5)

# Done this to set machine architecture and be able to call cmake_utils
enable_language(CXX)

###############################################################################
# Find package cmake_utils
###############################################################################
# Package cmake_utils is required to get every cmake macro needed
find_package(cmake_utils REQUIRED)

###############################################################################
# Project
###############################################################################
# Configure project by info set in project_settings.cmake
# - Load project_settings variables
# - Read version
# - Set installation paths
configure_project()

# Call explicitly project
project(
    ${MODULE_NAME}
    VERSION
        ${MODULE_VERSION}
    DESCRIPTION
        ${MODULE_DESCRIPTION}
)

###############################################################################
# Test
###############################################################################
# Compile tests if CMake options requires it
option(BUILD_COMPOSE_TESTS "Compile compose tests" OFF)

if(BUILD_COMPOSE_TESTS)
    message(STATUS "Compiling Compose Tests of ${PROJECT_NAME}")
    configure_test_flags()
    test_requirements()
    add_subdirectory(compose)
endif()
# NOTE: this is done explicitly and not by a CMake macro (compile_test) as other packages
# because it does not feel like reusable, so adding it to cmake_utils does not seems appropriate.

###############################################################################
# Packaging
###############################################################################
# NOTE: No packaging needed for this package as it is test only. However it removes the colcon warning.
# Install package
eprosima_packaging()
