// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#ifndef MACROS_HPP_
#define MACROS_HPP_

#include <limits>
#include <string>

#define DEFINE_DYNAMIC_DATA_METHODS(TYPE, DDSTYPE, METHOD_TYPE) \
  template<> \
  DDS_ReturnCode_t set_dynamic_data<TYPE, DDSTYPE>( \
    DDS_DynamicData * dynamic_data, size_t index, const DDSTYPE value) \
  { \
    auto member_id = static_cast<DDS_DynamicDataMemberId>(index); \
    return dynamic_data->set_ ## METHOD_TYPE(NULL, member_id, value); \
  } \
  template<> \
  DDS_ReturnCode_t set_dynamic_data_array<TYPE, DDSTYPE>( \
    DDS_DynamicData * dynamic_data, size_t index, size_t array_size, const DDSTYPE * values) \
  { \
    auto member_id = static_cast<DDS_DynamicDataMemberId>(index); \
    return dynamic_data->set_ ## METHOD_TYPE ## _array(NULL, member_id, \
             static_cast<DDS_UnsignedLong>(array_size), values); \
  } \
  template<> \
  DDS_ReturnCode_t get_dynamic_data<TYPE, DDSTYPE>( \
    DDS_DynamicData * dynamic_data, DDSTYPE & value, size_t index) \
  { \
    return dynamic_data->get_ ## METHOD_TYPE( \
      value, \
      NULL, \
      static_cast<DDS_DynamicDataMemberId>(index)); \
  } \
  template<> \
  DDS_ReturnCode_t get_dynamic_data_array<TYPE, DDSTYPE>( \
    DDS_DynamicData * dynamic_data, DDSTYPE * &values, size_t array_size, size_t index) \
  { \
    DDS_UnsignedLong length = static_cast<DDS_UnsignedLong>(array_size); \
    return dynamic_data->get_ ## METHOD_TYPE ## _array( \
      values, \
      &length, \
      NULL, \
      static_cast<DDS_DynamicDataMemberId>(index)); \
  } \


#define SPECIALIZE_GENERIC_C_ARRAY(C_NAME, C_TYPE) \
  template<> \
  struct GenericCArray<C_TYPE> \
  { \
    using type = rosidl_generator_c__ ## C_NAME ## __Array; \
  };

#endif  // MACROS_HPP_
