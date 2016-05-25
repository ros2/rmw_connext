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

#ifndef RMW_CONNEXT_DYNAMIC_CPP__TEMPLATES_HPP_
#define RMW_CONNEXT_DYNAMIC_CPP__TEMPLATES_HPP_

#include "rosidl_generator_cpp/message_type_support.hpp"

#include "rosidl_generator_c/string.h"
#include "rosidl_generator_c/string_functions.h"

#include "rmw/impl/cpp/macros.hpp"

#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/service_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "rosidl_typesupport_introspection_c/service_introspection.h"
#include "rosidl_typesupport_introspection_c/visibility_control.h"

#include "./macros.hpp"

/********** Utility structs **********/

// This struct is used to associate MessageMember the introspection packages
// with the corresponding MessageMembers struct
template<typename T>
struct GenericMembersT;

template<>
struct GenericMembersT<rosidl_typesupport_introspection_cpp::MessageMember>
{
  using type = rosidl_typesupport_introspection_cpp::MessageMembers;
};

template<>
struct GenericMembersT<rosidl_typesupport_introspection_c__MessageMember>
{
  using type = rosidl_typesupport_introspection_c__MessageMembers;
};

// This struct is used to associate a C array with a struct that we can metaprogram with
template<typename T>
struct GenericCArray;

// multiple definitions of ambiguous primitive types
SPECIALIZE_GENERIC_C_ARRAY(String, rosidl_generator_c__String)
SPECIALIZE_GENERIC_C_ARRAY(bool, bool)
SPECIALIZE_GENERIC_C_ARRAY(byte, uint8_t)
SPECIALIZE_GENERIC_C_ARRAY(char, char)
SPECIALIZE_GENERIC_C_ARRAY(float32, float)
SPECIALIZE_GENERIC_C_ARRAY(float64, double);
SPECIALIZE_GENERIC_C_ARRAY(int8, int8_t)
// TODO(jacquelinekay) this results in an ambiguous definition
// SPECIALIZE_GENERIC_C_ARRAY(uint8, uint8_t)
SPECIALIZE_GENERIC_C_ARRAY(int16, int16_t)
SPECIALIZE_GENERIC_C_ARRAY(uint16, uint16_t)
SPECIALIZE_GENERIC_C_ARRAY(int32, int32_t)
SPECIALIZE_GENERIC_C_ARRAY(uint32, uint32_t)
SPECIALIZE_GENERIC_C_ARRAY(int64, int64_t)
SPECIALIZE_GENERIC_C_ARRAY(uint64, uint64_t)

/********** end utility structs **********/

/********** Map dynamic data function names to types **********/
template<typename T>
DDS_ReturnCode_t set_dynamic_data(DDS_DynamicData * dynamic_data, size_t index, T value);

template<typename T>
DDS_ReturnCode_t set_dynamic_data_array(
  DDS_DynamicData * dynamic_data, size_t index, size_t array_size, T * values);

template<typename T>
DDS_ReturnCode_t get_dynamic_data(DDS_DynamicData * dynamic_data, T value, size_t index);

template<typename T>
DDS_ReturnCode_t get_dynamic_data_array(
  DDS_DynamicData * dynamic_data, T * values, size_t array_size, size_t index);

// TODO(jacquelinekay) this results in an ambiguous definition)
// DEFINE_DYNAMIC_DATA_METHODS(DDS_Boolean, boolean)
DEFINE_DYNAMIC_DATA_METHODS(char, char)
DEFINE_DYNAMIC_DATA_METHODS(float, float)
DEFINE_DYNAMIC_DATA_METHODS(double, double)
DEFINE_DYNAMIC_DATA_METHODS(DDS_Octet, octet)
DEFINE_DYNAMIC_DATA_METHODS(int16_t, short)
DEFINE_DYNAMIC_DATA_METHODS(uint16_t, ushort)
DEFINE_DYNAMIC_DATA_METHODS(int32_t, long)
DEFINE_DYNAMIC_DATA_METHODS(uint32_t, ulong)
DEFINE_DYNAMIC_DATA_METHODS(DDS_LongLong, longlong)
DEFINE_DYNAMIC_DATA_METHODS(DDS_UnsignedLongLong, ulonglong)

DDS_ReturnCode_t set_dynamic_data(
  DDS_DynamicData * dynamic_data, size_t index, const char * value)
{
  return dynamic_data->set_string(NULL, index, value);
}

DDS_ReturnCode_t set_dynamic_data(
  DDS_DynamicData * dynamic_data, size_t index, char * value)
{
  return dynamic_data->set_string(NULL, index, value);
}

