// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file main.cpp
 *
 */

#include <csignal>
#include <stdexcept>
#include <thread>
#include <iostream>

#include "ddsenabler/dds_enabler_runner.hpp"
#include "ddsenabler/DDSEnabler.hpp"
#include "CLIParser.hpp"

// #include <fastdds/dds/domain/DomainParticipantFactory.hpp>
// #include <fastdds/dds/log/Log.hpp>

// #include "Application.hpp"

// using eprosima::fastdds::dds::Log;

// using namespace eprosima::fastdds::examples::hello_world;

// std::function<void(int)> stop_app_handler;
// void signal_handler(
//         int signum)
// {
//     // stop_app_handler(signum);
// }
uint32_t received_types_ = 0;
uint32_t received_data_ = 0;
std::mutex type_received_mutex_;
std::mutex data_received_mutex_;

// Static type callback
static void test_type_callback(
    const char* typeName,
    const char* topicName,
    const char* serializedType)
{
    std::lock_guard<std::mutex> lock(type_received_mutex_);

    received_types_++;
    std::cout << "Type callback received: " << typeName << ", Total types: " <<
        received_types_ << std::endl;
}

// Static data callback
static void test_data_callback(
    const char* typeName,
    const char* topicName,
    const char* json,
    int64_t publishTime)
{
    std::lock_guard<std::mutex> lock(data_received_mutex_);

    received_data_++;
    std::cout << "Data callback received: " << typeName << ", Total data: " <<
        received_data_ << std::endl;
}

int get_received_types()
{
    std::lock_guard<std::mutex> lock(type_received_mutex_);

    return received_types_;
}

int get_received_data()
{
    std::lock_guard<std::mutex> lock(data_received_mutex_);

    return received_data_;
}

int main(
        int argc,
        char** argv)
{
    CLIParser::example_config config = CLIParser::parse_cli_options(argc, argv);


    YAML::Node yml;

    eprosima::ddsenabler::yaml::EnablerConfiguration configuration(yml);

    auto close_handler = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

    auto enabler = std::make_unique<DDSEnabler>(configuration, close_handler);

    // Bind the static callbacks (no captures allowed)
    enabler->set_data_callback(test_data_callback);
    enabler->set_type_callback(test_type_callback);

    // Loop until timeout seconds
    auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(config.timeout_seconds);
    while (std::chrono::steady_clock::now() < end_time)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (get_received_types() >= config.expected_types_ &&
            get_received_data() >= config.expected_data_)
        {
            std::cout << "Received enough data, stopping..." << std::endl;
            return EXIT_SUCCESS;
        }
    }

    std::cout << "Timeout reached, stopping..." << std::endl;
    return EXIT_FAILURE;
}
