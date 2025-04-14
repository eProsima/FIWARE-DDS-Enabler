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

cmake_minimum_required(VERSION 3.20)
project(ddsenabler_example LANGUAGES CXX)

# Create the executable from main.cpp
add_executable(ddsenabler_example main.cpp)

# Link the ddsenabler library
target_link_libraries(ddsenabler_example PRIVATE ddsenabler)

# If the ddsenabler package exports include directories, this is automatically handled.
# If not, you can manually add:
# target_include_directories(ddsenabler_example PRIVATE ${DDSENABLER_INCLUDE_DIRS})

# Install rule (optional)
install(TARGETS ddsenabler_example
    RUNTIME DESTINATION bin
)

# Copy the JSON files over to the build directory
file(GLOB_RECURSE JSON_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.json)
# for each json file detected
foreach(JSON_FILE_COMPLETE_PATH ${JSON_FILES})
    # obtain the file name
    get_filename_component(JSON_FILE ${JSON_FILE_COMPLETE_PATH} NAME_WE)
    # copy the file from src to build folders
    configure_file(
        ${JSON_FILE_COMPLETE_PATH}                   # from full src path
        ${CMAKE_CURRENT_BINARY_DIR}/${JSON_FILE}.json # to relative build path
        COPYONLY)
    install(FILES ${JSON_FILE_COMPLETE_PATH}
        DESTINATION ${DATA_INSTALL_DIR}/ddsenabler/examples/ddsenabler/${BIN_INSTALL_DIR})
endforeach()
