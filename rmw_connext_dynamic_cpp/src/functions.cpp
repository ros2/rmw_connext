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

#ifdef Connext_GLIBCXX_USE_CXX11_ABI_ZERO
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

#include <cassert>
#include <exception>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Winfinite-recursion"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif
#include <ndds/connext_cpp/connext_cpp_replier_details.h>
#include <ndds/connext_cpp/connext_cpp_requester_details.h>
#include <ndds/ndds_cpp.h>
#include <ndds/ndds_requestreply_cpp.h>
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/types.h"
// This header is in the rosidl_typesupport_connext_cpp package and
// is in the include/rosidl_typesupport_connext_cpp/impl folder.
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

#include "rmw_connext_shared_cpp/shared_functions.hpp"
#include "rmw_connext_shared_cpp/types.hpp"

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

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_LOCAL
inline std::string
_create_type_name(
  const void * untyped_members,
  const std::string & sep,
  const char * typesupport)
{
  if (using_introspection_c_typesupport(typesupport)) {
    return _create_type_name<rosidl_typesupport_introspection_c__MessageMembers>(untyped_members,
             sep);
  } else if (using_introspection_cpp_typesupport(typesupport)) {
    return _create_type_name<rosidl_typesupport_introspection_cpp::MessageMembers>(untyped_members,
             sep);
  }
  RMW_SET_ERROR_MSG("Unknown typesupport identifier");
  return "";
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

const void * get_request_ptr(const void * untyped_service_members, const char * typesupport)
{
  if (using_introspection_c_typesupport(typesupport)) {
    return get_request_ptr<rosidl_typesupport_introspection_c__ServiceMembers>(
      untyped_service_members);
  } else if (using_introspection_cpp_typesupport(typesupport)) {
    return get_request_ptr<rosidl_typesupport_introspection_cpp::ServiceMembers>(
      untyped_service_members);
  }
  RMW_SET_ERROR_MSG("Unknown typesupport identifier");
  return NULL;
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

const void * get_response_ptr(const void * untyped_service_members, const char * typesupport)
{
  if (using_introspection_c_typesupport(typesupport)) {
    return get_response_ptr<rosidl_typesupport_introspection_c__ServiceMembers>(
      untyped_service_members);
  } else if (using_introspection_cpp_typesupport(typesupport)) {
    return get_response_ptr<rosidl_typesupport_introspection_cpp::ServiceMembers>(
      untyped_service_members);
  }
  RMW_SET_ERROR_MSG("Unknown typesupport identifier");
  return NULL;
}

// Mapping dynamic data function names to types
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



DDS_ReturnCode_t get_dynamic_data_string(
  DDS_DynamicData * dynamic_data, char * values, size_t array_size, size_t index)
{
  DDS_UnsignedLong length = static_cast<DDS_UnsignedLong>(array_size);
  return dynamic_data->get_string(
    values,
    &length,
    NULL,
    index);
}

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
    // TODO Method name reflection...
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

// C++-style arrays
// TODO enable if with C++ style
// TODO Better naming
template<typename T, typename MessageMemberT>
size_t set_array_size_and_values(
    const rosidl_typesupport_introspection_cpp::MessageMember * member,
    const void * ros_message,
    const T * ros_values)
{
  if (member->array_size_ && !member->is_upper_bound_) {
    ros_values =
      reinterpret_cast<const T *>(static_cast<const char *>(ros_message) + member->offset_);
    return member->array_size_;
  }
  const void * untyped_vector = static_cast<const char *>(ros_message) + member->offset_;
  auto output = static_cast<const std::vector<T> *>(untyped_vector);
  ros_values = output->data();
  return output->size();
}

template<typename T>
struct GenericCArray;

SPECIALIZE_GENERIC_C_ARRAY(rosidl_generator_c__String)

// ArrayT has to be a rosidl_generator_c-style array
template<typename T>
size_t set_array_size_and_values(
  const rosidl_typesupport_introspection_c__MessageMember * member,
  const void * ros_message,
  const T * ros_values)
{
  if (member->array_size_ && !member->is_upper_bound_) {
    ros_values =
      reinterpret_cast<const T *>(static_cast<const char *>(ros_message) + member->offset_);
    return member->array_size_;
  }
  const void * untyped_array = static_cast<const char *>(ros_message) + member->offset_;
  auto output = static_cast<const typename GenericCArray<T>::type *>(untyped_array);
  ros_values = output->data;
  return output->size;
}

// Input to all set* functions is:
//  DDS_DynamicData * dynamic_data, const void * ros_message,
//  const void * untyped_members, const char * typesupport


// TODO Infer the function types and values from the types through a static map
// TODO bind set_array_method to dynamic_data
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
    set_primitive_value<T>(ros_message, member, dynamic_data, i);
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
        set_value_with_different_types<bool, DDS_Boolean>(member, ros_message, dynamic_data, i);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
        set_value<uint8_t>(member, ros_message, dynamic_data, i);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
        set_value<char>(member, ros_message, dynamic_data, i);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
        set_value<float>(member, ros_message, dynamic_data, i);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
        set_value<double>(member, ros_message, dynamic_data, i);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
        set_value_with_different_types<int8_t, DDS_Octet>(member, ros_message, dynamic_data, i);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
        set_value<uint8_t>(member, ros_message, dynamic_data, i);
        break;
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
        set_value<int16_t>(member, ros_message, dynamic_data, i);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
        set_value<uint16_t>(member, ros_message, dynamic_data, i);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
        set_value<int32_t>(member, ros_message, dynamic_data, i);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
        set_value<uint32_t>(member, ros_message, dynamic_data, i);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
        set_value_with_different_types<int64_t, DDS_LongLong>(member, ros_message, dynamic_data, i);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
        set_value_with_different_types<uint64_t, DDS_UnsignedLongLong>(member, ros_message, dynamic_data, i);
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
        if (using_introspection_c_typesupport(typesupport)) {
          set_value<rosidl_generator_c__String>(member, ros_message, dynamic_data, i);
        } else if (using_introspection_cpp_typesupport(typesupport)) {
          set_value<std::string>(member, ros_message, dynamic_data, i);
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

              if (!set_submessage_value(member, ros_message, array_data_ptr, j, typesupport)) {
                return false;
              }
            }
            status = dynamic_data->unbind_complex_member(array_data);
            if (status != DDS_RETCODE_OK) {
              RMW_SET_ERROR_MSG("failed to unbind complex member");
              return false;
            }
          } else {
            if (!set_submessage_value(member, ros_message, dynamic_data, i, typesupport)) {
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



template<typename MessageMemberT>
bool set_submessage_value(
  const MessageMemberT * member,
  const void * ros_message,
  DDS_DynamicData * dynamic_data,
  size_t i,
  const char * typesupport)
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
    &sub_dynamic_data, sub_ros_message, member->members_->data, typesupport);
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
    MessageMemberT * member,
    size_t array_size);

template<typename T>
void resize_array_and_get_values(
    T * ros_values,
    void * ros_message,
    rosidl_typesupport_introspection_cpp::MessageMember * member,
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

template<typename T>
void resize_array_and_get_values(
    T * ros_values,
    void * ros_message,
    rosidl_typesupport_introspection_c__MessageMember * member,
    size_t array_size)
{
  if (member->array_size_ && !member->is_upper_bound_) { \
    ros_values = reinterpret_cast<T *>(static_cast<char *>(ros_message) + member->offset_); \
  } else { \
    // Need vector copy?

    auto output = static_cast<const typename GenericCArray<T>::type *>(ros_message);
    ros_values = output->data;
    return output->size;

    void * untyped_vector = static_cast<char *>(ros_message) + member->offset_; \
    auto vector = static_cast<std::vector<T> *>(untyped_vector); \
    vector->resize(array_size); \
    ros_values = vector->data(); \
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

    DDS_UnsignedLong length = static_cast<DDS_UnsignedLong>(array_size);
    DDS_ReturnCode_t status = get_dynamic_data_array(
      dynamic_data,
      ros_values,
      length,
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
      DDS_UnsignedLong length = static_cast<DDS_UnsignedLong>(array_size);
      DDS_ReturnCode_t status = get_dynamic_data_array(
        dynamic_data,
        values,
        length,
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
void string_assign(rosidl_generator_c__String * dst, const char * src)
{
  rosidl_generator_c__String__assign(dst, src);
}

template<>
void string_assign(std::string * dst, const char * src)
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
        static_cast<DDS_DynamicDataMemberId>(j + 1));
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
}

// TODO Don't pass typesupport string and instead infer from MessageMemberT
template<typename MessageMemberT>
bool get_submessage_value(
  MessageMemberT * member,
  void * ros_message,
  DDS_DynamicData * dynamic_data,
  size_t i, const char * typesupport)
{
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
  bool success = _take(
    &sub_dynamic_data, sub_ros_message, member->members_->data, typesupport);
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
        if (!get_value_with_different_types<bool, DDS_Boolean>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
        if (!get_value<uint8_t>(member, ros_message, dynamic_data, i)) {
          return false;
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
        //GET_VALUE(char, get_char, get_char_array)
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
        if (using_introspection_c_typesupport(typesupport)) {
          if (!get_value<rosidl_generator_c__String>(member, ros_message, dynamic_data, i)) {
            return false;
          }
        } else if (using_introspection_cpp_typesupport(typesupport)) {
          if (!get_value<std::string>(member, ros_message, dynamic_data, i)) {
            return false;
          }
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
            for (size_t j = 0; j < array_size; ++j) {
              void * ros_message;
              {
                void * sub_ros_message = member->get_function(untyped_member, j);
                // offset message pointer since the macro adds the member offset to it
                ros_message = static_cast<char *>(sub_ros_message) - member->offset_;
              }
              DDS_DynamicData * array_data_ptr = &array_data;
              if (!get_submessage_value(member, ros_message, array_data_ptr, j, typesupport)) {
                break;
              }
            }
            status = dynamic_data->unbind_complex_member(array_data);
            if (status != DDS_RETCODE_OK) {
              RMW_SET_ERROR_MSG("failed to unbind complex member");
              return false;
            }
          } else {
            get_submessage_value(member, ros_message, dynamic_data, i, typesupport);
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




// This extern "C" prevents accidental overloading of functions. With this in
// place, overloading produces an error rather than a new C++ symbol.
extern "C"
{
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_EXPORT
const char * rti_connext_dynamic_identifier = "rmw_connext_dynamic_cpp";

struct CustomPublisherInfo
{
  const char * typesupport_identifier;
  DDSDynamicDataTypeSupport * dynamic_data_type_support_;
  DDSPublisher * dds_publisher_;
  DDSDataWriter * data_writer_;
  DDSDynamicDataWriter * dynamic_writer_;
  DDS_TypeCode * type_code_;
  const void * untyped_members_;
  DDS_DynamicData * dynamic_data;
  rmw_gid_t publisher_gid;
};

struct CustomSubscriberInfo
{
  const char * typesupport_identifier;
  DDSDynamicDataTypeSupport * dynamic_data_type_support_;
  DDSDynamicDataReader * dynamic_reader_;
  DDSDataReader * data_reader_;
  DDSReadCondition * read_condition_;
  DDSSubscriber * dds_subscriber_;
  bool ignore_local_publications;
  DDS_TypeCode * type_code_;
  const void * untyped_members_;
  DDS_DynamicData * dynamic_data;
};

struct ConnextDynamicServiceInfo
{
  const char * typesupport_identifier;
  connext::Replier<DDS_DynamicData, DDS_DynamicData> * replier_;
  DDSDataReader * request_datareader_;
  DDSReadCondition * read_condition_;
  DDS::DynamicDataTypeSupport * request_type_support_;
  DDS::DynamicDataTypeSupport * response_type_support_;
  DDS_TypeCode * response_type_code_;
  DDS_TypeCode * request_type_code_;
  const void * untyped_request_members_;
  const void * untyped_response_members_;
};

struct ConnextDynamicClientInfo
{
  const char * typesupport_identifier;
  connext::Requester<DDS_DynamicData, DDS_DynamicData> * requester_;
  DDSDataReader * response_datareader_;
  DDSReadCondition * read_condition_;
  DDS::DynamicDataTypeSupport * request_type_support_;
  DDS::DynamicDataTypeSupport * response_type_support_;
  DDS_TypeCode * response_type_code_;
  DDS_TypeCode * request_type_code_;
  const void * untyped_request_members_;
  const void * untyped_response_members_;
};


const char *
rmw_get_implementation_identifier()
{
  return rti_connext_dynamic_identifier;
}

rmw_ret_t
rmw_init()
{
  return init();
}

rmw_node_t *
rmw_create_node(const char * name, size_t domain_id)
{
  return create_node(rti_connext_dynamic_identifier, name, domain_id);
}

rmw_ret_t
rmw_destroy_node(rmw_node_t * node)
{
  return destroy_node(rti_connext_dynamic_identifier, node);
}

rmw_ret_t
destroy_type_code(DDS_TypeCode * type_code)
{
  DDS_TypeCodeFactory * factory = NULL;
  factory = DDS_TypeCodeFactory::get_instance();
  if (!factory) {
    RMW_SET_ERROR_MSG("failed to get typecode factory");
    return RMW_RET_ERROR;
  }

  DDS_ExceptionCode_t ex;
  factory->delete_tc(type_code, ex);
  if (ex != DDS_NO_EXCEPTION_CODE) {
    RMW_SET_ERROR_MSG("failed to delete type code struct");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}


DDS_TypeCode * _create_type_code(
  std::string type_name, const void * untyped_members, const char * typesupport)
{
  if (using_introspection_c_typesupport(typesupport)) {
    return create_type_code<rosidl_typesupport_introspection_c__MessageMembers>(
      type_name, untyped_members, typesupport);
  } else if (using_introspection_cpp_typesupport(typesupport)) {
    return create_type_code<rosidl_typesupport_introspection_cpp::MessageMembers>(
      type_name, untyped_members, typesupport);
  }

  RMW_SET_ERROR_MSG("Unknown typesupport identifier")
  return NULL;
}

rmw_publisher_t *
rmw_create_publisher(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t * qos_profile)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_dynamic_identifier,
    return NULL)
  if (!type_support) {
    RMW_SET_ERROR_MSG("type support handle is null");
    return NULL;
  }

  if (!qos_profile) {
    RMW_SET_ERROR_MSG("qos_profile is null");
    return nullptr;
  }

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDSDomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = participant->get_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get participant qos");
    return NULL;
  }
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_publisher_t * publisher = nullptr;
  DDS_TypeCode * type_code = nullptr;
  void * buf = nullptr;
  DDSDynamicDataTypeSupport * ddts = nullptr;
  DDS_PublisherQos publisher_qos;
  DDSPublisher * dds_publisher = nullptr;
  DDSTopic * topic = nullptr;
  DDSTopicDescription * topic_description = nullptr;
  DDS_DataWriterQos datawriter_qos;
  DDSDataWriter * topic_writer = nullptr;
  DDSDynamicDataWriter * dynamic_writer = nullptr;
  DDS_DynamicData * dynamic_data = nullptr;
  CustomPublisherInfo * custom_publisher_info = nullptr;
  std::string type_name = _create_type_name(type_support->data, "msg",
      type_support->typesupport_identifier);
  // Start initializing elements.
  publisher = rmw_publisher_allocate();
  if (!publisher) {
    RMW_SET_ERROR_MSG("failed to allocate memory for publisher");
    goto fail;
  }

  type_code = _create_type_code(
    type_name, type_support->data, type_support->typesupport_identifier);
  if (!type_code) {
    // error string was set within the function
    goto fail;
  }

  // Allocate memory for the DDSDynamicDataTypeSupport object.
  buf = rmw_allocate(sizeof(DDSDynamicDataTypeSupport));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the DDSDynamicDataTypeSupport in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(
    ddts, buf,
    goto fail,
    DDSDynamicDataTypeSupport, type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
  buf = nullptr;  // Only free the casted pointer; don't need the buf pointer anymore.
  status = ddts->register_type(participant, type_name.c_str());
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to register type");
    // Delete ddts to prevent the goto fail block from trying to unregister_type.
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      ddts->~DDSDynamicDataTypeSupport(), DDSDynamicDataTypeSupport)
    rmw_free(ddts);
    ddts = nullptr;
    goto fail;
  }

  status = participant->get_default_publisher_qos(publisher_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default publisher qos");
    goto fail;
  }

  dds_publisher = participant->create_publisher(
    publisher_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!dds_publisher) {
    RMW_SET_ERROR_MSG("failed to create publisher");
    goto fail;
  }

  topic_description = participant->lookup_topicdescription(topic_name);
  if (!topic_description) {
    DDS_TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to get default topic qos");
      goto fail;
    }

    topic = participant->create_topic(
      topic_name, type_name.c_str(), default_topic_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!topic) {
      RMW_SET_ERROR_MSG("failed to create topic");
      goto fail;
    }
  } else {
    DDS_Duration_t timeout = DDS_Duration_t::from_seconds(0);
    topic = participant->find_topic(topic_name, timeout);
    if (!topic) {
      RMW_SET_ERROR_MSG("failed to find topic");
      goto fail;
    }
  }

  if (!get_datawriter_qos(participant, *qos_profile, datawriter_qos)) {
    // error string was set within the function
    goto fail;
  }

  topic_writer = dds_publisher->create_datawriter(
    topic, datawriter_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!topic_writer) {
    RMW_SET_ERROR_MSG("failed to create data writer");
    goto fail;
  }

  dynamic_writer = DDSDynamicDataWriter::narrow(topic_writer);
  if (!dynamic_writer) {
    RMW_SET_ERROR_MSG("failed to narrow data writer");
    goto fail;
  }

  dynamic_data = ddts->create_data();
  if (!dynamic_data) {
    RMW_SET_ERROR_MSG("failed to create data");
    goto fail;
  }

  // Allocate memory for the CustomPublisherInfo object.
  buf = rmw_allocate(sizeof(CustomPublisherInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the CustomPublisherInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(custom_publisher_info, buf, goto fail, CustomPublisherInfo);
  buf = nullptr;  // Only free the casted pointer; don't need the buf pointer anymore.
  custom_publisher_info->dynamic_data_type_support_ = ddts;
  custom_publisher_info->dds_publisher_ = dds_publisher;
  custom_publisher_info->data_writer_ = topic_writer;
  custom_publisher_info->dynamic_writer_ = dynamic_writer;
  custom_publisher_info->type_code_ = type_code;
  custom_publisher_info->untyped_members_ = type_support->data;
  custom_publisher_info->dynamic_data = dynamic_data;
  custom_publisher_info->publisher_gid.implementation_identifier = rti_connext_dynamic_identifier;
  custom_publisher_info->typesupport_identifier = type_support->typesupport_identifier;
  static_assert(
    sizeof(ConnextPublisherGID) <= RMW_GID_STORAGE_SIZE,
    "RMW_GID_STORAGE_SIZE insufficient to store the rmw_connext_dynamic_cpp GID implemenation."
  );
  // Zero the gid memory before using it.
  memset(custom_publisher_info->publisher_gid.data, 0, RMW_GID_STORAGE_SIZE);
  {
    auto publisher_gid =
      reinterpret_cast<ConnextPublisherGID *>(custom_publisher_info->publisher_gid.data);
    publisher_gid->publication_handle = topic_writer->get_instance_handle();
  }
  custom_publisher_info->publisher_gid.implementation_identifier = rti_connext_dynamic_identifier;

  publisher->implementation_identifier = rti_connext_dynamic_identifier;
  publisher->data = custom_publisher_info;
  publisher->topic_name = reinterpret_cast<const char *>(rmw_allocate(strlen(topic_name) + 1));
  if (!publisher->topic_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  memcpy(const_cast<char *>(publisher->topic_name), topic_name, strlen(topic_name) + 1);

  return publisher;
fail:
  // Something went wrong, unwind anything that's already been done.
  if (custom_publisher_info) {
    rmw_free(custom_publisher_info);
  }
  if (dynamic_data) {
    if (ddts) {
      if (ddts->delete_data(dynamic_data) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "failed to delete dynamic data during handling of failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    } else {
      std::stringstream ss;
      ss << "leaking dynamic data during handling of failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (topic_writer) {
    if (dds_publisher) {
      if (dds_publisher->delete_datawriter(topic_writer) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "failed to delete datawriter during handling of failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    } else {
      std::stringstream ss;
      ss << "leaking datawriter during handling of failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (participant) {
    if (dds_publisher) {
      if (participant->delete_publisher(dds_publisher) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "failed to delete publisher during handling of failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    }
    if (ddts) {
      // TODO(wjwwood) Cannot unregister and free type here, in case another topic is using the
      // same type.
      // Should be cleaned up when the node is destroyed.
      // If we figure out a way to unregister types when they are not being used, then we can
      // add this code back in:
      /*
      if (ddts->unregister_type(participant, type_name.c_str()) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "failed to unregister type during handling of failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
      // Call destructor directly since we used a placement new to construct it.
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
        ddts->~DDSDynamicDataTypeSupport(), DDSDynamicDataTypeSupport)
      rmw_free(ddts);
      */
    }
  } else if (dds_publisher || ddts) {
    std::stringstream ss;
    ss << "leaking type registration and/or publisher during handling of failure at " <<
      __FILE__ << ":" << __LINE__ << '\n';
    (std::cerr << ss.str()).flush();
  }
  if (type_code) {
    if (destroy_type_code(type_code) != RMW_RET_OK) {
      std::stringstream ss;
      ss << "leaking type code during handling of failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
      ss.clear();
      ss << "  error: " << rmw_get_error_string_safe() << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (publisher) {
    rmw_publisher_free(publisher);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

rmw_ret_t
rmw_destroy_publisher(rmw_node_t * node, rmw_publisher_t * publisher)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  auto participant = static_cast<DDSDomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return RMW_RET_ERROR;
  }

  auto custom_publisher_info = static_cast<CustomPublisherInfo *>(publisher->data);
  if (custom_publisher_info) {
    DDSDynamicDataTypeSupport * ddts = custom_publisher_info->dynamic_data_type_support_;
    if (ddts) {
      if (custom_publisher_info->dynamic_data) {
        if (ddts->delete_data(custom_publisher_info->dynamic_data) != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete dynamic data");
          return RMW_RET_ERROR;
        }
        custom_publisher_info->dynamic_data = nullptr;
      }
      std::string type_name = _create_type_name(
        custom_publisher_info->untyped_members_, "msg",
        custom_publisher_info->typesupport_identifier);
      // TODO(wjwwood) Cannot unregister and free type here, in case another topic is using the
      // same type.
      // Should be cleaned up when the node is destroyed.
      // If we figure out a way to unregister types when they are not being used, then we can
      // add this code back in:
      /*
      if (ddts->unregister_type(participant, type_name.c_str()) != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG(("failed to unregister type: " + type_name).c_str());
        return RMW_RET_ERROR;
      }
      rmw_free(ddts);
      */
    }
    custom_publisher_info->dynamic_data_type_support_ = nullptr;
    DDSPublisher * dds_publisher = custom_publisher_info->dds_publisher_;
    if (dds_publisher) {
      auto data_writer = custom_publisher_info->data_writer_;
      if (data_writer) {
        if (dds_publisher->delete_datawriter(data_writer) != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete datawriter");
          return RMW_RET_ERROR;
        }
      }
      custom_publisher_info->data_writer_ = nullptr;
      custom_publisher_info->dynamic_writer_ = nullptr;
      if (dds_publisher->delete_contained_entities() != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to delete contained entities for publisher");
        return RMW_RET_ERROR;
      }
      if (participant->delete_publisher(dds_publisher) != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to delete publisher");
        return RMW_RET_ERROR;
      }
    }
    custom_publisher_info->dds_publisher_ = nullptr;
    if (custom_publisher_info->type_code_) {
      if (destroy_type_code(custom_publisher_info->type_code_) != RMW_RET_OK) {
        // Error string already set.
        return RMW_RET_ERROR;
      }
    }
    custom_publisher_info->type_code_ = nullptr;
    rmw_free(custom_publisher_info);
  }
  if (publisher->topic_name) {
    rmw_free(const_cast<char *>(publisher->topic_name));
  }
  publisher->data = nullptr;
  rmw_publisher_free(publisher);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_publish(const rmw_publisher_t * publisher, const void * ros_message)
{
  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  if (!ros_message) {
    RMW_SET_ERROR_MSG("ros message handle is null");
    return RMW_RET_ERROR;
  }

  CustomPublisherInfo * publisher_info = static_cast<CustomPublisherInfo *>(publisher->data);
  if (!publisher_info) {
    RMW_SET_ERROR_MSG("publisher info handle is null");
    return RMW_RET_ERROR;
  }
  DDSDynamicDataTypeSupport * ddts = publisher_info->dynamic_data_type_support_;
  if (!ddts) {
    RMW_SET_ERROR_MSG("dynamic data type support handle is null");
    return RMW_RET_ERROR;
  }
  DDSDynamicDataWriter * dynamic_writer = publisher_info->dynamic_writer_;
  if (!dynamic_writer) {
    RMW_SET_ERROR_MSG("data writer handle is null");
    return RMW_RET_ERROR;
  }
  DDS_TypeCode * type_code = publisher_info->type_code_;
  if (!type_code) {
    RMW_SET_ERROR_MSG("type code handle is null");
    return RMW_RET_ERROR;
  }
  DDS_DynamicData * dynamic_data = publisher_info->dynamic_data;
  if (!dynamic_data) {
    RMW_SET_ERROR_MSG("data handle is null");
    return RMW_RET_ERROR;
  }

  DDS_ReturnCode_t status = dynamic_data->clear_all_members();
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to clear all members");
    return RMW_RET_ERROR;
  }
  bool published = _publish(
    dynamic_data, ros_message, publisher_info->untyped_members_,
    publisher_info->typesupport_identifier);
  if (!published) {
    // error string was set within the function
    return RMW_RET_ERROR;
  }

  status = dynamic_writer->write(*dynamic_data, DDS_HANDLE_NIL);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to write");
    return RMW_RET_ERROR;
  }

  return RMW_RET_OK;
}

rmw_subscription_t *
rmw_create_subscription(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t * qos_profile,
  bool ignore_local_publications)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_dynamic_identifier,
    return NULL)

  if (!type_support) {
    RMW_SET_ERROR_MSG("type support handle is null");
    return NULL;
  }

  if (!qos_profile) {
    RMW_SET_ERROR_MSG("qos_profile is null");
    return nullptr;
  }

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDSDomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  std::string type_name = _create_type_name(
    type_support->data, "msg", type_support->typesupport_identifier);

  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = participant->get_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get participant qos");
    return NULL;
  }
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_subscription_t * subscription = nullptr;
  DDS_TypeCode * type_code = nullptr;
  void * buf = nullptr;
  DDSDynamicDataTypeSupport * ddts = nullptr;
  DDS_SubscriberQos subscriber_qos;
  DDSSubscriber * dds_subscriber = nullptr;
  DDSTopic * topic;
  DDSTopicDescription * topic_description = nullptr;
  DDSDataReader * topic_reader = nullptr;
  DDSReadCondition * read_condition = nullptr;
  DDSDynamicDataReader * dynamic_reader = nullptr;
  DDS_DataReaderQos datareader_qos;
  DDS_DynamicData * dynamic_data = nullptr;
  CustomSubscriberInfo * custom_subscriber_info = nullptr;
  // Begin initialization of elements.
  subscription = rmw_subscription_allocate();
  if (!subscription) {
    RMW_SET_ERROR_MSG("failed to allocate memory for subscription");
    goto fail;
  }

  type_code =
    _create_type_code(type_name, type_support->data, type_support->typesupport_identifier);
  if (!type_code) {
    // error string was set within the function
    goto fail;
  }

  // Allocate memory for the DDSDynamicDataTypeSupport object.
  buf = rmw_allocate(sizeof(DDSDynamicDataTypeSupport));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the DDSDynamicDataTypeSupport in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(
    ddts, buf,
    goto fail,
    DDSDynamicDataTypeSupport, type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
  buf = nullptr;  // Only free the casted pointer; don't need the buf pointer anymore.
  status = ddts->register_type(participant, type_name.c_str());
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to register type");
    // Delete ddts to prevent the goto fail block from trying to unregister_type.
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      ddts->~DDSDynamicDataTypeSupport(), DDSDynamicDataTypeSupport)
    rmw_free(ddts);
    ddts = nullptr;
    goto fail;
  }

  status = participant->get_default_subscriber_qos(subscriber_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default subscriber qos");
    goto fail;
  }

  dds_subscriber = participant->create_subscriber(
    subscriber_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!dds_subscriber) {
    RMW_SET_ERROR_MSG("failed to create subscriber");
    goto fail;
  }

  topic_description = participant->lookup_topicdescription(topic_name);
  if (!topic_description) {
    DDS_TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to get default topic qos");
      goto fail;
    }

    topic = participant->create_topic(
      topic_name, type_name.c_str(), default_topic_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!topic) {
      RMW_SET_ERROR_MSG("failed to create topic");
      goto fail;
    }
  } else {
    DDS_Duration_t timeout = DDS_Duration_t::from_seconds(0);
    topic = participant->find_topic(topic_name, timeout);
    if (!topic) {
      RMW_SET_ERROR_MSG("failed to find topic");
      goto fail;
    }
  }

  if (!get_datareader_qos(participant, *qos_profile, datareader_qos)) {
    // error string was set within the function
    goto fail;
  }

  topic_reader = dds_subscriber->create_datareader(
    topic, datareader_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!topic_reader) {
    RMW_SET_ERROR_MSG("failed to create datareader");
    goto fail;
  }

  read_condition = topic_reader->create_readcondition(
    DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  dynamic_reader = DDSDynamicDataReader::narrow(topic_reader);
  if (!dynamic_reader) {
    RMW_SET_ERROR_MSG("failed to narrow datareader");
    goto fail;
  }

  dynamic_data = ddts->create_data();
  if (!dynamic_data) {
    RMW_SET_ERROR_MSG("failed to create data");
    goto fail;
  }

  // Allocate memory for the CustomSubscriberInfo object.
  buf = rmw_allocate(sizeof(CustomSubscriberInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the CustomSubscriberInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(custom_subscriber_info, buf, goto fail, CustomSubscriberInfo);
  buf = nullptr;  // Only free the casted pointer; don't need the buf anymore.
  custom_subscriber_info->dynamic_data_type_support_ = ddts;
  custom_subscriber_info->dynamic_reader_ = dynamic_reader;
  custom_subscriber_info->data_reader_ = topic_reader;
  custom_subscriber_info->read_condition_ = read_condition;
  custom_subscriber_info->dds_subscriber_ = dds_subscriber;
  custom_subscriber_info->ignore_local_publications = ignore_local_publications;
  custom_subscriber_info->type_code_ = type_code;
  custom_subscriber_info->untyped_members_ = type_support->data;
  custom_subscriber_info->dynamic_data = dynamic_data;
  custom_subscriber_info->typesupport_identifier = type_support->typesupport_identifier;

  subscription->implementation_identifier = rti_connext_dynamic_identifier;
  subscription->data = custom_subscriber_info;

  subscription->topic_name = reinterpret_cast<const char *>(rmw_allocate(strlen(topic_name) + 1));
  if (!subscription->topic_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  memcpy(const_cast<char *>(subscription->topic_name), topic_name, strlen(topic_name) + 1);
  return subscription;
fail:
  // Something has gone wrong, unroll what has been done.
  if (custom_subscriber_info) {
    rmw_free(custom_subscriber_info);
  }
  if (dynamic_data) {
    if (ddts) {
      if (ddts->delete_data(dynamic_data) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "failed to delete dynamic data during handling of failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    } else {
      std::stringstream ss;
      ss << "leaking dynamic data during handling of failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (topic_reader) {
    if (read_condition) {
      if (topic_reader->delete_readcondition(read_condition) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "leaking readcondition while handling failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    }
    if (dds_subscriber) {
      if (dds_subscriber->delete_datareader(topic_reader) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "failed to delete datareader during handling of failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    } else {
      std::stringstream ss;
      ss << "leaking datareader during handling of failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (participant) {
    if (dds_subscriber) {
      if (participant->delete_subscriber(dds_subscriber) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "failed to delete subscriber during handling of failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    }
    if (ddts) {
      // TODO(wjwwood) Cannot unregister and free type here, in case another topic is using the
      // same type.
      // Should be cleaned up when the node is destroyed.
      // If we figure out a way to unregister types when they are not being used, then we can
      // add this code back in:
      /*
      if (ddts->unregister_type(participant, type_name.c_str()) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "failed to unregister type during handling of failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
      // Call destructor directly since we used a placement new to construct it.
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
        ddts->~DDSDynamicDataTypeSupport(), DDSDynamicDataTypeSupport)
      rmw_free(ddts);
      */
    }
  } else if (dds_subscriber || ddts) {
    std::stringstream ss;
    ss << "leaking type registration and/or publisher during handling of failure at " <<
      __FILE__ << ":" << __LINE__ << '\n';
    (std::cerr << ss.str()).flush();
  }
  if (type_code) {
    if (destroy_type_code(type_code) != RMW_RET_OK) {
      std::stringstream ss;
      ss << "leaking type code during handling of failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
      ss.clear();
      ss << "  error: " << rmw_get_error_string_safe() << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (subscription) {
    rmw_subscription_free(subscription);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

rmw_ret_t
rmw_destroy_subscription(rmw_node_t * node, rmw_subscription_t * subscription)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  if (!subscription) {
    RMW_SET_ERROR_MSG("subscription handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription handle,
    subscription->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  auto participant = static_cast<DDSDomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return RMW_RET_ERROR;
  }

  auto custom_subscription_info = static_cast<CustomSubscriberInfo *>(subscription->data);
  if (custom_subscription_info) {
    DDSDynamicDataTypeSupport * ddts = custom_subscription_info->dynamic_data_type_support_;
    if (ddts) {
      if (custom_subscription_info->dynamic_data) {
        if (ddts->delete_data(custom_subscription_info->dynamic_data) != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete dynamic data");
          return RMW_RET_ERROR;
        }
        custom_subscription_info->dynamic_data = nullptr;
      }
      std::string type_name = _create_type_name(
        custom_subscription_info->untyped_members_, "msg",
        custom_subscription_info->typesupport_identifier);
      // TODO(wjwwood) Cannot unregister and free type here, in case another topic is using the
      // same type.
      // Should be cleaned up when the node is destroyed.
      // If we figure out a way to unregister types when they are not being used, then we can
      // add this code back in:
      /*
      if (ddts->unregister_type(participant, type_name.c_str()) != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG(("failed to unregister type: " + type_name).c_str());
        return RMW_RET_ERROR;
      }
      rmw_free(ddts);
      */
    }
    custom_subscription_info->dynamic_data_type_support_ = nullptr;
    DDSSubscriber * dds_subscriber = custom_subscription_info->dds_subscriber_;
    if (dds_subscriber) {
      auto data_reader = custom_subscription_info->data_reader_;
      if (data_reader) {
        auto read_condition = custom_subscription_info->read_condition_;
        if (read_condition) {
          if (data_reader->delete_readcondition(read_condition) != DDS_RETCODE_OK) {
            RMW_SET_ERROR_MSG("failed to delete readcondition");
            return RMW_RET_ERROR;
          }
          custom_subscription_info->read_condition_ = nullptr;
        }
        if (dds_subscriber->delete_datareader(data_reader) != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete datareader");
          return RMW_RET_ERROR;
        }
      }
      custom_subscription_info->data_reader_ = nullptr;
      custom_subscription_info->dynamic_reader_ = nullptr;
      if (participant->delete_subscriber(dds_subscriber) != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to delete subscriber");
        return RMW_RET_ERROR;
      }
    }
    custom_subscription_info->dds_subscriber_ = nullptr;
    if (custom_subscription_info->type_code_) {
      if (destroy_type_code(custom_subscription_info->type_code_) != RMW_RET_OK) {
        return RMW_RET_ERROR;
      }
    }
    custom_subscription_info->type_code_ = nullptr;
    rmw_free(custom_subscription_info);
  }
  subscription->data = nullptr;
  if (subscription->topic_name) {
    rmw_free(const_cast<char *>(subscription->topic_name));
  }
  rmw_subscription_free(subscription);
  return RMW_RET_OK;
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


rmw_ret_t
_take_impl(const rmw_subscription_t * subscription, void * ros_message, bool * taken,
  DDS_InstanceHandle_t * sending_publication_handle)
{
  if (!subscription) {
    RMW_SET_ERROR_MSG("subscription handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription handle,
    subscription->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  if (!ros_message) {
    RMW_SET_ERROR_MSG("ros message handle is null");
    return RMW_RET_ERROR;
  }
  if (!taken) {
    RMW_SET_ERROR_MSG("taken handle is null");
    return RMW_RET_ERROR;
  }

  CustomSubscriberInfo * subscriber_info =
    static_cast<CustomSubscriberInfo *>(subscription->data);
  if (!subscriber_info) {
    RMW_SET_ERROR_MSG("subscriber info handle is null");
    return RMW_RET_ERROR;
  }
  DDSDynamicDataTypeSupport * ddts = subscriber_info->dynamic_data_type_support_;
  if (!ddts) {
    RMW_SET_ERROR_MSG("dynamic data type support handle is null");
    return RMW_RET_ERROR;
  }
  DDSDynamicDataReader * dynamic_reader = subscriber_info->dynamic_reader_;
  if (!dynamic_reader) {
    RMW_SET_ERROR_MSG("data reader handle is null");
    return RMW_RET_ERROR;
  }
  DDS_TypeCode * type_code = subscriber_info->type_code_;
  if (!type_code) {
    RMW_SET_ERROR_MSG("type code handle is null");
    return RMW_RET_ERROR;
  }

  DDS_DynamicDataSeq dynamic_data_sequence;
  DDS_SampleInfoSeq sample_infos;
  DDS_ReturnCode_t status = dynamic_reader->take(
    dynamic_data_sequence,
    sample_infos,
    1,
    DDS_ANY_SAMPLE_STATE,
    DDS_ANY_VIEW_STATE,
    DDS_ANY_INSTANCE_STATE);
  if (status == DDS_RETCODE_NO_DATA) {
    *taken = false;
    return RMW_RET_OK;
  }
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to take sample");
    return RMW_RET_ERROR;
  }

  bool ignore_sample = false;
  DDS_SampleInfo & sample_info = sample_infos[0];
  if (!sample_info.valid_data) {
    // skip sample without data
    ignore_sample = true;
  } else if (subscriber_info->ignore_local_publications) {
    // compare the lower 12 octets of the guids from the sender and this receiver
    // if they are equal the sample has been sent from this process and should be ignored
    DDS_GUID_t sender_guid = sample_info.original_publication_virtual_guid;
    DDS_InstanceHandle_t receiver_instance_handle = dynamic_reader->get_instance_handle();
    ignore_sample = true;
    for (size_t i = 0; i < 12; ++i) {
      DDS_Octet * sender_element = &(sender_guid.value[i]);
      DDS_Octet * receiver_element = &(reinterpret_cast<DDS_Octet *>(&receiver_instance_handle)[i]);
      if (*sender_element != *receiver_element) {
        ignore_sample = false;
        break;
      }
    }
  }
  if (sample_info.valid_data && sending_publication_handle != nullptr) {
    *sending_publication_handle = sample_info.publication_handle;
  }

  bool success = true;
  if (!ignore_sample) {
    success = _take(&dynamic_data_sequence[0], ros_message, subscriber_info->untyped_members_,
        subscriber_info->typesupport_identifier);
    if (success) {
      *taken = true;
    }
  }

  dynamic_reader->return_loan(dynamic_data_sequence, sample_infos);

  if (!success) {
    // error string was set within the function
    return RMW_RET_ERROR;
  }

  return RMW_RET_OK;
}

rmw_ret_t
rmw_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken)
{
  return _take_impl(subscription, ros_message, taken, nullptr);
}

rmw_ret_t
rmw_take_with_info(
  const rmw_subscription_t * subscription,
  void * ros_message,
  bool * taken,
  rmw_message_info_t * message_info)
{
  if (!message_info) {
    RMW_SET_ERROR_MSG("message info is null");
    return RMW_RET_ERROR;
  }
  DDS_InstanceHandle_t sending_publication_handle;
  auto ret = _take_impl(subscription, ros_message, taken, &sending_publication_handle);
  if (ret != RMW_RET_OK) {
    // Error string is already set.
    return RMW_RET_ERROR;
  }

  rmw_gid_t * sender_gid = &message_info->publisher_gid;
  sender_gid->implementation_identifier = rti_connext_dynamic_identifier;
  memset(sender_gid->data, 0, RMW_GID_STORAGE_SIZE);
  auto detail = reinterpret_cast<ConnextPublisherGID *>(sender_gid->data);
  detail->publication_handle = sending_publication_handle;

  return RMW_RET_OK;
}

rmw_guard_condition_t *
rmw_create_guard_condition()
{
  return create_guard_condition(rti_connext_dynamic_identifier);
}

rmw_ret_t
rmw_destroy_guard_condition(rmw_guard_condition_t * guard_condition)
{
  return destroy_guard_condition(rti_connext_dynamic_identifier, guard_condition);
}

rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition_handle)
{
  return trigger_guard_condition(rti_connext_dynamic_identifier, guard_condition_handle);
}

rmw_waitset_t *
rmw_create_waitset(size_t max_conditions)
{
  return create_waitset(rti_connext_dynamic_identifier, max_conditions);
}

rmw_ret_t
rmw_destroy_waitset(rmw_waitset_t * waitset)
{
  return destroy_waitset(rti_connext_dynamic_identifier, waitset);
}

rmw_ret_t
rmw_wait(
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_waitset_t * waitset,
  const rmw_time_t * wait_timeout)
{
  return wait<CustomSubscriberInfo, ConnextDynamicServiceInfo, ConnextDynamicClientInfo>
           (rti_connext_dynamic_identifier, subscriptions, guard_conditions, services, clients,
           waitset,
           wait_timeout);
}

rmw_client_t *
rmw_create_client(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rmw_qos_profile_t * qos_profile)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_dynamic_identifier,
    return NULL)

  if (!type_support) {
    RMW_SET_ERROR_MSG("type support handle is null");
    return NULL;
  }

  if (!qos_profile) {
    RMW_SET_ERROR_MSG("qos_profile is null");
    return nullptr;
  }

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDSDomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  const void * untyped_request_members =
    get_request_ptr(type_support->data, type_support->typesupport_identifier);
  if (!untyped_request_members) {
    RMW_SET_ERROR_MSG("couldn't get request members");
    return NULL;
  }
  const void * untyped_response_members = get_response_ptr(type_support->data,
      type_support->typesupport_identifier);
  if (!untyped_response_members) {
    RMW_SET_ERROR_MSG("couldn't get response members");
    return NULL;
  }

  std::string request_type_name = _create_type_name(untyped_request_members, "srv",
      type_support->typesupport_identifier);
  std::string response_type_name = _create_type_name(untyped_response_members, "srv",
      type_support->typesupport_identifier);

  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = participant->get_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get participant qos");
    return NULL;
  }
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_client_t * client = nullptr;
  DDS_TypeCode * request_type_code = nullptr;
  void * buf = nullptr;
  DDS::DynamicDataTypeSupport * request_type_support = nullptr;
  DDS_TypeCode * response_type_code = nullptr;
  DDS::DynamicDataTypeSupport * response_type_support = nullptr;
  DDS_DataReaderQos datareader_qos;
  DDS_DataWriterQos datawriter_qos;
  connext::Requester<DDS_DynamicData, DDS_DynamicData> * requester = nullptr;
  DDSDataReader * response_datareader = nullptr;
  DDSReadCondition * read_condition = nullptr;
  ConnextDynamicClientInfo * client_info = nullptr;

  // Begin initializing elements
  client = rmw_client_allocate();
  if (!client) {
    RMW_SET_ERROR_MSG("failed to allocate memory for client");
    goto fail;
  }

  request_type_code = _create_type_code(request_type_name, untyped_request_members,
      type_support->typesupport_identifier);
  if (!request_type_code) {
    // error string was set within the function
    RMW_SET_ERROR_MSG("Could not set type code for request");
    goto fail;
  }

  // Allocate memory for the DDS::DynamicDataTypeSupport object.
  buf = rmw_allocate(sizeof(DDS::DynamicDataTypeSupport));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }

  response_type_code = _create_type_code(response_type_name, untyped_response_members,
      type_support->typesupport_identifier);
  if (!response_type_code) {
    // error string was set within the function
    RMW_SET_ERROR_MSG("Could not set type code for response");
    goto fail;
  }

  // Use a placement new to construct the DDS::DynamicDataTypeSupport in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(
    request_type_support, buf,
    goto fail,
    DDS::DynamicDataTypeSupport, request_type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT)
  buf = nullptr;  // Only free the casted pointer; don't need the buf pointer anymore.

  if (!request_type_support->is_valid()) {
    RMW_SET_ERROR_MSG("failed to construct dynamic data type support for request");
    goto fail;
  }
  // Allocate memory for the DDS::DynamicDataTypeSupport object.
  buf = rmw_allocate(sizeof(DDS::DynamicDataTypeSupport));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the DDS::DynamicDataTypeSupport in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(
    response_type_support, buf,
    goto fail,
    DDS::DynamicDataTypeSupport, response_type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT)
  buf = nullptr;  // Only free the casted pointer; don't need the buf pointer anymore.

  if (!response_type_support->is_valid()) {
    RMW_SET_ERROR_MSG("failed to construct dynamic data type support for response");
    goto fail;
  }

  // create requester
  {
    if (!get_datareader_qos(participant, *qos_profile, datareader_qos)) {
      // error string was set within the function
      goto fail;
    }
    if (!get_datawriter_qos(participant, *qos_profile, datawriter_qos)) {
      // error string was set within the function
      goto fail;
    }

    connext::RequesterParams requester_params(participant);
    requester_params.service_name(service_name);
    requester_params.request_type_support(request_type_support);
    requester_params.reply_type_support(response_type_support);
    requester_params.datareader_qos(datareader_qos);
    requester_params.datawriter_qos(datawriter_qos);

    // Allocate memory for the Requester object.
    using Requester = connext::Requester<DDS_DynamicData, DDS_DynamicData>;
    buf = rmw_allocate(sizeof(connext::Requester<DDS_DynamicData, DDS_DynamicData>));
    if (!buf) {
      RMW_SET_ERROR_MSG("failed to allocate memory");
      goto fail;
    }
    // Use a placement new to construct the Requester in the preallocated buffer.
    RMW_TRY_PLACEMENT_NEW(
      requester, buf,
      goto fail,
      Requester, requester_params)
    buf = nullptr;  // Only free the casted pointer; don't need the buf pointer anymore.
  }

  response_datareader = requester->get_reply_datareader();
  if (!response_datareader) {
    RMW_SET_ERROR_MSG("failed to get response datareader");
    goto fail;
  }

  read_condition = response_datareader->create_readcondition(
    DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  // Allocate memory for the ConnextDynamicClientInfo object.
  buf = rmw_allocate(sizeof(ConnextDynamicClientInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the ConnextDynamicClientInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(client_info, buf, goto fail, ConnextDynamicClientInfo)
  buf = nullptr;  // Only free the casted pointer; don't need the buf pointer anymore.
  client_info->requester_ = requester;
  client_info->response_datareader_ = response_datareader;
  client_info->read_condition_ = read_condition;
  client_info->request_type_support_ = request_type_support;
  client_info->response_type_support_ = response_type_support;
  client_info->response_type_code_ = response_type_code;
  client_info->request_type_code_ = request_type_code;
  client_info->untyped_request_members_ = untyped_request_members;
  client_info->untyped_response_members_ = untyped_response_members;
  client_info->typesupport_identifier = type_support->typesupport_identifier;

  client->implementation_identifier = rti_connext_dynamic_identifier;
  client->data = client_info;
  client->service_name = reinterpret_cast<const char *>(rmw_allocate(strlen(service_name) + 1));
  if (!client->service_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  memcpy(const_cast<char *>(client->service_name), service_name, strlen(service_name) + 1);
  return client;
fail:
  if (response_datareader) {
    if (read_condition) {
      if (response_datareader->delete_readcondition(read_condition) != DDS::RETCODE_OK) {
        fprintf(stderr, "leaking readcondition while handling failure\n");
      }
    }
  }
  if (client) {
    rmw_client_free(client);
  }
  if (request_type_code) {
    if (destroy_type_code(request_type_code) != RMW_RET_OK) {
      std::stringstream ss;
      ss << "leaking type code during handling of failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
      ss.clear();
      ss << "  error: " << rmw_get_error_string_safe() << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (request_type_support) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      request_type_support->~DynamicDataTypeSupport(), DynamicDataTypeSupport)
    rmw_free(request_type_support);
  }
  if (response_type_code) {
    if (destroy_type_code(response_type_code) != RMW_RET_OK) {
      std::stringstream ss;
      ss << "leaking type code during handling of failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
      ss.clear();
      ss << "  error: " << rmw_get_error_string_safe() << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (response_type_support) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      response_type_support->~DynamicDataTypeSupport(), DynamicDataTypeSupport)
    rmw_free(response_type_support);
  }
  if (requester) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      requester->~Requester(), "connext::Requester<DDS_DynamicData, DDS_DynamicData>")
    rmw_free(requester);
  }
  if (client_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      client_info->~ConnextDynamicClientInfo(), ConnextDynamicClientInfo)
    rmw_free(client_info);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

rmw_ret_t
rmw_destroy_client(rmw_client_t * client)
{
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  ConnextDynamicClientInfo * client_info = static_cast<ConnextDynamicClientInfo *>(client->data);
  if (client_info) {
    auto response_datareader = client_info->response_datareader_;
    if (response_datareader) {
      auto read_condition = client_info->read_condition_;
      if (response_datareader->delete_readcondition(read_condition) != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to delete readcondition");
        return RMW_RET_ERROR;
      }
      client_info->read_condition_ = nullptr;
    }
    if (client_info->request_type_code_) {
      if (destroy_type_code(client_info->request_type_code_) != RMW_RET_OK) {
        RMW_SET_ERROR_MSG("failed to destroy type code");
        return RMW_RET_ERROR;
      }
    }
    if (client_info->response_type_code_) {
      if (destroy_type_code(client_info->response_type_code_) != RMW_RET_OK) {
        RMW_SET_ERROR_MSG("failed to destroy type code");
        return RMW_RET_ERROR;
      }
    }
    // TODO(dirk-thomas) the deletion break something within Connext
    // afterwards it is not possible to create another client of the same type
    // if (client_info->request_type_support_) {
    //   RMW_TRY_DESTRUCTOR(
    //     client_info->request_type_support_->~DynamicDataTypeSupport(),
    //     DynamicDataTypeSupport, result = RMW_RET_ERROR)
    //   rmw_free(client_info->request_type_support_);
    // }
    // if (client_info->response_type_support_) {
    //   RMW_TRY_DESTRUCTOR(
    //     client_info->response_type_support_->~DynamicDataTypeSupport(),
    //     DynamicDataTypeSupport, result = RMW_RET_ERROR)
    //   rmw_free(client_info->response_type_support_);
    // }
    if (client_info->requester_) {
      RMW_TRY_DESTRUCTOR(
        client_info->requester_->~Requester(),
        "connext::Requester<DDS_DynamicData, DDS_DynamicData>",
        result = RMW_RET_ERROR)
      rmw_free(client_info->requester_);
    }
    if (client->service_name) {
      rmw_free(const_cast<char *>(client->service_name));
    }
  }

  if (client_info) {
    RMW_TRY_DESTRUCTOR(
      client_info->~ConnextDynamicClientInfo(), ConnextDynamicClientInfo, result = RMW_RET_ERROR)
    rmw_free(client_info);
  }

  rmw_client_free(client);

  // TODO(wjwwood): if multiple destructors fail, some of the error messages could be suppressed.
  return result;
}

rmw_ret_t
rmw_send_request(
  const rmw_client_t * client,
  const void * ros_request,
  int64_t * sequence_id)
{
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  if (!ros_request) {
    RMW_SET_ERROR_MSG("ros request handle is null");
    return RMW_RET_ERROR;
  }

  ConnextDynamicClientInfo * client_info = static_cast<ConnextDynamicClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }
  connext::Requester<DDS_DynamicData, DDS_DynamicData> * requester = client_info->requester_;
  if (!requester) {
    RMW_SET_ERROR_MSG("requester handle is null");
    return RMW_RET_ERROR;
  }

  DDS::DynamicData * sample = client_info->request_type_support_->create_data();
  if (!sample) {
    RMW_SET_ERROR_MSG("failed to create data");
    return RMW_RET_ERROR;
  }
  DDS::WriteParams_t writeParams;
  connext::WriteSampleRef<DDS::DynamicData> request(*sample, writeParams);

  bool published = _publish(
    sample, ros_request, client_info->untyped_request_members_,
    client_info->typesupport_identifier);
  if (!published) {
    // error string was set within the function
    if (client_info->request_type_support_->delete_data(sample) != DDS_RETCODE_OK) {
      std::stringstream ss;
      ss << "leaking dynamic data object while handling error at " <<
        __FILE__ << ":" << __LINE__;
      (std::cerr << ss.str()).flush();
    }
    return RMW_RET_ERROR;
  }

  requester->send_request(request);
  *sequence_id = ((int64_t)request.identity().sequence_number.high) << 32 |
    request.identity().sequence_number.low;

  if (client_info->request_type_support_->delete_data(sample) != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete dynamic data object");
    return RMW_RET_ERROR;
  }

  return RMW_RET_OK;
}

rmw_service_t *
rmw_create_service(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rmw_qos_profile_t * qos_profile)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_dynamic_identifier,
    return NULL)
  if (node->implementation_identifier != rti_connext_dynamic_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return NULL;
  }

  if (!type_support) {
    RMW_SET_ERROR_MSG("type support handle is null");
    return NULL;
  }

  if (!qos_profile) {
    RMW_SET_ERROR_MSG("qos_profile is null");
    return nullptr;
  }

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDSDomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  const void * untyped_request_members = get_request_ptr(type_support->data,
      type_support->typesupport_identifier);
  const void * untyped_response_members = get_response_ptr(type_support->data,
      type_support->typesupport_identifier);
  std::string request_type_name = _create_type_name(untyped_request_members, "srv",
      type_support->typesupport_identifier);
  std::string response_type_name = _create_type_name(untyped_response_members, "srv",
      type_support->typesupport_identifier);

  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = participant->get_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get participant qos");
    return NULL;
  }
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_service_t * service = nullptr;
  DDS_TypeCode * request_type_code = nullptr;
  void * buf = nullptr;
  DDS::DynamicDataTypeSupport * request_type_support = nullptr;
  DDS_TypeCode * response_type_code = nullptr;
  DDS::DynamicDataTypeSupport * response_type_support = nullptr;
  DDS_DataReaderQos datareader_qos;
  DDS_DataWriterQos datawriter_qos;
  connext::Replier<DDS_DynamicData, DDS_DynamicData> * replier = nullptr;
  DDSDataReader * request_datareader = nullptr;
  DDSReadCondition * read_condition = nullptr;
  ConnextDynamicServiceInfo * server_info = nullptr;
  // Begin initializing elements
  service = rmw_service_allocate();
  if (!service) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }

  request_type_code = _create_type_code(request_type_name, untyped_request_members,
      type_support->typesupport_identifier);
  if (!request_type_code) {
    // error string was set within the function
    RMW_SET_ERROR_MSG("Could not set type code for response");
    goto fail;
  }
  // Allocate memory for the DDS::DynamicDataTypeSupport object.
  buf = rmw_allocate(sizeof(DDS::DynamicDataTypeSupport));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the DDS::DynamicDataTypeSupport in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(
    request_type_support, buf,
    goto fail,
    DDS::DynamicDataTypeSupport, request_type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT)
  buf = nullptr;  // Only free the casted pointer; don't need the buf anymore.

  response_type_code = _create_type_code(response_type_name, untyped_response_members,
      type_support->typesupport_identifier);
  if (!response_type_code) {
    // error string was set within the function
    RMW_SET_ERROR_MSG("Could not set type code for response");
    goto fail;
  }
  // Allocate memory for the DDS::DynamicDataTypeSupport object.
  buf = rmw_allocate(sizeof(DDS::DynamicDataTypeSupport));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the DDS::DynamicDataTypeSupport in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(
    response_type_support, buf,
    goto fail,
    DDS::DynamicDataTypeSupport, response_type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT)
  buf = nullptr;  // Only free the casted pointer; don't need the buf anymore.

  {
    if (!get_datareader_qos(participant, *qos_profile, datareader_qos)) {
      // error string was set within the function
      goto fail;
    }
    if (!get_datawriter_qos(participant, *qos_profile, datawriter_qos)) {
      // error string was set within the function
      goto fail;
    }

    // create requester
    connext::ReplierParams<DDS_DynamicData, DDS_DynamicData> replier_params(participant);
    replier_params.service_name(service_name);
    replier_params.request_type_support(request_type_support);
    replier_params.reply_type_support(response_type_support);
    replier_params.datareader_qos(datareader_qos);
    replier_params.datawriter_qos(datawriter_qos);

    // Allocate memory for the Replier object.
    using Replier = connext::Replier<DDS_DynamicData, DDS_DynamicData>;
    buf = rmw_allocate(sizeof(Replier));
    if (!buf) {
      RMW_SET_ERROR_MSG("failed to allocate memory");
      goto fail;
    }
    // Use a placement new to construct the Replier in the preallocated buffer.
    RMW_TRY_PLACEMENT_NEW(replier, buf, goto fail, Replier, replier_params)
    buf = nullptr;  // Only free casted pointer; don't need the buf pointer anymore.
  }

  request_datareader = replier->get_request_datareader();
  if (!request_datareader) {
    RMW_SET_ERROR_MSG("failed to get request datareader");
    goto fail;
  }

  read_condition = request_datareader->create_readcondition(
    DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  // Allocate memory for the ConnextDynamicServiceInfo object.
  buf = rmw_allocate(sizeof(ConnextDynamicServiceInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the ConnextDynamicServiceInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(server_info, buf, goto fail, ConnextDynamicServiceInfo)
  buf = nullptr;  // Only free the casted pointer; don't need the buf pointer anymore.
  server_info->replier_ = replier;
  server_info->request_datareader_ = request_datareader;
  server_info->read_condition_ = read_condition;
  server_info->response_type_support_ = response_type_support;
  server_info->untyped_request_members_ = untyped_request_members;
  server_info->untyped_response_members_ = untyped_response_members;
  server_info->typesupport_identifier = type_support->typesupport_identifier;

  service->implementation_identifier = rti_connext_dynamic_identifier;
  service->data = server_info;
  service->service_name = reinterpret_cast<const char *>(rmw_allocate(strlen(service_name) + 1));
  if (!service->service_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  memcpy(const_cast<char *>(service->service_name), service_name, strlen(service_name) + 1);
  return service;
fail:
  if (request_datareader) {
    if (read_condition) {
      if (request_datareader->delete_readcondition(read_condition) != DDS::RETCODE_OK) {
        fprintf(stderr, "leaking readcondition while handling failure\n");
      }
    }
  }
  if (service) {
    rmw_service_free(service);
  }
  if (request_type_code) {
    if (destroy_type_code(request_type_code) != RMW_RET_OK) {
      std::stringstream ss;
      ss << "leaking type code during handling of failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
      ss.clear();
      ss << "  error: " << rmw_get_error_string_safe() << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (request_type_support) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      request_type_support->~DynamicDataTypeSupport(), DynamicDataTypeSupport)
    rmw_free(request_type_support);
  }
  if (response_type_code) {
    if (destroy_type_code(response_type_code) != RMW_RET_OK) {
      std::stringstream ss;
      ss << "leaking type code during handling of failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
      ss.clear();
      ss << "  error: " << rmw_get_error_string_safe() << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (response_type_support) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      response_type_support->~DynamicDataTypeSupport(), DynamicDataTypeSupport)
    rmw_free(response_type_support);
  }
  if (replier) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      replier->~Replier(), "connext::Replier<DDS_DynamicData, DDS_DynamicData>")
    rmw_free(replier);
  }
  if (server_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      server_info->~ConnextDynamicServiceInfo(), ConnextDynamicServiceInfo)
    rmw_free(server_info);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

rmw_ret_t
rmw_destroy_service(rmw_service_t * service)
{
  if (!service) {
    RMW_SET_ERROR_MSG("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  auto service_info = static_cast<ConnextDynamicServiceInfo *>(service->data);
  if (service_info) {
    auto request_datareader = service_info->request_datareader_;
    if (request_datareader) {
      auto read_condition = service_info->read_condition_;
      if (request_datareader->delete_readcondition(read_condition) != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to delete readcondition");
        return RMW_RET_ERROR;
      }
      service_info->read_condition_ = nullptr;
    }
    if (service_info->request_type_code_) {
      if (destroy_type_code(service_info->request_type_code_) != RMW_RET_OK) {
        RMW_SET_ERROR_MSG("failed to destroy type code");
        return RMW_RET_ERROR;
      }
    }
    if (service_info->response_type_code_) {
      if (destroy_type_code(service_info->response_type_code_) != RMW_RET_OK) {
        RMW_SET_ERROR_MSG("failed to destroy type code");
        return RMW_RET_ERROR;
      }
    }
    if (service_info->request_type_support_) {
      RMW_TRY_DESTRUCTOR(
        service_info->request_type_support_->~DynamicDataTypeSupport(),
        DynamicDataTypeSupport, result = RMW_RET_ERROR)
      rmw_free(service_info->request_type_support_);
    }
    if (service_info->response_type_support_) {
      RMW_TRY_DESTRUCTOR(
        service_info->response_type_support_->~DynamicDataTypeSupport(),
        DynamicDataTypeSupport, result = RMW_RET_ERROR)
      rmw_free(service_info->response_type_support_);
    }
    if (service_info->replier_) {
      RMW_TRY_DESTRUCTOR(
        service_info->replier_->~Replier(),
        "connext::Replier<DDS_DynamicData, DDS_DynamicData>",
        result = RMW_RET_ERROR)
      rmw_free(service_info->replier_);
    }
    if (service->service_name) {
      rmw_free(const_cast<char *>(service->service_name));
    }
  }
  if (service_info) {
    RMW_TRY_DESTRUCTOR(service_info->~ConnextDynamicServiceInfo(),
      ConnextDynamicServiceInfo, result = RMW_RET_ERROR)
    rmw_free(service_info);
  }

  rmw_service_free(service);

  // TODO(wjwwood): if the function returns early some memory leaks will occur.
  return result;
}

rmw_ret_t
rmw_take_request(
  const rmw_service_t * service,
  rmw_request_id_t * request_header,
  void * ros_request,
  bool * taken)
{
  if (!service) {
    RMW_SET_ERROR_MSG("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  if (!request_header) {
    RMW_SET_ERROR_MSG("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_request) {
    RMW_SET_ERROR_MSG("ros request handle is null");
    return RMW_RET_ERROR;
  }
  if (!taken) {
    RMW_SET_ERROR_MSG("taken handle is null");
    return RMW_RET_ERROR;
  }

  ConnextDynamicServiceInfo * service_info =
    static_cast<ConnextDynamicServiceInfo *>(service->data);
  if (!service_info) {
    RMW_SET_ERROR_MSG("service info handle is null");
    return RMW_RET_ERROR;
  }
  connext::Replier<DDS_DynamicData, DDS_DynamicData> * replier = service_info->replier_;
  if (!replier) {
    RMW_SET_ERROR_MSG("replier handle is null");
    return RMW_RET_ERROR;
  }

  connext::LoanedSamples<DDS::DynamicData> requests = replier->take_requests(1);
  if (requests.begin() != requests.end() && requests.begin()->info().valid_data) {
    bool success = _take(
      &requests.begin()->data(), ros_request, service_info->untyped_request_members_,
      service_info->typesupport_identifier);
    if (!success) {
      // error string was set within the function
      return RMW_RET_ERROR;
    }

    size_t SAMPLE_IDENTITY_SIZE = 16;
    memcpy(
      &request_header->writer_guid[0], requests.begin()->identity().writer_guid.value,
      SAMPLE_IDENTITY_SIZE);

    request_header->sequence_number =
      ((int64_t)requests.begin()->identity().sequence_number.high) << 32 |
      requests.begin()->identity().sequence_number.low;
    *taken = true;
  } else {
    *taken = false;
  }

  return RMW_RET_OK;
}

rmw_ret_t
rmw_take_response(
  const rmw_client_t * client,
  rmw_request_id_t * request_header,
  void * ros_response,
  bool * taken)
{
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  if (!request_header) {
    RMW_SET_ERROR_MSG("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_response) {
    RMW_SET_ERROR_MSG("ros response handle is null");
    return RMW_RET_ERROR;
  }
  if (!taken) {
    RMW_SET_ERROR_MSG("taken handle is null");
    return RMW_RET_ERROR;
  }

  ConnextDynamicClientInfo * client_info =
    static_cast<ConnextDynamicClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }
  connext::Requester<DDS_DynamicData, DDS_DynamicData> * requester = client_info->requester_;
  if (!requester) {
    RMW_SET_ERROR_MSG("requester handle is null");
    return RMW_RET_ERROR;
  }

  connext::LoanedSamples<DDS::DynamicData> replies = requester->take_replies(1);
  if (replies.begin() != replies.end() && replies.begin()->info().valid_data) {
    bool success = _take(
      &replies.begin()->data(), ros_response, client_info->untyped_response_members_,
      client_info->typesupport_identifier);
    if (!success) {
      // error string was set within the function
      return RMW_RET_ERROR;
    }

    request_header->sequence_number =
      (((int64_t)replies.begin()->related_identity().sequence_number.high) << 32) |
      replies.begin()->related_identity().sequence_number.low;
    *taken = true;
  } else {
    *taken = false;
  }

  return RMW_RET_OK;
}

rmw_ret_t
rmw_send_response(
  const rmw_service_t * service,
  rmw_request_id_t * request_header,
  void * ros_response)
{
  if (!service) {
    RMW_SET_ERROR_MSG("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  if (!request_header) {
    RMW_SET_ERROR_MSG("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_response) {
    RMW_SET_ERROR_MSG("ros response handle is null");
    return RMW_RET_ERROR;
  }

  ConnextDynamicServiceInfo * service_info =
    static_cast<ConnextDynamicServiceInfo *>(service->data);
  if (!service_info) {
    RMW_SET_ERROR_MSG("service info handle is null");
    return RMW_RET_ERROR;
  }
  connext::Replier<DDS_DynamicData, DDS_DynamicData> * replier = service_info->replier_;
  if (!replier) {
    RMW_SET_ERROR_MSG("replier handle is null");
    return RMW_RET_ERROR;
  }

  DDS::DynamicData * sample = service_info->response_type_support_->create_data();
  if (!sample) {
    RMW_SET_ERROR_MSG("failed to create data");
    return RMW_RET_ERROR;
  }
  DDS::WriteParams_t writeParams;
  connext::WriteSampleRef<DDS::DynamicData> response(*sample, writeParams);

  bool published = _publish(sample, ros_response, service_info->untyped_response_members_,
      service_info->typesupport_identifier);
  if (!published) {
    // error string was set within the function
    if (service_info->response_type_support_->delete_data(sample) != DDS_RETCODE_OK) {
      std::stringstream ss;
      ss << "leaking dynamic data object while handling error at " <<
        __FILE__ << ":" << __LINE__;
      (std::cerr << ss.str()).flush();
    }
    return RMW_RET_ERROR;
  }

  DDS_SampleIdentity_t request_identity;

  size_t SAMPLE_IDENTITY_SIZE = 16;
  memcpy(request_identity.writer_guid.value, &request_header->writer_guid[0], SAMPLE_IDENTITY_SIZE);

  request_identity.sequence_number.high = (int32_t)(
    (request_header->sequence_number & 0xFFFFFFFF00000000) >> 32);
  request_identity.sequence_number.low = (uint32_t)(request_header->sequence_number & 0xFFFFFFFF);

  replier->send_reply(response, request_identity);

  if (service_info->response_type_support_->delete_data(sample) != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete dynamic data object");
    return RMW_RET_ERROR;
  }

  return RMW_RET_OK;
}


rmw_ret_t
rmw_get_topic_names_and_types(
  const rmw_node_t * node,
  rmw_topic_names_and_types_t * topic_names_and_types)
{
  return get_topic_names_and_types(
    rti_connext_dynamic_identifier,
    node,
    topic_names_and_types);
}

rmw_ret_t
rmw_destroy_topic_names_and_types(
  rmw_topic_names_and_types_t * topic_names_and_types)
{
  if (!topic_names_and_types) {
    RMW_SET_ERROR_MSG("topics handle is null");
    return RMW_RET_ERROR;
  }
  destroy_topic_names_and_types(topic_names_and_types);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_count_publishers(
  const rmw_node_t * node,
  const char * topic_name,
  size_t * count)
{
  return count_publishers(rti_connext_dynamic_identifier, node, topic_name, count);
}

rmw_ret_t
rmw_count_subscribers(
  const rmw_node_t * node,
  const char * topic_name,
  size_t * count)
{
  return count_subscribers(rti_connext_dynamic_identifier, node, topic_name, count);
}

rmw_ret_t
rmw_get_gid_for_publisher(const rmw_publisher_t * publisher, rmw_gid_t * gid)
{
  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier,
    rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)
  if (!gid) {
    RMW_SET_ERROR_MSG("gid is null");
    return RMW_RET_ERROR;
  }

  const CustomPublisherInfo * publisher_info =
    static_cast<const CustomPublisherInfo *>(publisher->data);
  if (!publisher_info) {
    RMW_SET_ERROR_MSG("publisher info handle is null");
    return RMW_RET_ERROR;
  }

  *gid = publisher_info->publisher_gid;
  return RMW_RET_OK;
}

rmw_ret_t
rmw_compare_gids_equal(const rmw_gid_t * gid1, const rmw_gid_t * gid2, bool * result)
{
  if (!gid1) {
    RMW_SET_ERROR_MSG("gid1 is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    gid1,
    gid1->implementation_identifier,
    rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)
  if (!gid2) {
    RMW_SET_ERROR_MSG("gid2 is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    gid2,
    gid2->implementation_identifier,
    rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)
  if (!result) {
    RMW_SET_ERROR_MSG("result is null");
    return RMW_RET_ERROR;
  }
  auto detail1 = reinterpret_cast<const ConnextPublisherGID *>(gid1->data);
  if (!detail1) {
    RMW_SET_ERROR_MSG("gid1 is invalid");
    return RMW_RET_ERROR;
  }
  auto detail2 = reinterpret_cast<const ConnextPublisherGID *>(gid2->data);
  if (!detail2) {
    RMW_SET_ERROR_MSG("gid2 is invalid");
    return RMW_RET_ERROR;
  }
  auto matches =
    DDS_InstanceHandle_equals(&detail1->publication_handle, &detail2->publication_handle);
  *result = (matches == DDS_BOOLEAN_TRUE);
  return RMW_RET_OK;
}

const rmw_guard_condition_t *
rmw_node_get_graph_guard_condition(const rmw_node_t * node)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_dynamic_identifier,
    return nullptr)

  return node_get_graph_guard_condition(node);
}

rmw_ret_t
rmw_service_server_is_available(
  const rmw_node_t * node,
  const rmw_client_t * client,
  bool * is_available)
{
  // TODO(wjwwood): remove this once local graph changes are detected.
  RMW_SET_ERROR_MSG("not implemented");
  return RMW_RET_ERROR;

  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  if (!is_available) {
    RMW_SET_ERROR_MSG("is_available is null");
    return RMW_RET_ERROR;
  }

  ConnextDynamicClientInfo * client_info =
    static_cast<ConnextDynamicClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }

  *is_available = false;
  // In the Connext RPC implementation, a server is ready when:
  //   - At least one server is subscribed to the request topic.
  //   - At least one server is publishing to the reponse topic.
  size_t number_of_request_subscribers = 0;
  rmw_ret_t ret = rmw_count_subscribers(
    node,
    client_info->requester_->get_request_datawriter()->get_topic()->get_name(),
    &number_of_request_subscribers);
  if (ret != RMW_RET_OK) {
    // error string already set
    return ret;
  }
  if (number_of_request_subscribers == 0) {
    // not ready
    return RMW_RET_OK;
  }

  size_t number_of_response_publishers = 0;
  ret = rmw_count_publishers(
    node,
    client_info->response_datareader_->get_topicdescription()->get_name(),
    &number_of_response_publishers);
  if (ret != RMW_RET_OK) {
    // error string already set
    return ret;
  }
  if (number_of_response_publishers == 0) {
    // not ready
    return RMW_RET_OK;
  }

  // all conditions met, there is a service server available
  *is_available = true;
  return RMW_RET_OK;
}
}  // extern "C"