DDS_ReturnCode_t get_dynamic_data_string(
  DDS_DynamicData * dynamic_data, char * values, DDS_UnsignedLong * array_size, size_t index)
{
  auto member_id =  static_cast<DDS_DynamicDataMemberId>(index);
  return dynamic_data->get_string(values, array_size, NULL, member_id);
}

/********** end dynamic data functions **********/

/********** general utility functions **********/

template<typename MembersType>
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_LOCAL
inline std::string
_create_type_name(
  const void * untyped_members,
  const std::string & sep)
{
  auto members = static_cast<const MembersType *>(untyped_members);
  if (!members) {
    RMW_SET_ERROR_MSG("members handle is null");
    return "";
  }
  return
    std::string(members->package_name_) + "::" + sep + "::dds_::" + members->message_name_ + "_";
}

template<typename MembersType>
DDS_TypeCode * create_type_code(
  std::string type_name, const void * untyped_members, const char * introspection_identifier)
{
  auto members = static_cast<const MembersType *>(untyped_members);
  if (!members) {
    RMW_SET_ERROR_MSG("members handle is null");
    return NULL;
  }

  DDS_TypeCodeFactory * factory = DDS_TypeCodeFactory::get_instance();
  if (!factory) {
    RMW_SET_ERROR_MSG("failed to get typecode factory");
    return NULL;
  }
  // Past this point, a failure results in unrolling code in the goto fail block.
  DDS_StructMemberSeq struct_members;
  DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
  // Start initializing elements.
  DDS_TypeCode * type_code = factory->create_struct_tc(type_name.c_str(), struct_members, ex);
  if (!type_code || ex != DDS_NO_EXCEPTION_CODE) {
    RMW_SET_ERROR_MSG("failed to create struct typecode");
    goto fail;
  }
  for (uint32_t i = 0; i < members->member_count_; ++i) {
    auto * member = members->members_ + i;
    const DDS_TypeCode * member_type_code = nullptr;
    DDS_TypeCode * member_type_code_non_const = nullptr;
    switch (member->type_id_) {
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
        member_type_code = factory->get_primitive_tc(DDS_TK_BOOLEAN);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
        member_type_code = factory->get_primitive_tc(DDS_TK_OCTET);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
        member_type_code = factory->get_primitive_tc(DDS_TK_CHAR);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
        member_type_code = factory->get_primitive_tc(DDS_TK_FLOAT);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
        member_type_code = factory->get_primitive_tc(DDS_TK_DOUBLE);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
        member_type_code = factory->get_primitive_tc(DDS_TK_OCTET);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
        member_type_code = factory->get_primitive_tc(DDS_TK_OCTET);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
        member_type_code = factory->get_primitive_tc(DDS_TK_SHORT);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
        member_type_code = factory->get_primitive_tc(DDS_TK_USHORT);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
        member_type_code = factory->get_primitive_tc(DDS_TK_LONG);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
        member_type_code = factory->get_primitive_tc(DDS_TK_ULONG);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
        member_type_code = factory->get_primitive_tc(DDS_TK_LONGLONG);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
        member_type_code = factory->get_primitive_tc(DDS_TK_ULONGLONG);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
        {
          DDS_UnsignedLong max_string_size;
          if (member->string_upper_bound_) {
            if (member->string_upper_bound_ > (std::numeric_limits<DDS_UnsignedLong>::max)()) {
              RMW_SET_ERROR_MSG(
                "failed to create string typecode since the upper bound exceeds the DDS type");
              goto fail;
            }
            max_string_size = static_cast<DDS_UnsignedLong>(member->string_upper_bound_);
          } else {
            max_string_size = RTI_INT32_MAX;
          }
          member_type_code_non_const = factory->create_string_tc(max_string_size, ex);
          member_type_code = member_type_code_non_const;
        }
        if (!member_type_code || ex != DDS_NO_EXCEPTION_CODE) {
          RMW_SET_ERROR_MSG("failed to create string typecode");
          goto fail;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE:
        {
          if (!member->members_) {
            RMW_SET_ERROR_MSG("members handle is null");
            return NULL;
          }
          auto sub_members = static_cast<const MembersType *>(member->members_->data);
          if (!sub_members) {
            RMW_SET_ERROR_MSG("sub members handle is null");
            return NULL;
          }
          std::string field_type_name = _create_type_name<MembersType>(sub_members, "msg");
          member_type_code = create_type_code<MembersType>(field_type_name, sub_members,
              introspection_identifier);
          if (!member_type_code) {
            // error string was set within the function
            goto fail;
          }
        }
        break;
      default:
        RMW_SET_ERROR_MSG(
          (std::string("unknown type id ") + std::to_string(member->type_id_)).c_str());
        goto fail;
    }
    if (!member_type_code) {
      RMW_SET_ERROR_MSG("failed to create typecode");
      goto fail;
    }
    if (member->is_array_) {
      if (member->array_size_) {
        if (member->array_size_ > (std::numeric_limits<DDS_UnsignedLong>::max)()) {
          RMW_SET_ERROR_MSG("failed to create array typecode since the size exceeds the DDS type");
          goto fail;
        }
        DDS_UnsignedLong array_size = static_cast<DDS_UnsignedLong>(member->array_size_);
        if (!member->is_upper_bound_) {
          member_type_code_non_const = factory->create_array_tc(array_size, member_type_code, ex);
          member_type_code = member_type_code_non_const;
          if (!member_type_code || ex != DDS_NO_EXCEPTION_CODE) {
            RMW_SET_ERROR_MSG("failed to create array typecode");
            goto fail;
          }
        } else {
          member_type_code_non_const =
            factory->create_sequence_tc(array_size, member_type_code, ex);
          member_type_code = member_type_code_non_const;
          if (!member_type_code || ex != DDS_NO_EXCEPTION_CODE) {
            RMW_SET_ERROR_MSG("failed to create sequence typecode");
            goto fail;
          }
        }
      } else {
        member_type_code_non_const = factory->create_sequence_tc(
          RTI_INT32_MAX, member_type_code, ex);
        member_type_code = member_type_code_non_const;
        if (!member_type_code || ex != DDS_NO_EXCEPTION_CODE) {
          RMW_SET_ERROR_MSG("failed to create sequence typecode");
          goto fail;
        }
      }
    }
    auto zero_based_index = type_code->add_member(
      (std::string(member->name_) + "_").c_str(),
      i,
      member_type_code,
      DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, ex);
    if (ex != DDS_NO_EXCEPTION_CODE) {
      RMW_SET_ERROR_MSG("failed to add member");
      goto fail;
    }
    if (zero_based_index != i) {
      RMW_SET_ERROR_MSG("unexpected member index");
      return NULL;
    }
  }
  // since empty message definitions are not supported
  // we have to add the same dummy field as in rosidl_generator_dds_idl
  if (members->member_count_ == 0) {
    const DDS_TypeCode * member_type_code;
    member_type_code = factory->get_primitive_tc(DDS_TK_BOOLEAN);
    if (!member_type_code) {
      RMW_SET_ERROR_MSG("failed to get primitive typecode");
      goto fail;
    }
    type_code->add_member("_dummy", DDS_TYPECODE_MEMBER_ID_INVALID,
      member_type_code,
      DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, ex);
    if (ex != DDS_NO_EXCEPTION_CODE) {
      RMW_SET_ERROR_MSG("failed to add member");
      goto fail;
    }
  }
  DDS_StructMemberSeq_finalize(&struct_members);
  return type_code;
fail:
  if (type_code) {
    if (factory) {
      DDS_ExceptionCode_t exc = DDS_NO_EXCEPTION_CODE;
      factory->delete_tc(type_code, exc);
      if (exc != DDS_NO_EXCEPTION_CODE) {
        std::stringstream ss;
        ss << "failed to delete type code during handling of failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    } else {
      std::stringstream ss;
      ss << "leaking type code during handling of failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  return nullptr;
}


template<typename ServiceType>
const void * get_request_ptr(const void * untyped_service_members)
{
  auto service_members = static_cast<const ServiceType *>(untyped_service_members);
  if (!service_members) {
    RMW_SET_ERROR_MSG("service members handle is null");
    return NULL;
  }
  return service_members->request_members_;
}

template<typename ServiceType>
const void * get_response_ptr(const void * untyped_service_members)
{
  auto service_members = static_cast<const ServiceType *>(untyped_service_members);
  if (!service_members) {
    RMW_SET_ERROR_MSG("service members handle is null");
    return NULL;
  }
  return service_members->response_members_;
}
/********** end general utility functions **********/

/********** set_*values **********/

template<typename T, typename MessageMemberT>
bool set_primitive_value(
    const void * ros_message,
    const MessageMemberT * member,
    DDS_DynamicData * dynamic_data,
    size_t i)
{
  const T * value =
    reinterpret_cast<const T *>(static_cast<const char *>(ros_message) + member->offset_);
  DDS_ReturnCode_t status = set_dynamic_data(
    dynamic_data,
    i + 1,
    *value);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to set primitive value");
    return false;
  }
  return true;
}

template<typename T, typename MessageMemberT>
size_t set_array_size_and_values(
  const MessageMemberT * member,
  const void * ros_message,
  T * ros_values);

template<typename T, typename std::enable_if<!std::is_same<T, bool>::value>::type * = nullptr>
size_t set_array_size_and_values(
    const rosidl_typesupport_introspection_cpp::MessageMember * member,
    const void * ros_message,
    T * ros_values)
{
  if (member->array_size_ && !member->is_upper_bound_) {
    ros_values =
      const_cast<T *>(reinterpret_cast<const T *>(static_cast<const char *>(ros_message) + member->offset_));
    return member->array_size_;
  }
  const void * untyped_vector = static_cast<const char *>(ros_message) + member->offset_;
  auto output = static_cast<const std::vector<T> *>(untyped_vector);
  ros_values = const_cast<T *>(output->data());
  return output->size();
}

template<>
size_t set_array_size_and_values(
    const rosidl_typesupport_introspection_cpp::MessageMember * member,
    const void * ros_message,
    bool * ros_values)
{
  if (member->array_size_ && !member->is_upper_bound_) {
    ros_values =
      const_cast<bool *>(reinterpret_cast<const bool *>(static_cast<const char *>(ros_message) + member->offset_));
    return member->array_size_;
  }
  const void * untyped_vector = static_cast<const char *>(ros_message) + member->offset_;
  auto output = static_cast<const std::vector<bool> *>(untyped_vector);
  for (size_t i = 0; i < output->size(); ++i) {
    ros_values[i] = (*output)[i];
  }
  return output->size();
}

template<typename T>
size_t set_array_size_and_values(
  const rosidl_typesupport_introspection_c__MessageMember * member,
  const void * ros_message,
  T * ros_values)
{
  if (member->array_size_ && !member->is_upper_bound_) {
    ros_values =
      const_cast<T *>(reinterpret_cast<const T *>(static_cast<const char *>(ros_message) + member->offset_));
    return member->array_size_;
  }
  const void * untyped_array = static_cast<const char *>(ros_message) + member->offset_;
  auto output = static_cast<const typename GenericCArray<T>::type *>(untyped_array);
  ros_values = output->data;
  return output->size;
}

template<typename T, typename MessageMemberT>
bool set_value(
  const MessageMemberT * member,
  const void * ros_message,
  DDS_DynamicData * dynamic_data,
  size_t i)
{
  T * ros_values = nullptr;
  if (member->is_array_) {
    size_t array_size = set_array_size_and_values(member, ros_message, ros_values);
    DDS_ReturnCode_t status = set_dynamic_data_array(
      dynamic_data,
      i + 1,
      static_cast<DDS_UnsignedLong>(array_size),
      ros_values);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to set array value");
      return false;
    }
    return true;
  }
  return set_primitive_value<T>(ros_message, member, dynamic_data, i);
}

// TODO(jacquelinekay): Consider that set_value is a generalization of
// set_value_with_different_types and the two functions could be phrased more elegantly as such
template<typename T, typename DDSType, typename MessageMemberT>
bool set_value_with_different_types(
  const MessageMemberT * member,
  const void * ros_message,
  DDS_DynamicData * dynamic_data,
  size_t i)
{
  T * ros_values = nullptr;
  if (member->is_array_) {
    size_t array_size = set_array_size_and_values(member, ros_message, ros_values);
    DDSType * values = nullptr;
    if (array_size > 0) {
      values = static_cast<DDSType *>(rmw_allocate(sizeof(DDSType) * array_size));
      if (!values) {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        return false;
      }
      for (size_t j = 0; j < array_size; ++j) {
        values[j] = ros_values[j];
      }
    }
    DDS_ReturnCode_t status = set_dynamic_data_array(
      dynamic_data,
      i + 1,
      static_cast<DDS_UnsignedLong>(array_size),
      values);
    rmw_free(values);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to set array value");
      return false;
    }
  } else {
    set_primitive_value<DDSType>(ros_message, member, dynamic_data, i);
  }
  return true;
}

template<>
bool set_value<std::string>(
  const rosidl_typesupport_introspection_cpp::MessageMember * member,
  const void * ros_message,
  DDS_DynamicData * dynamic_data,
  size_t i)
{
  std::string * ros_values = nullptr;
  if (member->is_array_) {
    DDS_DynamicData dynamic_data_member(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    DDS_ReturnCode_t status = dynamic_data->bind_complex_member(
      dynamic_data_member,
      NULL,
      static_cast<DDS_DynamicDataMemberId>(i + 1));
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to bind complex member");
      return false;
    }
    size_t array_size = set_array_size_and_values(member, ros_message, ros_values);
    if (array_size > (std::numeric_limits<DDS_DynamicDataMemberId>::max)()) {
      RMW_SET_ERROR_MSG(
        "failed to set string since the requested string length exceeds the DDS type");
      return false;
    }
    for (size_t j = 0; j < array_size; ++j) {
      status = set_dynamic_data(
        dynamic_data,
        static_cast<DDS_DynamicDataMemberId>(j + 1),
        ros_values[j].c_str());
      if (status != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to set array value");
        return false;
      }
    }
    status = dynamic_data->unbind_complex_member(dynamic_data_member);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to unbind complex member");
      return false;
    }
  } else {
    const std::string * value =
      reinterpret_cast<const std::string *>(static_cast<const char *>(ros_message) + member->offset_);
    if (!value) {
      RMW_SET_ERROR_MSG("failed to cast string");
      return false;
    }
    if (!value->c_str()) {
      return false;
    }
    DDS_ReturnCode_t status = set_dynamic_data(
      dynamic_data,
      static_cast<DDS_DynamicDataMemberId>(i + 1),
      value->c_str());
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to set value");
      return false;
    }
  }
  return true;
}

