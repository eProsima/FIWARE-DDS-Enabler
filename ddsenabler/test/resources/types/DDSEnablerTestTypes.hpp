// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file DDSEnablerTestTypes.hpp
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool fastddsgen.
 */

#ifndef FAST_DDS_GENERATED__DDSENABLERTESTTYPES_HPP
#define FAST_DDS_GENERATED__DDSENABLERTESTTYPES_HPP

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <fastcdr/cdr/fixed_size_string.hpp>

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(DDSENABLERTESTTYPES_SOURCE)
#define DDSENABLERTESTTYPES_DllAPI __declspec( dllexport )
#else
#define DDSENABLERTESTTYPES_DllAPI __declspec( dllimport )
#endif // DDSENABLERTESTTYPES_SOURCE
#else
#define DDSENABLERTESTTYPES_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define DDSENABLERTESTTYPES_DllAPI
#endif // _WIN32

/*!
 * @brief This class represents the structure DDSEnablerTestType1 defined by the user in the IDL file.
 * @ingroup DDSEnablerTestTypes
 */
class DDSEnablerTestType1
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport DDSEnablerTestType1()
    {
    }

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~DDSEnablerTestType1()
    {
    }

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object DDSEnablerTestType1 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType1(
            const DDSEnablerTestType1& x)
    {
                    m_value = x.m_value;

    }

    /*!
     * @brief Move constructor.
     * @param x Reference to the object DDSEnablerTestType1 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType1(
            DDSEnablerTestType1&& x) noexcept
    {
        m_value = x.m_value;
    }

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object DDSEnablerTestType1 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType1& operator =(
            const DDSEnablerTestType1& x)
    {

                    m_value = x.m_value;

        return *this;
    }

    /*!
     * @brief Move assignment.
     * @param x Reference to the object DDSEnablerTestType1 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType1& operator =(
            DDSEnablerTestType1&& x) noexcept
    {

        m_value = x.m_value;
        return *this;
    }

    /*!
     * @brief Comparison operator.
     * @param x DDSEnablerTestType1 object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const DDSEnablerTestType1& x) const
    {
        return (m_value == x.m_value);
    }

    /*!
     * @brief Comparison operator.
     * @param x DDSEnablerTestType1 object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const DDSEnablerTestType1& x) const
    {
        return !(*this == x);
    }

    /*!
     * @brief This function sets a value in member value
     * @param _value New value for member value
     */
    eProsima_user_DllExport void value(
            int16_t _value)
    {
        m_value = _value;
    }

    /*!
     * @brief This function returns the value of member value
     * @return Value of member value
     */
    eProsima_user_DllExport int16_t value() const
    {
        return m_value;
    }

    /*!
     * @brief This function returns a reference to member value
     * @return Reference to member value
     */
    eProsima_user_DllExport int16_t& value()
    {
        return m_value;
    }



private:

    int16_t m_value{0};

};
/*!
 * @brief This class represents the structure DDSEnablerTestType2 defined by the user in the IDL file.
 * @ingroup DDSEnablerTestTypes
 */
class DDSEnablerTestType2
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport DDSEnablerTestType2()
    {
    }

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~DDSEnablerTestType2()
    {
    }

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object DDSEnablerTestType2 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType2(
            const DDSEnablerTestType2& x)
    {
                    m_value = x.m_value;

    }

    /*!
     * @brief Move constructor.
     * @param x Reference to the object DDSEnablerTestType2 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType2(
            DDSEnablerTestType2&& x) noexcept
    {
        m_value = std::move(x.m_value);
    }

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object DDSEnablerTestType2 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType2& operator =(
            const DDSEnablerTestType2& x)
    {

                    m_value = x.m_value;

        return *this;
    }

    /*!
     * @brief Move assignment.
     * @param x Reference to the object DDSEnablerTestType2 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType2& operator =(
            DDSEnablerTestType2&& x) noexcept
    {

        m_value = std::move(x.m_value);
        return *this;
    }

    /*!
     * @brief Comparison operator.
     * @param x DDSEnablerTestType2 object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const DDSEnablerTestType2& x) const
    {
        return (m_value == x.m_value);
    }

    /*!
     * @brief Comparison operator.
     * @param x DDSEnablerTestType2 object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const DDSEnablerTestType2& x) const
    {
        return !(*this == x);
    }

    /*!
     * @brief This function copies the value in member value
     * @param _value New value to be copied in member value
     */
    eProsima_user_DllExport void value(
            const std::string& _value)
    {
        m_value = _value;
    }

    /*!
     * @brief This function moves the value in member value
     * @param _value New value to be moved in member value
     */
    eProsima_user_DllExport void value(
            std::string&& _value)
    {
        m_value = std::move(_value);
    }

    /*!
     * @brief This function returns a constant reference to member value
     * @return Constant reference to member value
     */
    eProsima_user_DllExport const std::string& value() const
    {
        return m_value;
    }

    /*!
     * @brief This function returns a reference to member value
     * @return Reference to member value
     */
    eProsima_user_DllExport std::string& value()
    {
        return m_value;
    }



