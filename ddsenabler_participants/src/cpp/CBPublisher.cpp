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

ReturnCode_t CBPublisher::publish_data(
        KnownType& a_type,
        const std::string data_json)
{
    // WIP Use JSON -> DynamicData FUNCTION
    DynamicData::_ref_type sample = DynamicDataFactory::get_instance()->create_data(a_type.dyn_type_);

    auto ret = a_type.writer_->write(&sample);
    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER,
                "CBPublisher::publish_data: " << a_type.dyn_type_->get_name().to_string() << "." );
    }

    return ret;
}

bool CBPublisher::create_participant()
{
    participant_ = DomainParticipantFactory::get_instance()
                    ->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    if (participant_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER,
                "Error creating CB publisher DomainParticipant.");
        return false;
    }

    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
    if (publisher_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER,
                "Error creating CB publisher Publisher.");
        return false;
    }

    return true;
}

bool CBPublisher::create_writer(
        KnownType& a_type)
{
    try
    {
        if (RETCODE_OK != a_type.type_sup_.register_type(participant_))
        {
            EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER, "Error register_type: " <<
                    a_type.type_sup_.get_type_name());
            return false;
        }

        std::ostringstream topic_name;
        topic_name << "CBPublisher" << a_type.type_sup_.get_type_name() << "TopicName";
        Topic* topic = participant_->create_topic(topic_name.str(), a_type.type_sup_.get_type_name(),
                        TOPIC_QOS_DEFAULT);
        if (topic == nullptr)
        {
            EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER, "Error create_topic: " <<
                    a_type.type_sup_.get_type_name());
            return false;
        }

        DataWriterQos wqos = publisher_->get_default_datawriter_qos();
        a_type.writer_ = publisher_->create_datawriter(topic, wqos);
        if (a_type.writer_ == nullptr)
        {
            EPROSIMA_LOG_ERROR(DDSENABLER_CB_PUBLISHER, "Error create_datawriter: " <<
                    a_type.type_sup_.get_type_name());
            return false;
        }

        a_type.has_writer_ = true;
    }
    catch (const std::exception& e)
    {
        return false;
    }

    return true;
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