template<>
bool set_value<rosidl_generator_c__String>(
  const rosidl_typesupport_introspection_c__MessageMember * member,
  const void * ros_message,
  DDS_DynamicData * dynamic_data,
  size_t i)
{
  rosidl_generator_c__String * ros_values = nullptr;
  if (member->is_array_) {
    DDS_DynamicData dynamic_data_member(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    DDS_ReturnCode_t status = dynamic_data->bind_complex_member(
      dynamic_data_member,
      NULL,
      static_cast<DDS_DynamicDataMemberId>(i + 1));
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to bind complex member");
      return false;
    }
    size_t array_size = set_array_size_and_values(member, ros_message, ros_values);
    if (array_size > (std::numeric_limits<DDS_DynamicDataMemberId>::max)()) {
      RMW_SET_ERROR_MSG(
        "failed to set string since the requested string length exceeds the DDS type");
      return false;
    }
    for (size_t j = 0; j < array_size; ++j) {
      status = set_dynamic_data(
        dynamic_data,
        static_cast<DDS_DynamicDataMemberId>(j + 1),
        ros_values[j].data);
      if (status != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to set array value");
        return false;
      }
    }
    status = dynamic_data->unbind_complex_member(dynamic_data_member);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to unbind complex member");
      return false;
    }
  } else {
    const rosidl_generator_c__String * value =
      reinterpret_cast<const rosidl_generator_c__String *>(
        static_cast<const char *>(ros_message) + member->offset_);
    if (!value) {
      RMW_SET_ERROR_MSG("failed to cast string");
      return false;
    }
    if (!value->data) {
      return false;
    }
    if (value->data[0] == '\0') {
      return false;
    }
    DDS_ReturnCode_t status = set_dynamic_data(
      dynamic_data,
      static_cast<DDS_DynamicDataMemberId>(i + 1),
      value->data);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to set value");
      return false;
    }
  }
  return true;
}

