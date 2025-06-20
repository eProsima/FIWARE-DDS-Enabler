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

#include <nlohmann/json.hpp>

#include <ddspipe_yaml/YamlReader.hpp>

#include <EnablerConfiguration.hpp>

using namespace eprosima;
using namespace eprosima::ddsenabler::yaml;

TEST(DdsEnablerYamlTest, get_ddsenabler_correct_configuration_yaml)
{
    const char* yml_str =
            R"(
            dds:
              domain: 4

            ddsenabler:
                initial-publish-wait: 500

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

    ASSERT_EQ(configuration.simple_configuration->domain.domain_id, 4);
    ASSERT_EQ(configuration.enabler_configuration->initial_publish_wait, 500);
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

    ASSERT_EQ(configuration.simple_configuration->domain.domain_id, 0);
    ASSERT_EQ(configuration.enabler_configuration->initial_publish_wait, 0);
    ASSERT_EQ(configuration.n_threads, DEFAULT_N_THREADS);
}

TEST(DdsEnablerYamlTest, get_ddsenabler_incorrect_path_configuration_json)
{
    const char* path_str = "incorrect/path/file.json";

    EXPECT_THROW({EnablerConfiguration configuration(path_str);}, eprosima::utils::ConfigurationException);
}

TEST(DdsEnablerYamlTest, get_ddsenabler_correct_configuration_json)
{
    const char* path_str = "./resources/correct_config.json";

    ASSERT_TRUE(std::filesystem::exists(path_str));

    EnablerConfiguration configuration(path_str);

    utils::Formatter error_msg;

    ASSERT_TRUE(configuration.is_valid(error_msg));

    ASSERT_EQ(configuration.simple_configuration->domain.domain_id, 4);
    ASSERT_EQ(configuration.enabler_configuration->initial_publish_wait, 500);
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

TEST(DdsEnablerYamlTest, get_ddsenabler_incorrect_n_threads_configuration_json)
{
    const char* path_str = "./resources/incorrect_threads_config.json";

    ASSERT_TRUE(std::filesystem::exists(path_str));

    EXPECT_THROW({EnablerConfiguration configuration(path_str);}, std::exception);
}

TEST(DdsEnablerYamlTest, get_ddsenabler_default_values_configuration_json)
{
    const char* path_str = "./resources/default_config.json";

    ASSERT_TRUE(std::filesystem::exists(path_str));

    EnablerConfiguration configuration(path_str);

    utils::Formatter error_msg;

    ASSERT_TRUE(configuration.is_valid(error_msg));

    ASSERT_EQ(configuration.simple_configuration->domain.domain_id, 0);
    ASSERT_EQ(configuration.enabler_configuration->initial_publish_wait, 0);
    ASSERT_EQ(configuration.n_threads, DEFAULT_N_THREADS);
}

TEST(DdsEnablerYamlTest, get_ddsenabler_incorrect_path_configuration_yaml)
{
    const char* path_str = "incorrect/path/file.yaml";

    EXPECT_THROW({EnablerConfiguration configuration(path_str);}, eprosima::utils::ConfigurationException);
}

TEST(DdsEnablerYamlTest, get_ddsenabler_full_configuration_json)
{
    const char* path_str = "./resources/correct_config.json";

    ASSERT_TRUE(std::filesystem::exists(path_str));

    EnablerConfiguration configuration(path_str);

    utils::Formatter error_msg;

    ASSERT_TRUE(configuration.is_valid(error_msg));

    ASSERT_EQ(configuration.simple_configuration->domain.domain_id, 4);
    ASSERT_EQ(configuration.enabler_configuration->initial_publish_wait, 500);
    ASSERT_EQ(configuration.n_threads, 12);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
