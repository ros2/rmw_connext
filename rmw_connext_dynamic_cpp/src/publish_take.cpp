// Copyright 2014-2015 Open Source Robotics Foundation, Inc.
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

// the code using ROS messages must use the new ABI (_GLIBCXX_USE_CXX11_ABI 1)

#include "publish_take.hpp"

#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "rmw/allocators.h"
#include "rmw/rmw.h"

#include "rosidl_generator_c/string.h"
#include "rosidl_generator_c/string_functions.h"

#include "rmw/impl/cpp/macros.hpp"

#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"

#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"

#include "macros.hpp"

bool using_introspection_c_typesupport(const char * typesupport_identifier)
{
  return typesupport_identifier == rosidl_typesupport_introspection_c__identifier;
}

bool using_introspection_cpp_typesupport(const char * typesupport_identifier)
{
  return typesupport_identifier ==
         rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier;
}

#define SET_PRIMITIVE_VALUE(TYPE, METHOD_NAME) \
  const TYPE * value = \
    reinterpret_cast<const TYPE *>(static_cast<const char *>(ros_message) + member->offset_); \
  DDS_ReturnCode_t status = dynamic_data->METHOD_NAME( \
    NULL, \
    i + 1, \
    *value); \
  if (status != DDS_RETCODE_OK) { \
    RMW_SET_ERROR_MSG("failed to set primitive value using " #METHOD_NAME); \
    return false; \
  }

#define ARRAY_SIZE_AND_VALUES(TYPE) \
  const TYPE * ros_values = nullptr; \
  size_t array_size; \
  if (member->array_size_ && !member->is_upper_bound_) { \
    ros_values = \
      reinterpret_cast<const TYPE *>(static_cast<const char *>(ros_message) + member->offset_); \
    array_size = member->array_size_; \
  } else { \
    const void * untyped_vector = static_cast<const char *>(ros_message) + member->offset_; \
    auto vector = static_cast<const std::vector<TYPE> *>(untyped_vector); \
    ros_values = vector->data(); \
    array_size = vector->size(); \
  }

#define SET_VALUE(TYPE, METHOD_NAME, ARRAY_METHOD_NAME) \
  { \
    if (member->is_array_) { \
      ARRAY_SIZE_AND_VALUES(TYPE) \
      DDS_ReturnCode_t status = dynamic_data->ARRAY_METHOD_NAME( \
        NULL, \
        i + 1, \
        static_cast<DDS_UnsignedLong>(array_size), \
        ros_values); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to set array value using " #ARRAY_METHOD_NAME); \
        return false; \
      } \
    } else { \
      SET_PRIMITIVE_VALUE(TYPE, METHOD_NAME) \
    } \
  }

#define SET_VALUE_WITH_DIFFERENT_TYPES(TYPE, DDS_TYPE, METHOD_NAME, ARRAY_METHOD_NAME) \
  { \
    if (member->is_array_) { \
      ARRAY_SIZE_AND_VALUES(TYPE) \
      DDS_TYPE * values = nullptr; \
      if (array_size > 0) { \
        values = static_cast<DDS_TYPE *>(rmw_allocate(sizeof(DDS_TYPE) * array_size)); \
        if (!values) { \
          RMW_SET_ERROR_MSG("failed to allocate memory"); \
          return false; \
        } \
        for (size_t j = 0; j < array_size; ++j) { \
          values[j] = ros_values[j]; \
        } \
      } \
      DDS_ReturnCode_t status = dynamic_data->ARRAY_METHOD_NAME( \
        NULL, \
        i + 1, \
        static_cast<DDS_UnsignedLong>(array_size), \
        values); \
      rmw_free(values); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to set array value using " #ARRAY_METHOD_NAME); \
        return false; \
      } \
    } else { \
      SET_PRIMITIVE_VALUE(TYPE, METHOD_NAME) \
    } \
  }

#define SET_VALUE_WITH_BOOL_TYPE(TYPE, DDS_TYPE, METHOD_NAME, ARRAY_METHOD_NAME) \
  { \
    if (member->is_array_) { \
      DDS_TYPE * values = nullptr; \
      size_t array_size; \
      if (member->array_size_ && !member->is_upper_bound_) { \
        array_size = member->array_size_; \
        auto ros_values = \
          reinterpret_cast<const TYPE *>( \
          static_cast<const char *>(ros_message) + member->offset_); \
        values = static_cast<DDS_TYPE *>(rmw_allocate(sizeof(DDS_TYPE) * array_size)); \
        if (!values) { \
          RMW_SET_ERROR_MSG("failed to allocate memory"); \
          return false; \
        } \
        for (size_t j = 0; j < array_size; ++j) { \
          values[j] = ros_values[j]; \
        } \
      } else { \
        const void * untyped_vector = static_cast<const char *>(ros_message) + member->offset_; \
        auto vector = static_cast<const std::vector<TYPE> *>(untyped_vector); \
        array_size = vector->size(); \
        if (array_size > (std::numeric_limits<DDS_UnsignedLong>::max)()) { \
          RMW_SET_ERROR_MSG( \
            "failed to set values since the requested array size exceeds the DDS type"); \
          return false; \
        } \
        if (array_size > 0) { \
          values = static_cast<DDS_TYPE *>(rmw_allocate(sizeof(DDS_TYPE) * array_size)); \
          if (!values) { \
            RMW_SET_ERROR_MSG("failed to allocate memory"); \
            return false; \
          } \
          for (size_t j = 0; j < array_size; ++j) { \
            values[j] = (*vector)[j]; \
          } \
        } \
      } \
      DDS_ReturnCode_t status = dynamic_data->ARRAY_METHOD_NAME( \
        NULL, \
        i + 1, \
        static_cast<DDS_UnsignedLong>(array_size), \
        values); \
      rmw_free(values); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to set array value using " #ARRAY_METHOD_NAME); \
        return false; \
      } \
    } else { \
      SET_PRIMITIVE_VALUE(TYPE, METHOD_NAME) \
    } \
  }

#define SET_STRING_VALUE(TYPE, METHOD_NAME, ACCESSOR) \
  { \
    if (member->is_array_) { \
      DDS_DynamicData dynamic_data_member(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT); \
      DDS_ReturnCode_t status = dynamic_data->bind_complex_member( \
        dynamic_data_member, \
        NULL, \
        static_cast<DDS_DynamicDataMemberId>(i + 1)); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to bind complex member"); \
        return false; \
      } \
      ARRAY_SIZE_AND_VALUES(TYPE) \
      if (array_size > (std::numeric_limits<DDS_DynamicDataMemberId>::max)()) { \
        RMW_SET_ERROR_MSG( \
          "failed to set string since the requested string length exceeds the DDS type"); \
        return false; \
      } \
      for (size_t j = 0; j < array_size; ++j) { \
        status = dynamic_data_member.METHOD_NAME( \
          NULL, \
          static_cast<DDS_DynamicDataMemberId>(j + 1), \
          ros_values[j].ACCESSOR); \
        if (status != DDS_RETCODE_OK) { \
          RMW_SET_ERROR_MSG("failed to set array value using " #METHOD_NAME); \
          return false; \
        } \
      } \
      status = dynamic_data->unbind_complex_member(dynamic_data_member); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to unbind complex member"); \
        return false; \
      } \
    } else { \
      const TYPE * value = \
        reinterpret_cast<const TYPE *>(static_cast<const char *>(ros_message) + member->offset_); \
      DDS_ReturnCode_t status = dynamic_data->METHOD_NAME( \
        NULL, \
        static_cast<DDS_DynamicDataMemberId>(i + 1), \
        value->ACCESSOR); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to set value using " #METHOD_NAME); \
        return false; \
      } \
    } \
  }

#define SET_SUBMESSAGE_VALUE(dynamic_data, i, INTROSPECTION_TYPE) \
  DDS_DynamicData sub_dynamic_data(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT); \
  DDS_ReturnCode_t status = dynamic_data->bind_complex_member( \
    sub_dynamic_data, \
    NULL, \
    static_cast<DDS_DynamicDataMemberId>(i + 1)); \
  if (status != DDS_RETCODE_OK) { \
    RMW_SET_ERROR_MSG("failed to bind complex member"); \
    return false; \
  } \
  const void * sub_ros_message = static_cast<const char *>(ros_message) + member->offset_; \
  if (!member->members_) { \
    RMW_SET_ERROR_MSG("members handle is null"); \
    return false; \
  } \
  bool published = publish<INTROSPECTION_TYPE(MessageMembers)>( \
    &sub_dynamic_data, sub_ros_message, member->members_->data, typesupport); \
  if (!published) { \
    DDS_UnsignedLong count = sub_dynamic_data.get_member_count(); \
    for (DDS_UnsignedLong k = 0; k < count; ++k) { \
      DDS_DynamicDataMemberInfo info; \
      status = sub_dynamic_data.get_member_info_by_index(info, k); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to get member info"); \
        return false; \
      } \
    } \
  } \
  status = dynamic_data->unbind_complex_member(sub_dynamic_data); \
  if (!published) { \
    RMW_SET_ERROR_MSG("failed to publish sub message"); \
    return false; \
  } \
  if (status != DDS_RETCODE_OK) { \
    RMW_SET_ERROR_MSG("failed to unbind complex member"); \
    return false; \
  }

template<typename MembersType>
bool publish(
  DDS_DynamicData * dynamic_data, const void * ros_message,
  const void * untyped_members, const char * typesupport)
{
  auto members = static_cast<const MembersType *>(untyped_members);
  if (!members) {
    RMW_SET_ERROR_MSG("members handle is null");
    return false;
  }
  for (uint32_t i = 0; i < members->member_count_; ++i) {
    auto * member = members->members_ + i;
    switch (member->type_id_) {
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
        SET_VALUE_WITH_BOOL_TYPE(bool, DDS_Boolean, set_boolean, set_boolean_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
        SET_VALUE(uint8_t, set_octet, set_octet_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
        SET_VALUE(char, set_char, set_char_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
        SET_VALUE(float, set_float, set_float_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
        SET_VALUE(double, set_double, set_double_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
        SET_VALUE_WITH_DIFFERENT_TYPES(int8_t, DDS_Octet, set_octet, set_octet_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
        SET_VALUE(uint8_t, set_octet, set_octet_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
        SET_VALUE(int16_t, set_short, set_short_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
        SET_VALUE(uint16_t, set_ushort, set_ushort_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
        SET_VALUE(int32_t, set_long, set_long_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
        SET_VALUE(uint32_t, set_ulong, set_ulong_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
        SET_VALUE_WITH_DIFFERENT_TYPES(int64_t, DDS_LongLong, set_longlong, set_longlong_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
        SET_VALUE_WITH_DIFFERENT_TYPES(
          uint64_t, DDS_UnsignedLongLong, set_ulonglong, set_ulonglong_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
        if (using_introspection_c_typesupport(typesupport)) {
          SET_STRING_VALUE(rosidl_generator_c__String, set_string, data)
        } else if (using_introspection_cpp_typesupport(typesupport)) {
          SET_STRING_VALUE(std::string, set_string, c_str())
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE:
        {
          if (member->is_array_) {
            const void * untyped_member = static_cast<const char *>(ros_message) + member->offset_;
            if (!member->size_function) {
              RMW_SET_ERROR_MSG("size function handle is null");
              return false;
            }
            if (!member->get_const_function) {
              RMW_SET_ERROR_MSG("get const function handle is null");
              return false;
            }

            DDS_DynamicData array_data(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
            DDS_ReturnCode_t status = dynamic_data->bind_complex_member(
              array_data,
              NULL,
              i + 1);
            if (status != DDS_RETCODE_OK) {
              RMW_SET_ERROR_MSG("failed to bind complex member");
              return false;
            }
            size_t array_size = member->size_function(untyped_member);
            for (size_t j = 0; j < array_size; ++j) {
              const void * ros_message;
              {
                const void * sub_ros_message = member->get_const_function(untyped_member, j);
                // offset message pointer since the macro adds the member offset to it
                ros_message = static_cast<const char *>(sub_ros_message) - member->offset_;
              }
              DDS_DynamicData * array_data_ptr = &array_data;
              if (using_introspection_c_typesupport(typesupport)) {
                SET_SUBMESSAGE_VALUE(array_data_ptr, j, INTROSPECTION_C_TYPE)
              } else if (using_introspection_cpp_typesupport(typesupport)) {
                SET_SUBMESSAGE_VALUE(array_data_ptr, j, INTROSPECTION_CPP_TYPE)
              }
            }
            status = dynamic_data->unbind_complex_member(array_data);
            if (status != DDS_RETCODE_OK) {
              RMW_SET_ERROR_MSG("failed to unbind complex member");
              return false;
            }
          } else {
            if (using_introspection_c_typesupport(typesupport)) {
              SET_SUBMESSAGE_VALUE(dynamic_data, i, INTROSPECTION_C_TYPE)
            } else if (using_introspection_cpp_typesupport(typesupport)) {
              SET_SUBMESSAGE_VALUE(dynamic_data, i, INTROSPECTION_CPP_TYPE)
            }
          }
        }
        break;
      default:
        RMW_SET_ERROR_MSG(
          (std::string("unknown type id ") + std::to_string(member->type_id_)).c_str());
        return false;
    }
  }
  return true;
}

bool _publish(DDS_DynamicData * dynamic_data, const void * ros_message,
  const void * untyped_members, const char * typesupport)
{
  if (using_introspection_c_typesupport(typesupport)) {
    return publish<rosidl_typesupport_introspection_c__MessageMembers>(
      dynamic_data, ros_message, untyped_members, typesupport);
  } else if (using_introspection_cpp_typesupport(typesupport)) {
    return publish<rosidl_typesupport_introspection_cpp::MessageMembers>(
      dynamic_data, ros_message, untyped_members, typesupport);
  }

  RMW_SET_ERROR_MSG("Unknown typesupport identifier")
  return false;
}

#define ARRAY_SIZE() \
  size_t array_size; \
  if (member->array_size_ && !member->is_upper_bound_) { \
    array_size = member->array_size_; \
  } else { \
    DDS_DynamicData dynamic_data_member(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT); \
    DDS_ReturnCode_t status = dynamic_data->bind_complex_member( \
      dynamic_data_member, \
      NULL, \
      i + 1); \
    if (status != DDS_RETCODE_OK) { \
      RMW_SET_ERROR_MSG("failed to bind complex member"); \
      return false; \
    } \
    array_size = dynamic_data_member.get_member_count(); \
    status = dynamic_data->unbind_complex_member(dynamic_data_member); \
    if (status != DDS_RETCODE_OK) { \
      RMW_SET_ERROR_MSG("failed to unbind complex member"); \
      return false; \
    } \
  }

#define ARRAY_RESIZE_AND_VALUES(TYPE) \
  TYPE * ros_values = nullptr; \
  if (member->array_size_ && !member->is_upper_bound_) { \
    ros_values = reinterpret_cast<TYPE *>(static_cast<char *>(ros_message) + member->offset_); \
  } else { \
    void * untyped_vector = static_cast<char *>(ros_message) + member->offset_; \
    auto vector = static_cast<std::vector<TYPE> *>(untyped_vector); \
    vector->resize(array_size); \
    ros_values = vector->data(); \
  }

#define GET_VALUE(TYPE, METHOD_NAME, ARRAY_METHOD_NAME) \
  { \
    if (member->is_array_) { \
      ARRAY_SIZE() \
      ARRAY_RESIZE_AND_VALUES(TYPE) \
      DDS_UnsignedLong length = static_cast<DDS_UnsignedLong>(array_size); \
      DDS_ReturnCode_t status = dynamic_data->ARRAY_METHOD_NAME( \
        ros_values, \
        &length, \
        NULL, \
        i + 1); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to get array value using " #ARRAY_METHOD_NAME); \
        return false; \
      } \
    } else { \
      TYPE * value = reinterpret_cast<TYPE *>(static_cast<char *>(ros_message) + member->offset_); \
      DDS_ReturnCode_t status = dynamic_data->METHOD_NAME( \
        *value, \
        NULL, \
        i + 1); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to get primitive value using " #METHOD_NAME); \
        return false; \
      } \
    } \
  }

#define GET_VALUE_WITH_DIFFERENT_TYPES(TYPE, DDS_TYPE, METHOD_NAME, ARRAY_METHOD_NAME) \
  { \
    if (member->is_array_) { \
      ARRAY_SIZE() \
      ARRAY_RESIZE_AND_VALUES(TYPE) \
      if (array_size > 0) { \
        DDS_TYPE * values = \
          static_cast<DDS_TYPE *>(rmw_allocate(sizeof(DDS_TYPE) * array_size)); \
        if (!values) { \
          RMW_SET_ERROR_MSG("failed to allocate memory"); \
          return false; \
        } \
        DDS_UnsignedLong length = static_cast<DDS_UnsignedLong>(array_size); \
        DDS_ReturnCode_t status = dynamic_data->ARRAY_METHOD_NAME( \
          values, \
          &length, \
          NULL, \
          i + 1); \
        if (status != DDS_RETCODE_OK) { \
          rmw_free(values); \
          RMW_SET_ERROR_MSG("failed to get array value using " #ARRAY_METHOD_NAME); \
          return false; \
        } \
        for (size_t i = 0; i < array_size; ++i) { \
          ros_values[i] = values[i]; \
        } \
        rmw_free(values); \
      } \
    } else { \
      DDS_TYPE value; \
      DDS_ReturnCode_t status = dynamic_data->METHOD_NAME( \
        value, \
        NULL, \
        i + 1); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to get primitive value using " #METHOD_NAME); \
        return false; \
      } \
      auto ros_value = \
        reinterpret_cast<TYPE *>(static_cast<char *>(ros_message) + member->offset_); \
      *ros_value = value; \
    } \
  }

#define GET_VALUE_WITH_BOOL_TYPE(TYPE, DDS_TYPE, METHOD_NAME, ARRAY_METHOD_NAME) \
  { \
    if (member->is_array_) { \
      ARRAY_SIZE() \
      if (array_size > 0) { \
        DDS_TYPE * values = \
          static_cast<DDS_TYPE *>(rmw_allocate(sizeof(DDS_TYPE) * array_size)); \
        if (!values) { \
          RMW_SET_ERROR_MSG("failed to allocate memory"); \
          return false; \
        } \
        DDS_UnsignedLong length = static_cast<DDS_UnsignedLong>(array_size); \
        DDS_ReturnCode_t status = dynamic_data->ARRAY_METHOD_NAME( \
          values, \
          &length, \
          NULL, \
          i + 1); \
        if (status != DDS_RETCODE_OK) { \
          rmw_free(values); \
          RMW_SET_ERROR_MSG("failed to get array value using " #ARRAY_METHOD_NAME); \
          return false; \
        } \
        if (member->array_size_ && !member->is_upper_bound_) { \
          auto ros_values = \
            reinterpret_cast<TYPE *>(static_cast<char *>(ros_message) + member->offset_); \
          for (size_t i = 0; i < array_size; ++i) { \
            ros_values[i] = values[i] == DDS_BOOLEAN_TRUE; \
          } \
        } else { \
          void * untyped_vector = static_cast<char *>(ros_message) + member->offset_; \
          auto vector = static_cast<std::vector<TYPE> *>(untyped_vector); \
          vector->resize(array_size); \
          for (size_t i = 0; i < array_size; ++i) { \
            (*vector)[i] = values[i] == DDS_BOOLEAN_TRUE; \
          } \
        } \
        rmw_free(values); \
      } \
    } else { \
      DDS_TYPE value; \
      DDS_ReturnCode_t status = dynamic_data->METHOD_NAME( \
        value, \
        NULL, \
        i + 1); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to get primitive value using " #METHOD_NAME); \
        return false; \
      } \
      auto ros_value = \
        reinterpret_cast<TYPE *>(static_cast<char *>(ros_message) + member->offset_); \
      *ros_value = value == DDS_BOOLEAN_TRUE; \
    } \
  }

#define GET_STRING_VALUE(TYPE, METHOD_NAME, ASSIGN_METHOD) \
  { \
    if (member->is_array_) { \
      ARRAY_SIZE() \
      if (array_size > (std::numeric_limits<DDS_DynamicDataMemberId>::max)()) { \
        RMW_SET_ERROR_MSG( \
          "failed to get string since the requested string length exceeds the DDS type"); \
        return false; \
      } \
      ARRAY_RESIZE_AND_VALUES(TYPE) \
      DDS_DynamicData dynamic_data_member(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT); \
      DDS_ReturnCode_t status = dynamic_data->bind_complex_member( \
        dynamic_data_member, \
        NULL, \
        static_cast<DDS_DynamicDataMemberId>(i + 1)); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to bind complex member"); \
        return false; \
      } \
      for (size_t j = 0; j < array_size; ++j) { \
        char * value = 0; \
        DDS_UnsignedLong size; \
        /* TODO(wjwwood): Figure out how value is allocated. Why are we freeing it? */ \
        status = dynamic_data_member.METHOD_NAME( \
          value, \
          &size, \
          NULL, \
          static_cast<DDS_DynamicDataMemberId>(j + 1)); \
        if (status != DDS_RETCODE_OK) { \
          if (value) { \
            delete[] value; \
          } \
          RMW_SET_ERROR_MSG("failed to get array value using " #METHOD_NAME); \
          return false; \
        } \
        ASSIGN_METHOD(ros_values[j], value); \
        if (value) { \
          delete[] value; \
        } \
      } \
      status = dynamic_data->unbind_complex_member(dynamic_data_member); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to unbind complex member"); \
        return false; \
      } \
    } else { \
      char * value = 0; \
      DDS_UnsignedLong size; \
      DDS_ReturnCode_t status = dynamic_data->METHOD_NAME( \
        value, \
        &size, \
        NULL, \
        static_cast<DDS_DynamicDataMemberId>(i + 1)); \
      if (status != DDS_RETCODE_OK) { \
        if (value) { \
          delete[] value; \
        } \
        RMW_SET_ERROR_MSG("failed to get primitive value using " #METHOD_NAME); \
        return false; \
      } \
      auto ros_value = \
        reinterpret_cast<TYPE *>(static_cast<char *>(ros_message) + member->offset_); \
      ASSIGN_METHOD(*ros_value, value); \
      if (value) { \
        delete[] value; \
      } \
    } \
  }

#define GET_SUBMESSAGE_VALUE(dynamic_data, i) \
  DDS_DynamicData sub_dynamic_data(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT); \
  DDS_ReturnCode_t status = dynamic_data->bind_complex_member( \
    sub_dynamic_data, \
    NULL, \
    static_cast<DDS_DynamicDataMemberId>(i + 1)); \
  if (status != DDS_RETCODE_OK) { \
    RMW_SET_ERROR_MSG("failed to bind complex member"); \
    return false; \
  } \
  void * sub_ros_message = static_cast<char *>(ros_message) + member->offset_; \
  if (!member->members_) { \
    RMW_SET_ERROR_MSG("members handle is null"); \
    return false; \
  } \
  bool success = _take( \
    &sub_dynamic_data, sub_ros_message, member->members_->data, typesupport); \
  status = dynamic_data->unbind_complex_member(sub_dynamic_data); \
  if (!success) { \
    return false; \
  } \
  if (status != DDS_RETCODE_OK) { \
    RMW_SET_ERROR_MSG("failed to unbind complex member"); \
    return false; \
  }

template<typename MembersType>
bool take(DDS_DynamicData * dynamic_data, void * ros_message,
  const void * untyped_members, const char * typesupport)
{
  auto members = static_cast<const MembersType *>(untyped_members);
  if (!members) {
    RMW_SET_ERROR_MSG("members handle is null");
    return false;
  }
  for (uint32_t i = 0; i < members->member_count_; ++i) {
    auto member = members->members_ + i;
    switch (member->type_id_) {
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
        GET_VALUE_WITH_BOOL_TYPE(bool, DDS_Boolean, get_boolean, get_boolean_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
        GET_VALUE(uint8_t, get_octet, get_octet_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
        GET_VALUE(char, get_char, get_char_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
        GET_VALUE(float, get_float, get_float_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
        GET_VALUE(double, get_double, get_double_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
        GET_VALUE_WITH_DIFFERENT_TYPES(int8_t, DDS_Octet, get_octet, get_octet_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
        GET_VALUE(uint8_t, get_octet, get_octet_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
        GET_VALUE(int16_t, get_short, get_short_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
        GET_VALUE(uint16_t, get_ushort, get_ushort_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
        GET_VALUE(int32_t, get_long, get_long_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
        GET_VALUE(uint32_t, get_ulong, get_ulong_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
        GET_VALUE_WITH_DIFFERENT_TYPES(int64_t, DDS_LongLong, get_longlong, get_longlong_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
        GET_VALUE_WITH_DIFFERENT_TYPES(
          uint64_t, DDS_UnsignedLongLong, get_ulonglong, get_ulonglong_array)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
        if (using_introspection_c_typesupport(typesupport)) {
          GET_STRING_VALUE(rosidl_generator_c__String, get_string, C_STRING_ASSIGN)
        } else if (using_introspection_cpp_typesupport(typesupport)) {
          GET_STRING_VALUE(std::string, get_string, CPP_STRING_ASSIGN)
        } else {
          RMW_SET_ERROR_MSG("Unknown typesupport identifier");
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE:
        {
          if (member->is_array_) {
            void * untyped_member = static_cast<char *>(ros_message) + member->offset_;
            if (!member->array_size_ || member->is_upper_bound_) {
              if (!member->resize_function) {
                RMW_SET_ERROR_MSG("resize function handle is null");
                return false;
              }
            }
            if (!member->get_function) {
              RMW_SET_ERROR_MSG("get function handle is null");
              return false;
            }

            ARRAY_SIZE()
            DDS_DynamicData array_data(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
            DDS_ReturnCode_t status = dynamic_data->bind_complex_member(
              array_data,
              NULL,
              i + 1);
            if (status != DDS_RETCODE_OK) {
              RMW_SET_ERROR_MSG("failed to bind complex member");
              return false;
            }
            if (!member->array_size_ || member->is_upper_bound_) {
              member->resize_function(untyped_member, array_size);
            }
            for (size_t j = 0; j < array_size; ++j) {
              void * ros_message;
              {
                void * sub_ros_message = member->get_function(untyped_member, j);
                // offset message pointer since the macro adds the member offset to it
                ros_message = static_cast<char *>(sub_ros_message) - member->offset_;
              }
              DDS_DynamicData * array_data_ptr = &array_data;
              // TODO(dirk-thomas) if the macro return unbind is not called
              GET_SUBMESSAGE_VALUE(array_data_ptr, j)
            }
            status = dynamic_data->unbind_complex_member(array_data);
            if (status != DDS_RETCODE_OK) {
              RMW_SET_ERROR_MSG("failed to unbind complex member");
              return false;
            }
          } else {
            GET_SUBMESSAGE_VALUE(dynamic_data, i)
          }
        }
        break;
      default:
        RMW_SET_ERROR_MSG(
          (std::string("unknown type id ") + std::to_string(member->type_id_)).c_str());
        return false;
    }
  }
  return true;
}

bool _take(DDS_DynamicData * dynamic_data, void * ros_message,
  const void * untyped_members, const char * typesupport)
{
  if (using_introspection_c_typesupport(typesupport)) {
    return take<rosidl_typesupport_introspection_c__MessageMembers>(
      dynamic_data, ros_message, untyped_members, typesupport);
  } else if (using_introspection_cpp_typesupport(typesupport)) {
    return take<rosidl_typesupport_introspection_cpp::MessageMembers>(
      dynamic_data, ros_message, untyped_members, typesupport);
  }
  RMW_SET_ERROR_MSG("Unknown typesupport identifier")
  return false;
}
