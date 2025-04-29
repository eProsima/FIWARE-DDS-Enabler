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

#include <csignal>
#include <cstdlib>
#include <iostream>

#include <fastdds/dds/log/Log.hpp>

#pragma once

class CLIParser
{
public:

    CLIParser() = delete;

    struct example_config
    {
        uint32_t expected_types_ = 0;
        uint32_t expected_data_ = 0;
        uint32_t timeout_seconds = 30;
    };

    /**
     * @brief Print usage help message and exit with the given return code
     *
     * @param return_code return code to exit with
     *
     * @warning This method finishes the execution of the program with the input return code
     */
    static void print_help(
            uint8_t return_code)
    {
        std::cout << "Usage: ddsenabler_example <expected_types> <expected_data> <timeout>"          << std::endl;
        std::cout << ""                                                                              << std::endl;
        std::cout << "expected_types: number of types to be expected"                                << std::endl;
        std::cout << "expected_data: number of data to be expected"                                  << std::endl;
        std::cout << "timeout: time to wait before stopping the program if the data is not received" << std::endl;
        std::exit(return_code);
    }

    /**
     * @brief Parse the command line options and return the configuration_config object
     *
     * @param argc number of arguments
     * @param argv array of arguments
     * @return configuration_config object with the parsed options
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    static example_config parse_cli_options(
            int argc,
            char* argv[])
    {
        example_config config;

        if (argc < 4)
        {
            std::cerr << "Missing entity argument" << std::endl;
            print_help(EXIT_FAILURE);
        }

        try
        {
            config.expected_types_ = std::stoi(argv[1]);
            config.expected_data_ = std::stoi(argv[2]);
            config.timeout_seconds = std::stoi(argv[3]);
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER, "Error parsing command line arguments: " << e.what());
            print_help(EXIT_FAILURE);
        }

        return config;
    }

    /**
     * @brief Parse the signal number into the signal name
     *
     * @param signum signal number
     * @return std::string signal name
     */
    static std::string parse_signal(
            const int& signum)
    {
        switch (signum)
        {
            case SIGINT:
                return "SIGINT";
            case SIGTERM:
                return "SIGTERM";
#ifndef _WIN32
            case SIGQUIT:
                return "SIGQUIT";
            case SIGHUP:
                return "SIGHUP";
#endif // _WIN32
            default:
                return "UNKNOWN SIGNAL";
        }
    }

};

