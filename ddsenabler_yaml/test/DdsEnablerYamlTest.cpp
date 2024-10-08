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

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <ddspipe_yaml/YamlReader.hpp>

#include <EnablerConfiguration.hpp>

using namespace eprosima;
using namespace eprosima::ddsenabler::yaml;

TEST(DdsEnablerYamlTest, get_ddsenabler_correct_configuration_json)
{
    const char* json_str =
            "{"
            "  \"dds\": {"
            "    \"domain\": 0"
            "  },"
            "  \"ddsenabler\": null,"
            "  \"specs\": {"
            "    \"threads\": 12,"
            "    \"logging\": {"
            "      \"verbosity\": \"info\","
            "      \"filter\": {"
            "        \"error\": \"DDSENABLER_ERROR\","
            "        \"warning\": \"DDSENABLER_WARNING\","
            "        \"info\": \"DDSENABLER_INFO\""
            "      }"
            "    }"
            "  }"
            "}";

    nlohmann::json json = nlohmann::json::parse(json_str);

    // Load configuration from JSON
    EnablerConfiguration configuration(true, json);

    utils::Formatter error_msg;

    ASSERT_TRUE(configuration.is_valid(error_msg));
    ASSERT_EQ(configuration.n_threads, 12);

    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.is_valid(error_msg));
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.verbosity.get_value(), utils::VerbosityKind::Info);
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Error].get_value(),
            "DDSENABLER_ERROR");
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Warning].get_value(),
            "DDSENABLER_WARNING");
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Info].get_value(),
            "DDSENABLER_INFO");
}

TEST(DdsEnablerYamlTest, get_ddsenabler_correct_configuration_yaml)
{
    const char* yml_str =
            R"(
            dds:
              domain: 0

            ddsenabler:

            specs:
              threads: 12
              logging:
                verbosity: info
                filter:
                  error: "DDSENABLER_ERROR"
                  warning: "DDSENABLER_WARNING"
                  info: "DDSENABLER_INFO"
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    EnablerConfiguration configuration(yml);

    utils::Formatter error_msg;

    ASSERT_TRUE(configuration.is_valid(error_msg));
    ASSERT_EQ(configuration.n_threads, 12);

    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.is_valid(error_msg));
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.verbosity.get_value(), utils::VerbosityKind::Info);
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Error].get_value(),
            "DDSENABLER_ERROR");
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Warning].get_value(),
            "DDSENABLER_WARNING");
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Info].get_value(),
            "DDSENABLER_INFO");
}

TEST(DdsEnablerYamlTest, get_ddsenabler_incorrect_n_threads_configuration_yaml)
{
    const char* yml_str =
            R"(
            specs:
              threads: error
        )";

    Yaml yml = YAML::Load(yml_str);

    EXPECT_THROW({EnablerConfiguration configuration(yml);}, std::exception);

    yml_str =
            R"(
            specs:
              threads: 0
        )";

    yml = YAML::Load(yml_str);

    // Load configuration from YAML
    EXPECT_THROW({EnablerConfiguration configuration(yml);}, std::exception);

    yml_str =
            R"(
            specs:
              threads: -1
        )";

    yml = YAML::Load(yml_str);

    // Load configuration from YAML
    EXPECT_THROW({EnablerConfiguration configuration(yml);}, std::exception);
}


TEST(DdsEnablerYamlTest, get_ddsenabler_default_values_configuration_yaml)
{
    const char* yml_str =
            R"(
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    EnablerConfiguration configuration(yml);

    utils::Formatter error_msg;

    ASSERT_TRUE(configuration.is_valid(error_msg));
    ASSERT_EQ(configuration.n_threads, DEFAULT_N_THREADS);
}

TEST(DdsEnablerYamlTest, get_ddsenabler_incorrect_path_configuration_yaml)
{
    const char* path_str = "incorrect/path/file";

    EXPECT_THROW({EnablerConfiguration configuration(path_str);}, eprosima::utils::ConfigurationException);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
