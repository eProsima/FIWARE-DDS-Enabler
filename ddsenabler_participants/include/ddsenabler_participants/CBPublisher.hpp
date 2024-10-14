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
 * @file CBPublisher.hpp
 */
#pragma once

#include <mutex>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include <ddsenabler_participants/types/KnownType.hpp>

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace ddsenabler {
namespace participants {

/**
 * Class that manages publishing data to the DDS environment from the context broker.
 *
 */
class CBPublisher
{

public:

    /**
     * @brief Constructor
     */
    CBPublisher();

    /**
     * @brief Destructor
     */
    ~CBPublisher();

    /**
     * @brief Get the guidPrefix of the publisher participant.
     */
    eprosima::fastdds::rtps::GuidPrefix_t get_publisher_guid();

    /**
     * @brief Create a writer for a type in a given topic.
     *
     * @param [in] topic_name The name of the topic.
     * @param [in] a_type Object containing the information of the type.
     */
    bool create_writer(
            std::string topic_name,
            KnownType& a_type);

    /**
     * @brief Publish a data sample.
     *
     * @param [in] topic_name The name of the topic.
     * @param [in] a_type Object containing the information of the type.
     * @param [in] data_json JSON representation of the content.
     *
     * @return \c RETCODE_OK if data is published correctly.
     * @return \c RETCODE_PRECONDITION_NOT_MET if writer does not exist.
     * @return \c ReturnCode_t if error when writing.
     */
    ReturnCode_t publish_data(
            std::string topic_name,
            KnownType& a_type,
            const std::string data_json);

protected:

    /**
     * @brief Create the participant and publisher for the DDSEnabler
     */
    void create_participant();

    //! Participant to be used by the DDSEnabler
    DomainParticipant* participant_ = nullptr;

    //! Publisher to be used by the DDSEnabler
    Publisher* publisher_ = nullptr;

    //! Mutex to protect acces to writers_
    std::mutex writers_mutex_;

    //! Map of the writers that have been created
    std::unordered_map<std::string, DataWriter*> writers_;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
