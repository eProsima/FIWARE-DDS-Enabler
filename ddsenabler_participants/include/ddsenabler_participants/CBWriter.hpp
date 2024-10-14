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

#include <mutex>

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>

#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsenabler_participants/library/library_dll.h>
#include <ddsenabler_participants/CBCallbacks.hpp>
#include <ddsenabler_participants/types/CBMessage.hpp>
#include <ddsenabler_participants/types/KnownType.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

class CBWriter
{
public:

    CBWriter() = default;
    ~CBWriter() = default;

    /**
     * @brief Set the callback to notify the context broker of data reception.
     *
     * @param [in] callback Callback to the contest broker.
     */
    void set_data_callback(
            DdsNotification callback);

    /**
     * @brief Set the callback to notify the context broker of type reception.
     *
     * @param [in] callback Callback to the contest broker.
     */
    void set_type_callback(
            DdsTypeNotification callback);

    /**
     * @brief Writes data.
     *
     * @param [in] msg Pointer to the data to be written.
     * @param [in] known_type Structure containing the type information required.
     */
    void write_data(
            const CBMessage& msg,
            KnownType& known_type);

    /**
     * @brief Writes the type information used in this topic the first time it is received.
     *
     * @param [in] msg Pointer to the data.
     * @param [in] known_type Structure containing the type information required.
     */
    void write_schema(
            const CBMessage& msg,
            KnownType& known_type);

    /**
     * @brief Returns the dyn_data of a dyn_type.
     *
     * @param [in] msg Pointer to the data.
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    fastdds::dds::DynamicData::_ref_type get_dynamic_data(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type) noexcept;

    // The mutex to protect the calls to write
    std::mutex mutex_;

    // Callbacks to notify the CB
    DdsNotification data_callback_;
    DdsTypeNotification type_callback_;
};

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
