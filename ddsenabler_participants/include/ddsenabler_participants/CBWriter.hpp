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
 * @file CBWriter.hpp
 */

#pragma once

#include <map>

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsenabler_participants/CBCallbacks.hpp>
#include <ddsenabler_participants/CBMessage.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

class CBWriter
{

public:

    DDSENABLER_PARTICIPANTS_DllAPI
    CBWriter() = default;

    DDSENABLER_PARTICIPANTS_DllAPI
    ~CBWriter() = default;

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_data_notification_callback(
            DdsDataNotification callback)
    {
        data_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_type_notification_callback(
            DdsTypeNotification callback)
    {
        type_notification_callback_ = callback;
    }

    DDSENABLER_PARTICIPANTS_DllAPI
    void set_topic_notification_callback(
            DdsTopicNotification callback)
    {
        topic_notification_callback_ = callback;
    }

    /**
     * @brief Writes the schema of a DynamicType to CB.
     *
     * @param [in] dyn_type DynamicType containing the type information required.
     * @param [in] type_id TypeIdentifier of the DynamicType.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void write_schema(
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const fastdds::dds::xtypes::TypeIdentifier& type_id);

    /**
     * @brief Writes the topic to CB.
     *
     * @param [in] topic DDS topic to be added.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void write_topic(
            const ddspipe::core::types::DdsTopic& topic);

    /**
     * @brief Writes data.
     *
     * @param [in] msg Pointer to the data to be written.
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    DDSENABLER_PARTICIPANTS_DllAPI
    void write_data(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

protected:

    /**
     * @brief Writes the type information used in this topic the first time it is received.
     *
     * @param [in] msg Pointer to the data.
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    void write_schema_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    /**
     * @brief Returns the dyn_data of a dyn_type.
     *
     * @param [in] msg Pointer to the data.
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    fastdds::dds::DynamicData::_ref_type get_dynamic_data_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type) noexcept;

    /**
     * @brief Returns the pubsub type of a dyn_type.
     *
     * @param [in] dyn_type DynamicType from which to get the pubsub type.
     * @return The pubsub type associated to the given dyn_type.
     * @note If the pubsub type is not already created, it will be created and stored in the map.
     */
    fastdds::dds::DynamicPubSubType get_pubsub_type_(
            const fastdds::dds::DynamicType::_ref_type& dyn_type) noexcept;

    // Callbacks to notify the CB
    DdsDataNotification data_notification_callback_;
    DdsTypeNotification type_notification_callback_;
    DdsTopicNotification topic_notification_callback_;

    // Map to store the pubsub types associated to dynamic types so they can be reused
    std::map<fastdds::dds::DynamicType::_ref_type, fastdds::dds::DynamicPubSubType> dynamic_pubsub_types_;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
