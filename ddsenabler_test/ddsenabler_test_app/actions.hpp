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

int action_discovery()
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

    if (test.wait_for_action_notification(1) == 0)
    {
        std::cout << "Action not available (REQUIRED MANUAL LAUNCH OF ROS2 FIBONACCI ACTION SERVER)..." << std::endl;
        return -1;
    }

    std::cout << "Action available" << std::endl;
    return 0;
}

int action_client()
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

    std::string json = "{\"order\": 5}";
    std::string action_name = "fibonacci/_action/";
    UUID action_id;

    if (test.wait_for_action_notification(1) == 0)
    {
        std::cout << "Action not available (REQUIRED MANUAL LAUNCH OF ROS2 FIBONACCI ACTION SERVER)..." << std::endl;
        return -1;
    }
    std::cout << "Action available" << std::endl;

    if (enabler->cancel_action_goal(action_name, RpcUtils::generate_UUID(), 0))
    {
        std::cerr << "Cancel action goal should have failed, action id does not exist" << std::endl;
        return -1;
    }

    int sent_requests = 0;
    while(sent_requests < 3)
    {
        if (!enabler->send_action_goal(action_name, json, action_id))
        {
            std::cerr << "Failed to send action goal" << std::endl;
            return -1;
        }

        eprosima::ddsenabler::participants::STATUS_CODE status;
        if (!test.wait_for_status_update(action_id, status, eprosima::ddsenabler::participants::STATUS_CODE::STATUS_ACCEPTED))
        {
            std::cerr << "Failed to receive action status update" << std::endl;
            return -1;
        }

        UUID received_action_id;
        int received_action_feedbacks = 0;
        do
        {
            if (!test.wait_for_feedback(received_action_feedbacks, received_action_id) || !(received_action_id == action_id))
            {
                std::cerr << "Failed to receive action feedback" << std::endl;
                return -1;
            }
            std::cout << "Waiting for all action feedbacks" << std::endl;
            received_action_feedbacks++;
        } while(received_action_feedbacks < 3);

        if (!test.wait_for_status_update(action_id, status, eprosima::ddsenabler::participants::STATUS_CODE::STATUS_SUCCEEDED))
        {
            std::cerr << "Failed to receive action status update for success" << std::endl;
            return -1;
        }

        if (!test.wait_for_result(action_id) || !(received_action_id == action_id))
        {
            std::cerr << "Failed to receive action result" << std::endl;
            return -1;
        }

        action_id = UUID();
        sent_requests++;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}

int action_client_cancel()
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

    std::string json = "{\"order\": 20}";
    std::string action_name = "fibonacci/_action/";
    UUID action_id;

    if (test.wait_for_action_notification(1) == 0)
    {
        std::cout << "Action not available (REQUIRED MANUAL LAUNCH OF ROS2 FIBONACCI ACTION SERVER)..." << std::endl;
        return -1;
    }
    std::cout << "Action available" << std::endl;

    if (!enabler->send_action_goal(action_name, json, action_id))
    {
        std::cerr << "Failed to send action goal" << std::endl;
        return -1;
    }

    eprosima::ddsenabler::participants::STATUS_CODE status;
    if (!test.wait_for_status_update(action_id, status, STATUS_CODE::STATUS_ACCEPTED))
    {
        std::cerr << "Failed to receive action status update" << std::endl;
        return -1;
    }

    int received_action_feedbacks = 0;
    UUID received_action_id;
    if (!test.wait_for_feedback(received_action_feedbacks, received_action_id) || !(received_action_id == action_id))
    {
        std::cerr << "Failed to receive action feedback" << std::endl;
        return -1;
    }

    if (!enabler->cancel_action_goal(action_name, action_id, 0))
    {
        std::cerr << "Failed to cancel action goal" << std::endl;
        return -1;
    }
    if (!test.wait_for_status_update(action_id, status, STATUS_CODE::STATUS_CANCELED))
    {
        std::cerr << "Failed to receive action status update for cancellation" << std::endl;
        return -1;
    }

    return 0;
}

