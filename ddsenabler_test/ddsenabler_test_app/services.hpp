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

#pragma once

#include "tester.hpp"

int service_discovery()
{
    tester::Tester test = tester::Tester();
    tester::Tester::current_test_instance_ = &test;

    auto enabler = test.create_ddsenabler();
    if (enabler == nullptr)
    {
        std::cerr << "Failed to create DDSEnabler instance." << std::endl;
        return -1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));

    if (test.wait_for_service_notification(1) == 0)
    {
        std::cerr << "Service not available (REQUIRED LAUNCH OF ROS2 ADD TWO INTS SERVER)..." << std::endl;
        return -1;
    }

    std::cout << "Service available, test successful" << std::endl;
    return 0;
}

int service_server()
{
    tester::Tester test = tester::Tester();
    tester::Tester::current_test_instance_ = &test;

    auto enabler = test.create_ddsenabler();
    if (enabler == nullptr)
    {
        std::cerr << "Failed to create DDSEnabler instance." << std::endl;
        return -1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::string service_name = "add_two_ints";

    if (enabler->revoke_service(service_name))
    {
        std::cerr << "Service revoked successfully before announcement" << std::endl;
        return -1;
    }

    if (!enabler->announce_service(service_name))
    {
        std::cerr << "Failed to announce service" << std::endl;
        return -1;
    }

    if (!test.wait_for_service_query(1))
    {
        std::cerr << "No service query received." << std::endl;
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (enabler->announce_service(service_name))
    {
        std::cerr << "Service announced again, should have failed." << std::endl;
        return -1;
    }

    std::string json = "{\"sum\": 3}";
    uint64_t request_id = 0;
    while(request_id < 3)
    {
        if (!test.wait_for_service_request(service_name, request_id, 100))
        {
            std::cerr << "Timeout waiting for request callback" << std::endl;
            return -1;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if(request_id > 0)
        {
            std::cout << "Sending reply for request id: " << request_id << std::endl;
            if (!enabler->send_service_reply(service_name, json, request_id))
            {
                std::cerr << "Failed to send service reply for request id: " << request_id << std::endl;
                return -1;
            }
        }

    }

    if (!enabler->revoke_service(service_name))
    {
        std::cerr << "Failed to revoke service" << std::endl;
        return -1;
    }
    if (enabler->revoke_service(service_name))
    {
        std::cerr << "Service revoked again, should have failed." << std::endl;
        return -1;
    }

    // TODO check clients know the server is stopped
    return 0;
}

int service_client()
{
    tester::Tester test = tester::Tester();
    tester::Tester::current_test_instance_ = &test;

    auto enabler = test.create_ddsenabler();
    if (enabler == nullptr)
    {
        std::cerr << "Failed to create DDSEnabler instance." << std::endl;
        return -1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));

    if (test.wait_for_service_notification(1) == 0)
    {
        std::cerr << "Service not available (REQUIRED LAUNCH OF ROS2 ADD TWO INTS SERVER)..." << std::endl;
        return -1;
    }

    std::string json = "{\"a\": 1, \"b\": 2}";
    std::string service_name = "add_two_ints";
    uint64_t request_id = 0;

    int sent_requests = 0;
    while(sent_requests < 3)
    {
        if(!enabler->send_service_request(service_name, json, request_id))
        {
            std::cerr << "Service not available (REQUIRED MANUAL LAUNCH OF ROS2 ADD TWO INTS SERVER)..." << std::endl;
            return -1;
        }
        sent_requests++;

        std::this_thread::sleep_for(std::chrono::seconds(3));

        uint64_t reply_id = 0;
        if (!test.wait_for_service_reply_notification(reply_id) || reply_id != request_id)
        {
            std::cerr << "Failed to receive reply for request id: " << request_id << std::endl;
            return -1;
        }
        request_id = 0;
    }
    return 0;
}

