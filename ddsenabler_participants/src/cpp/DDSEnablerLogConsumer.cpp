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
 * @file DDSEnablerLogConsumer.cpp
 */

#include <string>
#include <fstream>
#include <string>

#include <cpp_utils/exception/InitializationException.hpp>

#include <ddsenabler_participants/DDSEnablerLogConsumer.hpp>

namespace eprosima {
namespace ddsenabler {
namespace participants {

DDSEnablerLogConsumer::DDSEnablerLogConsumer(
        const ddspipe::core::DdsPipeLogConfiguration* configuration)
    : utils::BaseLogConsumer(configuration)
{
    verbosity_ = configuration->verbosity;
}

void DDSEnablerLogConsumer::set_log_callback(
        DdsLogFunc callback)
{
    log_callback_ = callback;
}

void DDSEnablerLogConsumer::Consume(
        const utils::Log::Entry& entry)
{
    if (!accept_entry_(entry))
    {
        return;
    }

    char* filename = (entry.context.filename != NULL)? (char*) entry.context.filename : (char*) "no-filename";
    char* funcname = (entry.context.function != NULL)? (char*) entry.context.function : (char*) "no-funcname";
    char* message = (entry.message.c_str() != NULL)? (char*) entry.message.c_str() : (char*) "no-message";

    log_callback_(
        filename,
        entry.context.line,
        funcname,
        static_cast<int>(entry.kind),
        message);
}

} /* namespace participants */
} /* namespace ddsenabler */
} /* namespace eprosima */