private:

    std::string m_value;

};
/*!
 * @brief This class represents the structure DDSEnablerTestType3 defined by the user in the IDL file.
 * @ingroup DDSEnablerTestTypes
 */
class DDSEnablerTestType3
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport DDSEnablerTestType3()
    {
    }

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~DDSEnablerTestType3()
    {
    }

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object DDSEnablerTestType3 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType3(
            const DDSEnablerTestType3& x)
    {
                    m_value = x.m_value;

    }

    /*!
     * @brief Move constructor.
     * @param x Reference to the object DDSEnablerTestType3 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType3(
            DDSEnablerTestType3&& x) noexcept
    {
        m_value = std::move(x.m_value);
    }

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object DDSEnablerTestType3 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType3& operator =(
            const DDSEnablerTestType3& x)
    {

                    m_value = x.m_value;

        return *this;
    }

    /*!
     * @brief Move assignment.
     * @param x Reference to the object DDSEnablerTestType3 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType3& operator =(
            DDSEnablerTestType3&& x) noexcept
    {

        m_value = std::move(x.m_value);
        return *this;
    }

    /*!
     * @brief Comparison operator.
     * @param x DDSEnablerTestType3 object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const DDSEnablerTestType3& x) const
    {
        return (m_value == x.m_value);
    }

    /*!
     * @brief Comparison operator.
     * @param x DDSEnablerTestType3 object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const DDSEnablerTestType3& x) const
    {
        return !(*this == x);
    }

    /*!
     * @brief This function copies the value in member value
     * @param _value New value to be copied in member value
     */
    eProsima_user_DllExport void value(
            const std::array<int32_t, 10>& _value)
    {
        m_value = _value;
    }

    /*!
     * @brief This function moves the value in member value
     * @param _value New value to be moved in member value
     */
    eProsima_user_DllExport void value(
            std::array<int32_t, 10>&& _value)
    {
        m_value = std::move(_value);
    }

    /*!
     * @brief This function returns a constant reference to member value
     * @return Constant reference to member value
     */
    eProsima_user_DllExport const std::array<int32_t, 10>& value() const
    {
        return m_value;
    }

    /*!
     * @brief This function returns a reference to member value
     * @return Reference to member value
     */
    eProsima_user_DllExport std::array<int32_t, 10>& value()
    {
        return m_value;
    }



private:

    std::array<int32_t, 10> m_value{0};

};
/*!
 * @brief This class represents the structure DDSEnablerTestType4 defined by the user in the IDL file.
 * @ingroup DDSEnablerTestTypes
 */
class DDSEnablerTestType4
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport DDSEnablerTestType4()
    {
    }

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~DDSEnablerTestType4()
    {
    }

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object DDSEnablerTestType4 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType4(
            const DDSEnablerTestType4& x)
    {
                    m_value = x.m_value;

    }

    /*!
     * @brief Move constructor.
     * @param x Reference to the object DDSEnablerTestType4 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType4(
            DDSEnablerTestType4&& x) noexcept
    {
        m_value = std::move(x.m_value);
    }

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object DDSEnablerTestType4 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType4& operator =(
            const DDSEnablerTestType4& x)
    {

                    m_value = x.m_value;

        return *this;
    }

    /*!
     * @brief Move assignment.
     * @param x Reference to the object DDSEnablerTestType4 that will be copied.
     */
    eProsima_user_DllExport DDSEnablerTestType4& operator =(
            DDSEnablerTestType4&& x) noexcept
    {

        m_value = std::move(x.m_value);
        return *this;
    }

    /*!
     * @brief Comparison operator.
     * @param x DDSEnablerTestType4 object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const DDSEnablerTestType4& x) const
    {
        return (m_value == x.m_value);
    }

    /*!
     * @brief Comparison operator.
     * @param x DDSEnablerTestType4 object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const DDSEnablerTestType4& x) const
    {
        return !(*this == x);
    }

    /*!
     * @brief This function copies the value in member value
     * @param _value New value to be copied in member value
     */
    eProsima_user_DllExport void value(
            const DDSEnablerTestType1& _value)
    {
        m_value = _value;
    }

    /*!
     * @brief This function moves the value in member value
     * @param _value New value to be moved in member value
     */
    eProsima_user_DllExport void value(
            DDSEnablerTestType1&& _value)
    {
        m_value = std::move(_value);
    }

    /*!
     * @brief This function returns a constant reference to member value
     * @return Constant reference to member value
     */
    eProsima_user_DllExport const DDSEnablerTestType1& value() const
    {
        return m_value;
    }

    /*!
     * @brief This function returns a reference to member value
     * @return Reference to member value
     */
    eProsima_user_DllExport DDSEnablerTestType1& value()
    {
        return m_value;
    }



private:

    DDSEnablerTestType1 m_value;

};

#endif // _FAST_DDS_GENERATED_DDSENABLERTESTTYPES_HPP_

