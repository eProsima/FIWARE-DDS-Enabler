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
# CMake build rules for DDS Enabler Submodule
###############################################################################
cmake_minimum_required(VERSION 3.5)
add_definitions(-fPIC)

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

# Call explictly project
project(
    ${MODULE_NAME}
    VERSION
        ${MODULE_VERSION}
    DESCRIPTION
        ${MODULE_DESCRIPTION}
    LANGUAGES
        CXX
)

###############################################################################
# C++ Project
###############################################################################
# Configure CPP project for dependencies and required flags:
# - Set CMake Build Type
# - Set C++ version
# - Set shared libraries by default
# - Find external packages and thirdparties
# - Activate Code coverage if flag CODE_COVERAGE
# - Activate Address sanitizer build if flag ASAN_BUILD
# - Activate Thread sanitizer build if flag TSAN_BUILD
# - Configure log depending on LOG_INFO flag and CMake type
configure_project_cpp()

# Compile C++ library
compile_library(
    "${PROJECT_SOURCE_DIR}/src/cpp" # Source directory
    "${PROJECT_SOURCE_DIR}/include" # Include directory
)

###############################################################################
# Test
###############################################################################
# Compile tests if CMake options requires it

if(BUILD_TESTING)
    set(BUILD_LIBRARY_TESTS ON)
endif()

compile_test_library(
    "${PROJECT_SOURCE_DIR}/test/" # Test directory
)

###############################################################################
# Examples
###############################################################################
option(COMPILE_EXAMPLES "Build examples" OFF)

if(COMPILE_EXAMPLES)
    add_subdirectory(examples)
endif()

###############################################################################
# Packaging
###############################################################################
# Install package
eprosima_packaging()

###############################################################################
# Documentation
###############################################################################
# Add an option to toggle the generation of the API documentation.
option(DDSENABLER_DOCS "Use doxygen to create product documentation" OFF)

if(DDSENABLER_DOCS)
    find_package(Doxygen)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "doxygen is needed to build the documentation. Please install it correctly")
    endif()
    if(UNIX)
        find_program(DOXYFILE_MAKE make)
        if(DOXYFILE_MAKE)
            message(STATUS "Found Make: ${DOXYFILE_MAKE}")
        else()
            message(FATAL_ERROR "make is needed to build the documentation. Please install it correctly")
        endif()
    elseif(WIN32)
        set(DOXYFILE_MAKE make.bat)
    endif()

    # Target to create documentation directories
    add_custom_target(docsdirs
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/docs
        COMMENT "Creating documentation directory" VERBATIM)

    ### Doxygen ########################3
    # Configure the template doxyfile for or specific project
    configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in
        ${PROJECT_BINARY_DIR}/Doxyfile
        @ONLY IMMEDIATE)
    # Add custom target to run doxygen whenever the project is build
    add_custom_target(doxygen
        COMMAND "${DOXYGEN_EXECUTABLE}" "${PROJECT_BINARY_DIR}/Doxyfile"
        SOURCES "${PROJECT_BINARY_DIR}/Doxyfile"
        COMMENT "Generating API documentation with doxygen" VERBATIM)

    add_dependencies(doxygen docsdirs)

    add_custom_target(docs ALL
        COMMENT "Generated project documentation" VERBATIM)

    add_dependencies(docs doxygen)

    install(DIRECTORY ${PROJECT_BINARY_DIR}/docs/api_reference
        DESTINATION share/docs
        COMPONENT documentation
        )
endif()

