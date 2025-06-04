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

#include <iostream>
#include <string>

#include "services.hpp"
#include "actions.hpp"

// Define static pointer
tester::Tester* tester::Tester::current_test_instance_ = nullptr;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <test_name>\n";
        return 1;
    }

    std::string test_name = argv[1];

    try
    {
        if (test_name == "service_discovery")
        {
            std::cout << "Running test: service_discovery\n";
            return service_discovery();
        }
        else if (test_name == "service_server")
        {
            std::cout << "Running test: service_server\n";
            return service_server();
        }
        else if (test_name == "service_client")
        {
            std::cout << "Running test: service_client\n";
            return service_client();
        }
        else if (test_name == "action_discovery")
        {
            std::cout << "Running test: action_discovery\n";
            return action_discovery();
        }
        else if (test_name == "action_client")
        {
            std::cout << "Running test: action_client\n";
            return action_client();
        }
        else if (test_name == "action_client_cancel")
        {
            std::cout << "Running test: action_client_cancel\n";
            return action_client_cancel();
        }
        else if (test_name == "action_server")
        {
            std::cout << "Running test: action_server\n";
            return action_server();
        }
        else
        {
            std::cerr << "Unknown test name: " << test_name << "\n";
            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception during test: " << e.what() << "\n";
        return -1;
    }
}
