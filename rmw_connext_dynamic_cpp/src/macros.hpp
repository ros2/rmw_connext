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

#define DEFINE_DYNAMIC_DATA_METHODS(TYPE, METHOD_TYPE) \
  template<> \
  DDS_ReturnCode_t set_dynamic_data( \
    DDS_DynamicData * dynamic_data, size_t index, TYPE value) \
  { \
    return dynamic_data->set_ ## METHOD_TYPE(NULL, index, value); \
  } \
  template<> \
  DDS_ReturnCode_t set_dynamic_data_array( \
    DDS_DynamicData * dynamic_data, size_t index, size_t array_size, TYPE * values) \
  { \
    return dynamic_data->set_ ## METHOD_TYPE ## _array(NULL, index, \
      static_cast<DDS_UnsignedLong>(array_size), values); \
  } \
  template<> \
  DDS_ReturnCode_t get_dynamic_data( \
    DDS_DynamicData * dynamic_data, TYPE value, size_t index) \
  { \
    return dynamic_data->get_ ## METHOD_TYPE( \
      value, \
      NULL, \
      static_cast<DDS_DynamicDataMemberId>(index)); \
  } \
  template<> \
  DDS_ReturnCode_t get_dynamic_data_array( \
    DDS_DynamicData * dynamic_data, TYPE * values, size_t array_size, size_t index) \
  { \
    DDS_UnsignedLong length = static_cast<DDS_UnsignedLong>(array_size); \
    return dynamic_data->get_ ## METHOD_TYPE ## _array( \
      values, \
      &length, \
      NULL, \
      index); \
  } \


#define SPECIALIZE_GENERIC_C_ARRAY(C_TYPE) \
template<> \
struct GenericCArray<C_TYPE> \
{ \
using type = C_TYPE ## __Array;\
};



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
      if (!value) { \
        RMW_SET_ERROR_MSG("failed to cast string"); \
        return false; \
      } \
      if (!value->ACCESSOR) { \
        continue; \
      } \
      /* perhaps this should conditionally depend on C typesupport */\
      if (value->ACCESSOR[0] == '\0') { \
        continue; \
      } \
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


#endif  // MACROS_HPP_