template<
  typename MembersType,
  typename StringType = typename std::conditional<
    std::is_same<MembersType, rosidl_typesupport_introspection_c__MessageMembers>::value,
    rosidl_generator_c__String, std::string
  >::type>
bool publish(
  DDS_DynamicData * dynamic_data, const void * ros_message,
  const void * untyped_members)
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
        if (!set_value_with_different_types<bool, DDS_Boolean>(
          member, ros_message, dynamic_data, i))
        {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
        if (!set_value<uint8_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
        if (!set_value<char>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
        if (!set_value<float>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
        if (!set_value<double>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
        if (!set_value_with_different_types<int8_t, DDS_Octet>(
          member, ros_message, dynamic_data, i))
        {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
        if (!set_value<uint8_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
        if (!set_value<int16_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
        if (!set_value<uint16_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
        if (!set_value<int32_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
        if (!set_value<uint32_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
        if (!set_value_with_different_types<int64_t, DDS_LongLong>(
          member, ros_message, dynamic_data, i))
        {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
        if (!set_value_with_different_types<uint64_t, DDS_UnsignedLongLong>(
          member, ros_message, dynamic_data, i))
        {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
        if (!set_value<StringType>(member, ros_message, dynamic_data, i)) {
          return false;
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

              if (!set_submessage_value(member, ros_message, array_data_ptr, j)) {
                return false;
              }
            }
            status = dynamic_data->unbind_complex_member(array_data);
            if (status != DDS_RETCODE_OK) {
              RMW_SET_ERROR_MSG("failed to unbind complex member");
              return false;
            }
          } else {
            if (!set_submessage_value(member, ros_message, dynamic_data, i)) {
              return false;
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

template<typename MessageMemberT>
bool set_submessage_value(
  const MessageMemberT * member,
  const void * ros_message,
  DDS_DynamicData * dynamic_data,
  size_t i)
{
  using MembersT = typename GenericMembersT<MessageMemberT>::type;
  DDS_DynamicData sub_dynamic_data(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
  DDS_ReturnCode_t status = dynamic_data->bind_complex_member(
    sub_dynamic_data,
    NULL,
    static_cast<DDS_DynamicDataMemberId>(i + 1));
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to bind complex member");
    return false;
  }
  const void * sub_ros_message = static_cast<const char *>(ros_message) + member->offset_;
  if (!member->members_) {
    RMW_SET_ERROR_MSG("members handle is null");
    return false;
  }
  bool published = publish<MembersT>(
    &sub_dynamic_data, sub_ros_message, member->members_->data);
  if (!published) {
    DDS_UnsignedLong count = sub_dynamic_data.get_member_count();
    for (DDS_UnsignedLong k = 0; k < count; ++k) {
      DDS_DynamicDataMemberInfo info;
      status = sub_dynamic_data.get_member_info_by_index(info, k);
      if (status != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to get member info");
        return false;
      }
    }
  }
  status = dynamic_data->unbind_complex_member(sub_dynamic_data);
  if (!published) {
    RMW_SET_ERROR_MSG("failed to publish sub message");
    return false;
  }
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to unbind complex member");
    return false;
  }
  return true;
}

/********** end set_*values functions **********/

/********** get_*values functions **********/

template<typename MessageMemberT>
bool get_array_size(const MessageMemberT * member, size_t & array_size, DDS_DynamicData * dynamic_data, size_t i)
{
  if (member->array_size_ && !member->is_upper_bound_) {
    array_size = member->array_size_;
  } else {
    DDS_DynamicData dynamic_data_member(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    DDS_ReturnCode_t status = dynamic_data->bind_complex_member(
      dynamic_data_member,
      NULL,
      i + 1);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to bind complex member");
      return false;
    }
    array_size = dynamic_data_member.get_member_count();
    status = dynamic_data->unbind_complex_member(dynamic_data_member);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to unbind complex member");
      return false;
    }
  }
  return true;
}

template<typename T, typename MessageMemberT>
void resize_array_and_get_values(
    T * ros_values,
    void * ros_message,
    const MessageMemberT * member,
    size_t array_size);

template<typename T, typename std::enable_if<!std::is_same<T, bool>::value>::type * = nullptr>
void resize_array_and_get_values(
    T * ros_values,
    void * ros_message,
    const rosidl_typesupport_introspection_cpp::MessageMember * member,
    size_t array_size)
{
  if (member->array_size_ && !member->is_upper_bound_) {
    ros_values = reinterpret_cast<T *>(static_cast<char *>(ros_message) + member->offset_);
  } else {
    void * untyped_vector = static_cast<char *>(ros_message) + member->offset_;
    auto vector = static_cast<std::vector<T> *>(untyped_vector);
    vector->resize(array_size);
    ros_values = vector->data();
  }
}

template<>
void resize_array_and_get_values(
    bool * ros_values,
    void * ros_message,
    const rosidl_typesupport_introspection_cpp::MessageMember * member,
    size_t array_size)
{
  if (member->array_size_ && !member->is_upper_bound_) {
    ros_values = reinterpret_cast<bool *>(static_cast<char *>(ros_message) + member->offset_);
  } else {
    void * untyped_vector = static_cast<char *>(ros_message) + member->offset_;
    auto vector = static_cast<std::vector<bool> *>(untyped_vector);
    vector->resize(array_size);
    for (size_t i = 0; i < vector->size(); ++i) {
      ros_values[i] = (*vector)[i];
    }
  }
}

template<typename T>
void resize_array_and_get_values(
    T * ros_values,
    void * ros_message,
    const rosidl_typesupport_introspection_c__MessageMember * member,
    size_t array_size)
{
  if (member->array_size_ && !member->is_upper_bound_) {
    ros_values = reinterpret_cast<T *>(static_cast<char *>(ros_message) + member->offset_);
  } else {
    // TODO deallocation
    auto output = static_cast<const typename GenericCArray<T>::type *>(ros_message);

    T * resized = static_cast<T *>(rmw_allocate(sizeof(T) * array_size));
    memcpy(resized, output->data, output->size);

    ros_values = resized;
  }
}

template<typename T, typename MessageMemberT>
bool get_value(
  const MessageMemberT * member,
  void * ros_message,
  DDS_DynamicData * dynamic_data,
  size_t i)
{
  T * ros_values = nullptr;
  if (member->is_array_) {
    size_t array_size;
    if (!get_array_size(member, array_size, dynamic_data, i)) {
      return false;
    }

    resize_array_and_get_values(ros_values, ros_message, member, array_size);

    DDS_ReturnCode_t status = get_dynamic_data_array(
      dynamic_data,
      ros_values,
      array_size,
      i + 1);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to get array value");
      return false;
    }
  } else {
    T * value = reinterpret_cast<T *>(static_cast<char *>(ros_message) + member->offset_);
    DDS_ReturnCode_t status = get_dynamic_data(
      dynamic_data,
      *value,
      i + 1);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to get primitive value");
      return false;
    }
  }
  return true;
}

template<typename T, typename DDSType>
T primitive_convert_from_dds(DDSType value)
{
  return value;
}

template<>
bool primitive_convert_from_dds(DDS_Boolean value)
{
  return value == DDS_BOOLEAN_TRUE;
}

template<typename T, typename DDSType, typename MessageMemberT>
bool get_value_with_different_types(
  const MessageMemberT * member,
  void * ros_message,
  DDS_DynamicData * dynamic_data,
  size_t i)
{
  T * ros_values = nullptr;
  if (member->is_array_) {
    size_t array_size;
    if (!get_array_size(member, array_size, dynamic_data, i)) {
      return false;
    }
    resize_array_and_get_values(ros_values, ros_message, member, array_size);

    if (array_size > 0) {
      DDSType * values =
        static_cast<DDSType *>(rmw_allocate(sizeof(DDSType) * array_size));
      if (!values) {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        return false;
      }
      DDS_ReturnCode_t status = get_dynamic_data_array(
        dynamic_data,
        values,
        array_size,
        i + 1);
      if (status != DDS_RETCODE_OK) {
        rmw_free(values);
        RMW_SET_ERROR_MSG("failed to get array value");
        return false;
      }
      for (size_t i = 0; i < array_size; ++i) {
        ros_values[i] = primitive_convert_from_dds<T, DDSType>(values[i]);
      }
      rmw_free(values);
    }
  } else {
    DDSType value = 0;
    DDS_ReturnCode_t status = get_dynamic_data(
      dynamic_data,
      value,
      i + 1);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to get primitive value");
      return false;
    }
    auto ros_value =
      reinterpret_cast<T *>(static_cast<char *>(ros_message) + member->offset_);
    *ros_value = primitive_convert_from_dds<T, DDSType>(value);
  }
  return true;
}

template<typename T, typename U>
void string_assign(T dst, U src);

template<>
void string_assign(rosidl_generator_c__String * dst, char * src)
{
  rosidl_generator_c__String__assign(dst, src);
}

template<>
void string_assign(std::string * dst, char * src)
{
  *dst = src;
}

template<typename T, typename MessageMemberT>
bool get_string_value(
  const MessageMemberT * member,
  void * ros_message,
  DDS_DynamicData * dynamic_data,
  size_t i)
{
  T * ros_values = nullptr;
  if (member->is_array_) {
    size_t array_size;
    if (!get_array_size(member, array_size, dynamic_data, i)) {
      return false;
    }
    if (array_size > (std::numeric_limits<DDS_DynamicDataMemberId>::max)()) {
      RMW_SET_ERROR_MSG(
        "failed to get string since the requested string length exceeds the DDS type");
      return false;
    }
    resize_array_and_get_values(ros_values, ros_message, member, array_size);

    DDS_DynamicData dynamic_data_member(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    DDS_ReturnCode_t status = dynamic_data->bind_complex_member(
      dynamic_data_member,
      NULL,
      static_cast<DDS_DynamicDataMemberId>(i + 1));
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to bind complex member");
      return false;
    }
    for (size_t j = 0; j < array_size; ++j) {
      char * value = 0;
      DDS_UnsignedLong size;
      /* TODO(wjwwood): Figure out how value is allocated. Why are we freeing it? */

      status = get_dynamic_data_string(
        dynamic_data,
        value,
        &size,
        j + 1);
      if (status != DDS_RETCODE_OK) {
        if (value) {
          delete[] value;
        }
        RMW_SET_ERROR_MSG("failed to get array value");
        return false;
      }
      string_assign(&ros_values[j], value);
      if (value) {
        delete[] value;
      }
    }
    status = dynamic_data->unbind_complex_member(dynamic_data_member);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to unbind complex member");
      return false;
    }
  } else {
    char * value = 0;
    DDS_UnsignedLong size;
    DDS_ReturnCode_t status = get_dynamic_data_string(
      dynamic_data,
      value,
      &size,
      static_cast<DDS_DynamicDataMemberId>(i + 1));
    if (status != DDS_RETCODE_OK) {
      if (value) {
        delete[] value;
      }
      RMW_SET_ERROR_MSG("failed to get primitive value");
      return false;
    }
    auto ros_value =
      reinterpret_cast<T *>(static_cast<char *>(ros_message) + member->offset_);
    string_assign(ros_value, value);
    if (value) {
      delete[] value;
    }
  }
  return true;
}

template<
  typename MembersType,
  typename StringType = typename std::conditional<
    std::is_same<MembersType, rosidl_typesupport_introspection_c__MessageMembers>::value,
    rosidl_generator_c__String, std::string
  >::type>
bool take(DDS_DynamicData * dynamic_data, void * ros_message,
  const void * untyped_members)
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
        if (!get_value_with_different_types<bool, DDS_Boolean>(
            member, ros_message, dynamic_data, i))
        {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
        if (!get_value<uint8_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
        if (!get_value<char>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
        if (!get_value<float>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
        if (!get_value<double>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
        if (!get_value_with_different_types<int8_t, DDS_Octet>(
          member, ros_message, dynamic_data, i))
        {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
        if (!get_value<uint8_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
        if (!get_value<int16_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
        if (!get_value<uint16_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
        if (!get_value<int32_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
        if (!get_value<uint32_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
        if (!get_value_with_different_types<int64_t, DDS_LongLong>(
          member, ros_message, dynamic_data, i))
        {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
        if (!get_value_with_different_types<uint64_t, DDS_UnsignedLongLong>(
          member, ros_message, dynamic_data, i))
        {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
        if (!get_string_value<StringType>(member, ros_message, dynamic_data, i)) {
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

            size_t array_size;
            if (!get_array_size(member, array_size, dynamic_data, i)) {
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
            if (!member->array_size_ || member->is_upper_bound_) {
              member->resize_function(untyped_member, array_size);
            }
            bool errored = false;
            for (size_t j = 0; j < array_size; ++j) {
              void * ros_message;
              {
                void * sub_ros_message = member->get_function(untyped_member, j);
                // offset message pointer since the macro adds the member offset to it
                ros_message = static_cast<char *>(sub_ros_message) - member->offset_;
              }
              DDS_DynamicData * array_data_ptr = &array_data;
              if (!get_submessage_value(member, ros_message, array_data_ptr, j)) {
                break;
              }
            }
            status = dynamic_data->unbind_complex_member(array_data);
            if (status != DDS_RETCODE_OK) {
              RMW_SET_ERROR_MSG("failed to unbind complex member");
              return false;
            }
            if (errored) {
              RMW_SET_ERROR_MSG("get_submessage_value failed");
              return false;
            }
          } else {
            if (!get_submessage_value(member, ros_message, dynamic_data, i)) {
              return false;
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

template<typename MessageMemberT>
bool get_submessage_value(
  const MessageMemberT * member,
  void * ros_message,
  DDS_DynamicData * dynamic_data,
  size_t i)
{
  using MembersT = typename GenericMembersT<MessageMemberT>::type;
  DDS_DynamicData sub_dynamic_data(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
  DDS_ReturnCode_t status = dynamic_data->bind_complex_member(
    sub_dynamic_data,
    NULL,
    static_cast<DDS_DynamicDataMemberId>(i + 1));
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to bind complex member");
    return false;
  }
  void * sub_ros_message = static_cast<char *>(ros_message) + member->offset_;
  if (!member->members_) {
    RMW_SET_ERROR_MSG("members handle is null");
    return false;
  }
  bool success = take<MembersT>(
    &sub_dynamic_data, sub_ros_message, member->members_->data);
  status = dynamic_data->unbind_complex_member(sub_dynamic_data);
  if (!success) {
    return false;
  }
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to unbind complex member");
    return false;
  }
  return true;
}

/********** end get_*values functions **********/

#endif  // RMW_CONNEXT_DYNAMIC_CPP__TEMPLATES_HPP_

