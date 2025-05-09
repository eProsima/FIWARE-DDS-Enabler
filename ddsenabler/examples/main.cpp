// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

uint32_t received_types_ = 0;
uint32_t received_data_ = 0;
std::mutex type_received_mutex_;
std::mutex data_received_mutex_;
bool stop_app_ = false;

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

void signal_handler(
        int signum)
{
    std::cout << "Signal " << CLIParser::parse_signal(signum) << " received, stopping..." << std::endl;
    stop_app_ = true;
}

int main(
        int argc,
        char** argv)
{
    CLIParser::example_config config = CLIParser::parse_cli_options(argc, argv);

    std::unique_ptr<DDSEnabler> enabler;
    create_dds_enabler(config.config_file_path_.c_str(), test_data_callback, test_type_callback, nullptr, enabler);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
#ifndef _WIN32
    signal(SIGQUIT, signal_handler);
    signal(SIGHUP, signal_handler);
#endif // _WIN32

    // Loop until timeout seconds
    auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(config.timeout_seconds);
    while (std::chrono::steady_clock::now() < end_time)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (stop_app_)
        {
            return EXIT_SUCCESS;
        }
        else if (get_received_types() >= config.expected_types_ &&
                get_received_data() >= config.expected_data_)
        {
            std::cout << "Received enough data, stopping..." << std::endl;
            return EXIT_SUCCESS;
        }
    }

    std::cout << "Timeout reached, stopping..." << std::endl;
    return EXIT_FAILURE;
}