int action_server()
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

    std::string action_name = "fibonacci/_action/";

    if (enabler->revoke_action(action_name))
    {
        std::cerr << "Action revoked successfully before announcement" << std::endl;
        return -1;
    }

    if (!enabler->announce_action(action_name))
    {
        std::cerr << "Failed to announce action" << std::endl;
        return -1;
    }

    if (enabler->announce_action(action_name))
    {
        std::cerr << "Action announced again, should have failed." << std::endl;
        return -1;
    }

    std::vector<uint64_t> fibonacci_sequence = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610};
    uint16_t received_requests = 0;
    UUID request_uuid;
    while(received_requests < 3)
    {
        uint64_t fibonacci_number = 0;
        if (!test.wait_for_action_request_notification(fibonacci_number, request_uuid))
        {
            std::cerr << "Timeout waiting for action request notification" << std::endl;
            return -1;
        }
        if (!enabler->update_action_status(
                action_name.c_str(),
                request_uuid,
                eprosima::ddsenabler::participants::STATUS_CODE::STATUS_EXECUTING))
        {
            std::cerr << "Failed to update action status to executing" << std::endl;
            return -1;
        }

        std::string json = "{\"sequence\": [";
        std::string feedback_json = "{\"partial_sequence\": [";
        for (size_t i = 0; i < fibonacci_number; ++i)
        {
            json += std::to_string(fibonacci_sequence[i]);
            feedback_json += std::to_string(fibonacci_sequence[i]);

            std::string feedback_tmp = feedback_json;
            feedback_tmp += "]}";
            if (!enabler->send_action_feedback(
                    action_name.c_str(),
                    feedback_tmp.c_str(),
                    request_uuid))
            {
                std::cerr << "Failed to send action feedback" << std::endl;
                return -1;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (i != fibonacci_number - 1)
            {
                json += ", ";
                feedback_json += ", ";
            }
        }
        json += "]}";

        if (!enabler->send_action_result(
                action_name.c_str(),
                request_uuid,
                eprosima::ddsenabler::participants::STATUS_CODE::STATUS_SUCCEEDED,
                json.c_str()));
        received_requests++;
    }

    if (!enabler->revoke_action(action_name))
    {
        std::cerr << "Failed to revoke action" << std::endl;
        return -1;
    }

    if (enabler->revoke_action(action_name))
    {
        std::cerr << "Action revoked again, should have failed." << std::endl;
        return -1;
    }

    return 0;
}

// TODO test being requested to cancel action goal
// int action_server_cancel()
// {
//     auto enabler = create_ddsenabler();
//     ASSERT_TRUE(enabler != nullptr);

//     // Get time for later timeout
//     auto start_time = std::chrono::steady_clock::now();
//     std::string action_name = "fibonacci/_action/";

//     std::this_thread::sleep_for(std::chrono::seconds(6));

//     ASSERT_FALSE(enabler->revoke_action(action_name));

//     ASSERT_TRUE(enabler->announce_action(action_name));

//     ASSERT_FALSE(enabler->announce_action(action_name));

//     uint16_t received_requests = 0;
//     UUID request_uuid;
//     std::cout << "Waiting for service to be available (REQUIRED MANUAL LAUNCH OF FIBONACCI CLIENT)..." << std::endl;
//     std::this_thread::sleep_for(std::chrono::seconds(1));

//     while(received_requests < 3)
//     {
//         uint64_t fibonacci_number = 0;
//         ASSERT_TRUE(test.wait_for_action_request_notification(fibonacci_number, request_uuid, 100));
//         ASSERT_TRUE(enabler->update_action_status(
//             action_name.c_str(),
//             request_uuid,
//             eprosima::ddsenabler::participants::STATUS_CODE::STATUS_EXECUTING));
//         std::string feedback_json = "{\"partial_sequence\": [0]}";
//         ASSERT_TRUE(enabler->send_action_feedback(
//             action_name.c_str(),
//             feedback_json.c_str(),
//             request_uuid));
//         std::cout << "Waiting for action cancel request" << std::endl;
//         uint64_t cancel_request_id = 0;
//         ASSERT_TRUE(test.wait_for_action_cancel_request(cancel_request_id));
//         ASSERT_TRUE(enabler->send_action_cancel_goal_reply(
//             action_name.c_str(),
//             std::vector<UUID>{request_uuid},
//             eprosima::ddsenabler::participants::CANCEL_CODE::ERROR_NONE,
//             cancel_request_id));
//         ASSERT_TRUE(enabler->update_action_status(
//             action_name.c_str(),
//             request_uuid,
//             eprosima::ddsenabler::participants::STATUS_CODE::STATUS_CANCELED));
//         std::string canceled_result_json = "{\"sequence\": []}";
//         ASSERT_TRUE(enabler->send_action_result(
//             action_name.c_str(),
//             request_uuid,
//             eprosima::ddsenabler::participants::STATUS_CODE::STATUS_CANCELED,
//             canceled_result_json.c_str()));
//         received_requests++;
//     }

//     ASSERT_TRUE(enabler->revoke_action(action_name));
//     ASSERT_FALSE(enabler->revoke_action(action_name));
// }