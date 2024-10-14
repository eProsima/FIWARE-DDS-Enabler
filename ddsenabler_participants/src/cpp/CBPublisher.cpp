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
 * @file CBPublisher.cpp
 */

#include <ddsenabler_participants/CBPublisher.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::xtypes;

namespace eprosima {
namespace ddsenabler {
namespace participants {

CBPublisher::CBPublisher()
{
    EPROSIMA_LOG_INFO(DDSENABLER_CB_PUBLISHER,
            "Creating CB publisher instance.");

    create_participant();
}

CBPublisher::~CBPublisher()
{
    EPROSIMA_LOG_INFO(DDSENABLER_CB_PUBLISHER,
            "Destroying CB publisher.");
}

eprosima::fastdds::rtps::GuidPrefix_t CBPublisher::get_publisher_guid()
{
    return participant_->guid().guidPrefix;
}

bool CBPublisher::create_writer(
        std::string topic_name,
        KnownType& a_type)
{
    auto it = writers_.find(topic_name);
    if (it == writers_.end())
    {
        //Writer does not exist
        try
        {
            if (RETCODE_OK != a_type.type_sup_.register_type(participant_))
            {
                EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER, "Error register_type: " <<
                        a_type.type_sup_.get_type_name());
                return false;
            }

            Topic* topic = participant_->create_topic(topic_name, a_type.type_sup_.get_type_name(), TOPIC_QOS_DEFAULT);
            if (topic == nullptr)
            {
                EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER, "Error create_topic: " <<
                        a_type.type_sup_.get_type_name());
                return false;
            }

            DataWriterQos wqos = publisher_->get_default_datawriter_qos();
            writers_[topic_name] = publisher_->create_datawriter(topic, wqos);
            if (writers_[topic_name] == nullptr)
            {
                EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER, "Error create_datawriter: " <<
                        a_type.type_sup_.get_type_name());
                return false;
            }
        }
        catch (const std::exception& e)
        {
            return false;
        }
    }
    else
    {
        // Writer already exists
        EPROSIMA_LOG_INFO(DDSENABLER_CB_PUBLISHER,
                "Writer in topic: " + topic_name + " already created.");
    }

    return true;
}

ReturnCode_t CBPublisher::publish_data(
        std::string topic_name,
        KnownType& a_type,
        const std::string data_json)
{
    ReturnCode_t ret;
    auto it = writers_.find(topic_name);
    if (it != writers_.end())
    {
        // WIP Use data_json -> DynamicData FUNCTION
        DynamicData::_ref_type sample = DynamicDataFactory::get_instance()->create_data(a_type.dyn_type_);

        ret = it->second->write(&sample);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER,
                "CBPublisher::publish_data writer does not exist in topic: " << topic_name << "." );
        ret = RETCODE_PRECONDITION_NOT_MET;
    }

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER,
                "CBPublisher::publish_data in Topic  " << topic_name << "." );
    }

    return ret;
}

void CBPublisher::create_participant()
{
    participant_ = DomainParticipantFactory::get_instance()
                    ->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    if (participant_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER,
                "Error creating CB publisher DomainParticipant.");
    }

    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
    if (publisher_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER,
                "Error creating CB publisher Publisher.");
    }
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
