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

#include <iostream>
#include <stdexcept>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#ifdef __clang__
# pragma clang diagnostic ignored "-Wdeprecated-register"
#endif
#include <ndds/connext_cpp/connext_cpp_replier_details.h>
#include <ndds/connext_cpp/connext_cpp_requester_details.h>
#include <ndds/ndds_cpp.h>
#include <ndds/ndds_requestreply_cpp.h>
#pragma GCC diagnostic pop

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>
#include <rmw/types.h>
// This header is in the rosidl_typesupport_connext_cpp package and
// is in the include/rosidl_typesupport_connext_cpp/impl folder.
#include <rosidl_generator_cpp/message_type_support.hpp>

#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/service_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

// This extern "C" prevents accidental overloading of functions. With this in
// place, overloading produces an error rather than a new C++ symbol.
extern "C"
{

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_EXPORT
const char * rti_connext_dynamic_identifier = "connext_dynamic";

const char *
rmw_get_implementation_identifier()
{
  return rti_connext_dynamic_identifier;
}

rmw_ret_t
rmw_init()
{
  DDSDomainParticipantFactory * dpf_ = DDSDomainParticipantFactory::get_instance();
  if (!dpf_) {
    rmw_set_error_string("  init() could not get participant factory");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_node_t *
rmw_create_node(const char * name)
{
  DDSDomainParticipantFactory * dpf_ = DDSDomainParticipantFactory::get_instance();
  if (!dpf_) {
    rmw_set_error_string("  create_node() could not get participant factory");
    return NULL;
  }

  // use loopback interface to enable cross vendor communication
  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = dpf_->get_default_participant_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("  create_node() could not get participant qos");
    return NULL;
  }
  status = DDSPropertyQosPolicyHelper::add_property(
    participant_qos.property,
    "dds.transport.UDPv4.builtin.ignore_loopback_interface",
    "0",
    DDS_BOOLEAN_FALSE);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("  create_node() could not add qos property");
    return NULL;
  }

  DDS_DomainId_t domain = 0;

  DDSDomainParticipant * participant = dpf_->create_participant(
    //domain, DDS_PARTICIPANT_QOS_DEFAULT, NULL,
    domain, participant_qos, NULL,
    DDS_STATUS_MASK_NONE);
  if (!participant) {
    rmw_set_error_string("  create_node() could not create participant");
    return NULL;
  }

  rmw_node_t * node = new rmw_node_t;
  node->implementation_identifier = rti_connext_dynamic_identifier;
  node->data = participant;
  return node;
}

DDS_TypeCode * create_type_code(
  std::string type_name, const rosidl_typesupport_introspection_cpp::MessageMembers * members,
  DDS_DomainParticipantQos & participant_qos)
{
  DDS_TypeCodeFactory * factory = NULL;
  factory = DDS_TypeCodeFactory::get_instance();
  if (!factory) {
    rmw_set_error_string("  create_type_code() could not get typecode factory");
    throw std::runtime_error("could not get typecode factory");
  }

  DDS_StructMemberSeq struct_members;
  DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
  DDS_TypeCode * type_code = factory->create_struct_tc(type_name.c_str(), struct_members, ex);
  for (unsigned long i = 0; i < members->member_count_; ++i) {
    const rosidl_typesupport_introspection_cpp::MessageMember * member = members->members_ + i;
    const DDS_TypeCode * member_type_code;
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
            max_string_size = member->string_upper_bound_;
          } else {
            // TODO use std::string().max_size() as soon as Connext supports dynamic allocation
            max_string_size = 255;
          }
          member_type_code = factory->create_string_tc(max_string_size, ex);
        }
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE:
        {
          const::rosidl_typesupport_introspection_cpp::MessageMembers * sub_members =
            (const::rosidl_typesupport_introspection_cpp::MessageMembers *)member->members_->data;
          std::string field_type_name = std::string(sub_members->package_name_) + "::msg::dds_::" +
            sub_members->message_name_ + "_";
          member_type_code = create_type_code(field_type_name, sub_members, participant_qos);
        }
        break;
      default:
        printf("unknown type id %u\n", member->type_id_);
        throw std::runtime_error("unknown type");
    }
    if (member->is_array_) {
      if (member->array_size_) {
        if (!member->is_upper_bound_) {
          member_type_code = factory->create_array_tc(member->array_size_, member_type_code, ex);
        } else {
          member_type_code = factory->create_sequence_tc(member->array_size_, member_type_code, ex);
        }
      } else {
        // TODO(dirk-thomas) the default bound of sequences is 100
        member_type_code = factory->create_sequence_tc(100, member_type_code, ex);
      }
    }
    type_code->add_member(
      (std::string(member->name_) + "_").c_str(),
      DDS_TYPECODE_MEMBER_ID_INVALID,
      member_type_code,
      DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, ex);
  }
  // since empty message definitions are not supported
  // we have to add the same dummy field as in rosidl_generator_dds_idl
  if (members->member_count_ == 0) {
    const DDS_TypeCode * member_type_code;
    member_type_code = factory->get_primitive_tc(DDS_TK_BOOLEAN);
    type_code->add_member("_dummy", DDS_TYPECODE_MEMBER_ID_INVALID,
      member_type_code,
      DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, ex);
  }
  //type_code->print_IDL(1, ex);
  DDS_StructMemberSeq_finalize(&struct_members);
  return type_code;
}


struct CustomPublisherInfo
{
  DDSDynamicDataTypeSupport * dynamic_data_type_support_;
  DDSDynamicDataWriter * dynamic_writer_;
  DDS_TypeCode * type_code_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * members_;
  DDS_DynamicData * dynamic_data;
};

rmw_publisher_t *
rmw_create_publisher(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  size_t queue_size)
{
  if (node->implementation_identifier != rti_connext_dynamic_identifier) {
    rmw_set_error_string("node handle not from this implementation");
    // printf("but from: %s\n", node->implementation_identifier);
    return NULL;
  }

  DDSDomainParticipant * participant = (DDSDomainParticipant *)node->data;

  if (type_support->typesupport_identifier !=
    rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier)
  {
    rmw_set_error_string("type support not from this implementation");
    // printf("but from: %s\n", type_support->typesupport_identifier);
    return NULL;
  }

  const rosidl_typesupport_introspection_cpp::MessageMembers * members =
    (rosidl_typesupport_introspection_cpp::MessageMembers *)type_support->data;
  std::string type_name = std::string(members->package_name_) + "::msg::dds_::" +
    members->message_name_ + "_";

  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = participant->get_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get participant qos");
    // printf("get_qos() failed. Status = %d\n", status);
    return NULL;
  }

  DDS_TypeCode * type_code = create_type_code(type_name, members, participant_qos);


  DDSDynamicDataTypeSupport * ddts = new DDSDynamicDataTypeSupport(
    type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
  status = ddts->register_type(participant, type_name.c_str());
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to register type");
    // printf("register_type() failed. Status = %d\n", status);
    return NULL;
  }


  DDS_PublisherQos publisher_qos;
  status = participant->get_default_publisher_qos(publisher_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default publisher qos");
    // printf("get_default_publisher_qos() failed. Status = %d\n", status);
    return NULL;
  }

  DDSPublisher * dds_publisher = participant->create_publisher(
    publisher_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!dds_publisher) {
    rmw_set_error_string("  create_publisher() could not create publisher");
    return NULL;
  }


  DDS_TopicQos default_topic_qos;
  status = participant->get_default_topic_qos(default_topic_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default topic qos");
    // printf("get_default_topic_qos() failed. Status = %d\n", status);
    return NULL;
  }

  DDSTopic * topic = participant->create_topic(
    topic_name, type_name.c_str(), default_topic_qos, NULL,
    DDS_STATUS_MASK_NONE
    );
  if (!topic) {
    rmw_set_error_string("  create_topic() could not create topic");
    return NULL;
  }


  DDS_DataWriterQos datawriter_qos;
  status = participant->get_default_datawriter_qos(datawriter_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default datawriter qos");
    // printf("get_default_datawriter_qos() failed. Status = %d\n", status);
    return NULL;
  }

  // ensure the history depth is at least the requested queue size
  // *INDENT-OFF*
  if (
    datawriter_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    datawriter_qos.history.depth < queue_size)
  // *INDENT-ON*
  {
    datawriter_qos.history.depth = queue_size;
  }

  DDSDataWriter * topic_writer = dds_publisher->create_datawriter(
    topic, datawriter_qos, NULL, DDS_STATUS_MASK_NONE);

  DDSDynamicDataWriter * dynamic_writer = DDSDynamicDataWriter::narrow(topic_writer);
  if (!dynamic_writer) {
    rmw_set_error_string("narrow() failed.");
    return NULL;
  }

  DDS_DynamicData * dynamic_data = ddts->create_data();

  CustomPublisherInfo * custom_publisher_info = new CustomPublisherInfo();
  custom_publisher_info->dynamic_data_type_support_ = ddts;
  custom_publisher_info->dynamic_writer_ = dynamic_writer;
  custom_publisher_info->type_code_ = type_code;
  custom_publisher_info->members_ = members;
  custom_publisher_info->dynamic_data = dynamic_data;

  rmw_publisher_t * publisher = new rmw_publisher_t;
  publisher->implementation_identifier = rti_connext_dynamic_identifier;
  publisher->data = custom_publisher_info;
  return publisher;
}

#define SET_PRIMITIVE_VALUE(TYPE, METHOD_NAME) \
  TYPE value = *((TYPE *)((char *)ros_message + member->offset_)); \
  DDS_ReturnCode_t status = dynamic_data->METHOD_NAME( \
    0, \
    i + 1, \
    value); \
  if (status != DDS_RETCODE_OK) { \
    printf(#METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
    throw std::runtime_error("set member failed"); \
  }

#define ARRAY_SIZE_AND_VALUES(TYPE) \
  TYPE * ros_values = nullptr; \
  size_t array_size; \
  if (member->array_size_ && !member->is_upper_bound_) { \
    ros_values = (TYPE *)((char *)ros_message + member->offset_); \
    array_size = member->array_size_; \
  } else { \
    auto untyped_vector = (void *)((char *)ros_message + member->offset_); \
    auto vector = reinterpret_cast<std::vector<TYPE> *>(untyped_vector); \
    ros_values = vector->data(); \
    array_size = vector->size(); \
  }

#define SET_VALUE(TYPE, METHOD_NAME, ARRAY_METHOD_NAME) \
  { \
    if (member->is_array_) { \
      ARRAY_SIZE_AND_VALUES(TYPE) \
      DDS_ReturnCode_t status = dynamic_data->ARRAY_METHOD_NAME( \
        0, \
        i + 1, \
        array_size, \
        ros_values); \
      if (status != DDS_RETCODE_OK) { \
        printf(#ARRAY_METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
        throw std::runtime_error("set array member failed"); \
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
        values = new DDS_TYPE[array_size]; \
        for (size_t i = 0; i < array_size; ++i) { \
          values[i] = ros_values[i]; \
        } \
      } \
      DDS_ReturnCode_t status = dynamic_data->ARRAY_METHOD_NAME( \
        0, \
        i + 1, \
        array_size, \
        values); \
      if (values) { \
        delete[] values; \
      } \
      if (status != DDS_RETCODE_OK) { \
        printf(#ARRAY_METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
        throw std::runtime_error("set array member failed"); \
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
        auto ros_values = (TYPE *)((char *)ros_message + member->offset_); \
        values = new DDS_TYPE[array_size]; \
        for (size_t i = 0; i < array_size; ++i) { \
          values[i] = ros_values[i]; \
        } \
      } else { \
        auto untyped_vector = (void *)((char *)ros_message + member->offset_); \
        auto vector = reinterpret_cast<std::vector<TYPE> *>(untyped_vector); \
        array_size = vector->size(); \
        if (array_size > 0) { \
          values = new DDS_TYPE[array_size]; \
          for (size_t i = 0; i < array_size; ++i) { \
            values[i] = (*vector)[i]; \
          } \
        } \
      } \
      DDS_ReturnCode_t status = dynamic_data->ARRAY_METHOD_NAME( \
        0, \
        i + 1, \
        array_size, \
        values); \
      if (values) { \
        delete[] values; \
      } \
      if (status != DDS_RETCODE_OK) { \
        printf(#ARRAY_METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
        throw std::runtime_error("set array member failed"); \
      } \
    } else { \
      SET_PRIMITIVE_VALUE(TYPE, METHOD_NAME) \
    } \
  }

#define SET_STRING_VALUE(TYPE, METHOD_NAME) \
  { \
    if (member->is_array_) { \
      DDS_DynamicData dynamic_data_member(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT); \
      DDS_ReturnCode_t status = dynamic_data->bind_complex_member( \
        dynamic_data_member, \
        0, \
        i + 1); \
      if (status != DDS_RETCODE_OK) { \
        printf("bind_complex_member(%s) failed. Status = %d\n", member->name_, status); \
        throw std::runtime_error("set array member failed"); \
      } \
      ARRAY_SIZE_AND_VALUES(TYPE) \
      for (size_t i = 0; i < array_size; ++i) { \
        status = dynamic_data_member.METHOD_NAME( \
          NULL, \
          i + 1, \
          ros_values[i].c_str()); \
        if (status != DDS_RETCODE_OK) { \
          printf(#METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
          throw std::runtime_error("set array member failed"); \
        } \
      } \
      dynamic_data->unbind_complex_member(dynamic_data_member); \
    } else { \
      TYPE value = *((TYPE *)((char *)ros_message + member->offset_)); \
      DDS_ReturnCode_t status = dynamic_data->METHOD_NAME( \
        0, \
        i + 1, \
        value.c_str()); \
      if (status != DDS_RETCODE_OK) { \
        printf(#METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
        throw std::runtime_error("set member failed"); \
      } \
    } \
  }

void _publish(DDS_DynamicData * dynamic_data, const void * ros_message,
  const rosidl_typesupport_introspection_cpp::MessageMembers * members)
{
  //DDS_DynamicData dynamic_data(type_code, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
  //DDS_DynamicData * dynamic_data = ddts->create_data();
  for (unsigned long i = 0; i < members->member_count_; ++i) {
    const rosidl_typesupport_introspection_cpp::MessageMember * member = members->members_ + i;
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
        SET_STRING_VALUE(std::string, set_string)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE:
        {
          DDS_DynamicData sub_dynamic_data(0, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
          dynamic_data->bind_complex_member(
            sub_dynamic_data,
            0,
            i + 1);
          void * sub_ros_message = (void *)((char *)ros_message + member->offset_);
          const::rosidl_typesupport_introspection_cpp::MessageMembers * sub_members =
            (const::rosidl_typesupport_introspection_cpp::MessageMembers *)member->members_->data;
          _publish(&sub_dynamic_data, sub_ros_message, sub_members);
          dynamic_data->unbind_complex_member(sub_dynamic_data);
        }
        break;
      default:
        printf("unknown type id %u\n", member->type_id_);
        throw std::runtime_error("unknown type");
    }
  }
}

rmw_ret_t
rmw_publish(const rmw_publisher_t * publisher, const void * ros_message)
{
  if (publisher->implementation_identifier != rti_connext_dynamic_identifier) {
    rmw_set_error_string("publisher handle not from this implementation");
    // printf("but from: %s\n", publisher->implementation_identifier);
    return RMW_RET_ERROR;
  }

  CustomPublisherInfo * custom_publisher_info = (CustomPublisherInfo *)publisher->data;
  DDSDynamicDataTypeSupport * ddts = custom_publisher_info->dynamic_data_type_support_;
  DDSDynamicDataWriter * dynamic_writer = custom_publisher_info->dynamic_writer_;
  DDS_TypeCode * type_code = custom_publisher_info->type_code_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * members =
    custom_publisher_info->members_;
  DDS_DynamicData * dynamic_data = custom_publisher_info->dynamic_data;

  dynamic_data->clear_all_members();
  _publish(dynamic_data, ros_message, members);

  DDS_ReturnCode_t status = dynamic_writer->write(*dynamic_data, DDS_HANDLE_NIL);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to write");
    // printf("write() failed. Status = %d\n", status);
    return RMW_RET_ERROR;
  }

  //ddts->delete_data(dynamic_data);
  return RMW_RET_OK;
}

struct CustomSubscriberInfo
{
  DDSDynamicDataTypeSupport * dynamic_data_type_support_;
  DDSDynamicDataReader * dynamic_reader_;
  DDS_TypeCode * type_code_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * members_;
  DDS_DynamicData * dynamic_data;
};

rmw_subscription_t *
rmw_create_subscription(const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  size_t queue_size)
{
  if (node->implementation_identifier != rti_connext_dynamic_identifier) {
    rmw_set_error_string("node handle not from this implementation");
    // printf("but from: %s\n", node->implementation_identifier);
    return NULL;
  }

  DDSDomainParticipant * participant = (DDSDomainParticipant *)node->data;

  if (type_support->typesupport_identifier !=
    rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier)
  {
    rmw_set_error_string("type support not from this implementation");
    // printf("but from: %s\n", type_support->typesupport_identifier);
    return NULL;
  }

  const rosidl_typesupport_introspection_cpp::MessageMembers * members =
    (rosidl_typesupport_introspection_cpp::MessageMembers *)type_support->data;
  std::string type_name = std::string(members->package_name_) + "::msg::dds_::" +
    members->message_name_ + "_";

  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = participant->get_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get participant qos");
    // printf("get_qos() failed. Status = %d\n", status);
    return NULL;
  }

  DDS_TypeCode * type_code = create_type_code(type_name, members, participant_qos);


  DDSDynamicDataTypeSupport * ddts = new DDSDynamicDataTypeSupport(
    type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
  status = ddts->register_type(participant, type_name.c_str());
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to register type");
    // printf("register_type() failed. Status = %d\n", status);
    return NULL;
  }


  DDS_SubscriberQos subscriber_qos;
  status = participant->get_default_subscriber_qos(subscriber_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default subscriber qos");
    // printf("get_default_subscriber_qos() failed. Status = %d\n", status);
    return NULL;
  }

  DDSSubscriber * dds_subscriber = participant->create_subscriber(
    subscriber_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!dds_subscriber) {
    rmw_set_error_string("  create_subscriber() could not create subscriber");
    return NULL;
  }


  DDS_TopicQos default_topic_qos;
  status = participant->get_default_topic_qos(default_topic_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default topic qos");
    // printf("get_default_topic_qos() failed. Status = %d\n", status);
    return NULL;
  }

  DDSTopic * topic = participant->create_topic(
    topic_name, type_name.c_str(), default_topic_qos, NULL,
    DDS_STATUS_MASK_NONE
    );
  if (!topic) {
    rmw_set_error_string("  create_topic() could not create topic");
    return NULL;
  }


  DDS_DataReaderQos datareader_qos;
  status = participant->get_default_datareader_qos(datareader_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default datareader qos");
    // printf("get_default_datareader_qos() failed. Status = %d\n", status);
    return NULL;
  }

  // ensure the history depth is at least the requested queue size
  // *INDENT-OFF*
  if (
    datareader_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    datareader_qos.history.depth < queue_size)
  // *INDENT-ON*
  {
    datareader_qos.history.depth = queue_size;
  }

  DDSDataReader * topic_reader = dds_subscriber->create_datareader(
    topic, datareader_qos, NULL, DDS_STATUS_MASK_NONE);

  DDSDynamicDataReader * dynamic_reader = DDSDynamicDataReader::narrow(topic_reader);
  if (!dynamic_reader) {
    rmw_set_error_string("narrow() failed.\n");
    return NULL;
  }

  DDS_DynamicData * dynamic_data = ddts->create_data();

  CustomSubscriberInfo * custom_subscriber_info = new CustomSubscriberInfo();
  custom_subscriber_info->dynamic_data_type_support_ = ddts;
  custom_subscriber_info->dynamic_reader_ = dynamic_reader;
  custom_subscriber_info->type_code_ = type_code;
  custom_subscriber_info->members_ = members;
  custom_subscriber_info->dynamic_data = dynamic_data;

  rmw_subscription_t * subscription = new rmw_subscription_t;
  subscription->implementation_identifier = rti_connext_dynamic_identifier;
  subscription->data = custom_subscriber_info;
  return subscription;
}

#define ARRAY_SIZE() \
  size_t array_size; \
  if (member->array_size_ && !member->is_upper_bound_) { \
    array_size = member->array_size_; \
  } else { \
    DDS_DynamicData dynamic_data_member(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT); \
    DDS_ReturnCode_t status = dynamic_data->bind_complex_member( \
      dynamic_data_member, \
      0, \
      i + 1); \
    if (status != DDS_RETCODE_OK) { \
      printf("bind_complex_member(%s) failed. Status = %d\n", member->name_, status); \
      throw std::runtime_error("set array member failed"); \
    } \
    array_size = dynamic_data_member.get_member_count(); \
    dynamic_data->unbind_complex_member(dynamic_data_member); \
  }

#define ARRAY_RESIZE_AND_VALUES(TYPE) \
  TYPE * ros_values = nullptr; \
  if (member->array_size_ && !member->is_upper_bound_) { \
    ros_values = (TYPE *)((char *)ros_message + member->offset_); \
  } else { \
    auto untyped_vector = (void *)((char *)ros_message + member->offset_); \
    auto vector = reinterpret_cast<std::vector<TYPE> *>(untyped_vector); \
    vector->resize(array_size); \
    ros_values = vector->data(); \
  }

#define GET_VALUE(TYPE, METHOD_NAME, ARRAY_METHOD_NAME) \
  { \
    if (member->is_array_) { \
      ARRAY_SIZE() \
      ARRAY_RESIZE_AND_VALUES(TYPE) \
      DDS_UnsignedLong length = array_size; \
      DDS_ReturnCode_t status = dynamic_data->ARRAY_METHOD_NAME( \
        ros_values, \
        &length, \
        0, \
        i + 1); \
      if (status != DDS_RETCODE_OK) { \
        printf(#METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
        throw std::runtime_error("get array member failed"); \
      } \
    } else { \
      TYPE * value = (TYPE *)((char *)ros_message + member->offset_); \
      DDS_ReturnCode_t status = dynamic_data->METHOD_NAME( \
        *value, \
        0, \
        i + 1); \
      if (status != DDS_RETCODE_OK) { \
        printf(#METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
        throw std::runtime_error("get member failed"); \
      } \
    } \
  }

#define GET_VALUE_WITH_DIFFERENT_TYPES(TYPE, DDS_TYPE, METHOD_NAME, ARRAY_METHOD_NAME) \
  { \
    if (member->is_array_) { \
      ARRAY_SIZE() \
      ARRAY_RESIZE_AND_VALUES(TYPE) \
      if (array_size > 0) { \
        DDS_TYPE * values = new DDS_TYPE[array_size]; \
        DDS_UnsignedLong length = array_size; \
        DDS_ReturnCode_t status = dynamic_data->ARRAY_METHOD_NAME( \
          values, \
          &length, \
          0, \
          i + 1); \
        if (status != DDS_RETCODE_OK) { \
          delete[] values; \
          printf(#ARRAY_METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
          throw std::runtime_error("get array member failed"); \
        } \
        for (size_t i = 0; i < array_size; ++i) { \
          ros_values[i] = values[i]; \
        } \
        delete[] values; \
      } \
    } else { \
      DDS_TYPE value; \
      DDS_ReturnCode_t status = dynamic_data->METHOD_NAME( \
        value, \
        0, \
        i + 1); \
      if (status != DDS_RETCODE_OK) { \
        printf(#METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
        throw std::runtime_error("get member failed"); \
      } \
      TYPE * ros_value = (TYPE *)((char *)ros_message + member->offset_); \
      *ros_value = value; \
    } \
  }

#define GET_VALUE_WITH_BOOL_TYPE(TYPE, DDS_TYPE, METHOD_NAME, ARRAY_METHOD_NAME) \
  { \
    if (member->is_array_) { \
      ARRAY_SIZE() \
      if (array_size > 0) { \
        DDS_TYPE * values = new DDS_TYPE[array_size]; \
        DDS_UnsignedLong length = array_size; \
        DDS_ReturnCode_t status = dynamic_data->ARRAY_METHOD_NAME( \
          values, \
          &length, \
          0, \
          i + 1); \
        if (status != DDS_RETCODE_OK) { \
          delete[] values; \
          printf(#ARRAY_METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
          throw std::runtime_error("get array member failed"); \
        } \
        if (member->array_size_ && !member->is_upper_bound_) { \
          auto ros_values = (TYPE *)((char *)ros_message + member->offset_); \
          for (size_t i = 0; i < array_size; ++i) { \
            ros_values[i] = values[i]; \
          } \
        } else { \
          auto untyped_vector = (void *)((char *)ros_message + member->offset_); \
          auto vector = reinterpret_cast<std::vector<TYPE> *>(untyped_vector); \
          vector->resize(array_size); \
          for (size_t i = 0; i < array_size; ++i) { \
            (*vector)[i] = values[i]; \
          } \
        } \
        delete[] values; \
      } \
    } else { \
      DDS_TYPE value; \
      DDS_ReturnCode_t status = dynamic_data->METHOD_NAME( \
        value, \
        0, \
        i + 1); \
      if (status != DDS_RETCODE_OK) { \
        printf(#METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
        throw std::runtime_error("get member failed"); \
      } \
      TYPE * ros_value = (TYPE *)((char *)ros_message + member->offset_); \
      *ros_value = value; \
    } \
  }

#define GET_STRING_VALUE(TYPE, METHOD_NAME) \
  { \
    if (member->is_array_) { \
      ARRAY_SIZE() \
      ARRAY_RESIZE_AND_VALUES(TYPE) \
      DDS_DynamicData dynamic_data_member(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT); \
      DDS_ReturnCode_t status = dynamic_data->bind_complex_member( \
        dynamic_data_member, \
        0, \
        i + 1); \
      if (status != DDS_RETCODE_OK) { \
        printf("bind_complex_member(%s) failed. Status = %d\n", member->name_, status); \
        throw std::runtime_error("get array member failed"); \
      } \
      for (size_t i = 0; i < array_size; ++i) { \
        char * value = 0; \
        DDS_UnsignedLong size; \
        status = dynamic_data_member.METHOD_NAME( \
          value, \
          &size, \
          NULL, \
          i + 1); \
        if (status != DDS_RETCODE_OK) { \
          if (value) { \
            delete[] value; \
          } \
          printf(#METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
          throw std::runtime_error("get member failed"); \
        } \
        ros_values[i] = value; \
        if (value) { \
          delete[] value; \
        } \
      } \
      dynamic_data->unbind_complex_member(dynamic_data_member); \
    } else { \
      char * value = 0; \
      DDS_UnsignedLong size; \
      DDS_ReturnCode_t status = dynamic_data->METHOD_NAME( \
        value, \
        &size, \
        0, \
        i + 1); \
      if (status != DDS_RETCODE_OK) { \
        if (value) { \
          delete[] value; \
        } \
        printf(#METHOD_NAME "(%s) failed. Status = %d\n", member->name_, status); \
        throw std::runtime_error("get member failed"); \
      } \
      TYPE * ros_value = (TYPE *)((char *)ros_message + member->offset_); \
      *ros_value = value; \
      if (value) { \
        delete[] value; \
      } \
    } \
  }

void _take(DDS_DynamicData * dynamic_data, void * ros_message,
  const rosidl_typesupport_introspection_cpp::MessageMembers * members)
{
  //DDS_DynamicData dynamic_data(type_code, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
  //DDS_DynamicData * dynamic_data = ddts->create_data();
  for (unsigned long i = 0; i < members->member_count_; ++i) {
    const rosidl_typesupport_introspection_cpp::MessageMember * member = members->members_ + i;
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
        GET_STRING_VALUE(std::string, get_string)
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE:
        {
          DDS_DynamicData sub_dynamic_data(0, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
          dynamic_data->bind_complex_member(
            sub_dynamic_data,
            0,
            i + 1);
          void * sub_ros_message = (void *)((char *)ros_message + member->offset_);
          const::rosidl_typesupport_introspection_cpp::MessageMembers * sub_members =
            (const::rosidl_typesupport_introspection_cpp::MessageMembers *)member->members_->data;
          _take(&sub_dynamic_data, sub_ros_message, sub_members);
          dynamic_data->unbind_complex_member(sub_dynamic_data);
        }
        break;
      default:
        printf("unknown type id %u\n", member->type_id_);
        throw std::runtime_error("unknown type");
    }
  }
}

rmw_ret_t
rmw_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken)
{
  if (subscription->implementation_identifier != rti_connext_dynamic_identifier) {
    rmw_set_error_string("subscriber handle not from this implementation");
    // printf("but from: %s\n", subscription->implementation_identifier);
    return RMW_RET_ERROR;
  }

  CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo *)subscription->data;
  DDSDynamicDataTypeSupport * ddts = custom_subscriber_info->dynamic_data_type_support_;
  DDSDynamicDataReader * dynamic_reader = custom_subscriber_info->dynamic_reader_;
  DDS_TypeCode * type_code = custom_subscriber_info->type_code_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * members =
    custom_subscriber_info->members_;

  DDS_DynamicDataSeq dynamic_data_sequence;
  DDS_SampleInfoSeq sample_infos;
  DDS_ReturnCode_t status = dynamic_reader->take(
    dynamic_data_sequence,
    sample_infos,
    1,
    DDS_NOT_READ_SAMPLE_STATE,
    DDS_ANY_VIEW_STATE,
    DDS_ANY_INSTANCE_STATE);
  if (status == DDS_RETCODE_NO_DATA) {
    *taken = false;
    return RMW_RET_OK;
  }
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to take");
    // printf("take() failed. Status = %d\n", status);
    return RMW_RET_ERROR;
  }

  if (ros_message == 0) {
    rmw_set_error_string("take() invoked without a valid ROS message pointer\n");
    return RMW_RET_ERROR;
  }

  _take(&dynamic_data_sequence[0], ros_message, members);

  dynamic_reader->return_loan(dynamic_data_sequence, sample_infos);

  *taken = true;

  return RMW_RET_OK;
}

rmw_guard_condition_t *
rmw_create_guard_condition()
{
  rmw_guard_condition_t * guard_condition_handle = new rmw_guard_condition_t;
  guard_condition_handle->implementation_identifier = rti_connext_dynamic_identifier;
  guard_condition_handle->data = new DDSGuardCondition();
  return guard_condition_handle;
}

rmw_ret_t
rmw_destroy_guard_condition(rmw_guard_condition_t * guard_condition)
{
  if (guard_condition) {
    delete static_cast<DDSGuardCondition *>(guard_condition->data);
    delete guard_condition;
    return RMW_RET_OK;
  }

  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition_handle)
{
  if (guard_condition_handle->implementation_identifier != rti_connext_dynamic_identifier) {
    rmw_set_error_string("guard condition handle not from this implementation");
    // printf("but from: %s\n", guard_condition_handle->implementation_identifier);
    return RMW_RET_ERROR;
  }

  DDSGuardCondition * guard_condition = (DDSGuardCondition *)guard_condition_handle->data;
  guard_condition->set_trigger_value(DDS_BOOLEAN_TRUE);
  return RMW_RET_OK;
}

struct ConnextDynamicServiceInfo
{
  connext::Replier<DDS_DynamicData, DDS_DynamicData> * replier_;
  DDSDataReader * request_datareader_;
  DDS::DynamicDataTypeSupport * response_type_support_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * request_members_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * response_members_;
};

struct ConnextDynamicClientInfo
{
  connext::Requester<DDS_DynamicData, DDS_DynamicData> * requester_;
  DDSDataReader * response_datareader_;
  DDS::DynamicDataTypeSupport * request_type_support_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * request_members_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * response_members_;
};

rmw_ret_t
rmw_wait(
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  bool non_blocking)
{
  DDSWaitSet waitset;

  // add a condition for each subscriber
  for (unsigned long i = 0; i < subscriptions->subscriber_count; ++i) {
    void * data = subscriptions->subscribers[i];
    CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo *)data;
    DDSDynamicDataReader * dynamic_reader = custom_subscriber_info->dynamic_reader_;
    DDSStatusCondition * condition = dynamic_reader->get_statuscondition();
    condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
    waitset.attach_condition(condition);
  }

  // add a condition for each guard condition
  for (unsigned long i = 0; i < guard_conditions->guard_condition_count; ++i) {
    void * data = guard_conditions->guard_conditions[i];
    DDSGuardCondition * guard_condition = (DDSGuardCondition *)data;
    waitset.attach_condition(guard_condition);
  }

  // add a condition for each service
  for (unsigned long i = 0; i < services->service_count; ++i) {
    void * data = services->services[i];
    ConnextDynamicServiceInfo * custom_service_info = (ConnextDynamicServiceInfo *)data;
    DDSDataReader * dynamic_reader = custom_service_info->request_datareader_;
    DDSStatusCondition * condition = dynamic_reader->get_statuscondition();
    condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
    waitset.attach_condition(condition);
  }

  // add a condition for each client
  for (unsigned long i = 0; i < clients->client_count; ++i) {
    void * data = clients->clients[i];
    ConnextDynamicClientInfo * custom_client_info = (ConnextDynamicClientInfo *)data;
    DDSDataReader * dynamic_reader = custom_client_info->response_datareader_;
    DDSStatusCondition * condition = dynamic_reader->get_statuscondition();
    condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
    waitset.attach_condition(condition);
  }

  // invoke wait until one of the conditions triggers
  DDSConditionSeq active_conditions;
  DDS_Duration_t timeout = DDS_Duration_t::from_seconds(non_blocking ? 0 : 1);
  DDS_ReturnCode_t status = DDS_RETCODE_TIMEOUT;
  while (DDS_RETCODE_TIMEOUT == status) {
    status = waitset.wait(active_conditions, timeout);
    if (DDS_RETCODE_TIMEOUT == status) {
      if (non_blocking) {
        break;
      }
      continue;
    }
    if (status != DDS_RETCODE_OK) {
      rmw_set_error_string("fail to wait on waitset");
      // printf("wait() failed. Status = %d\n", status);
      return RMW_RET_ERROR;
    }
  }

  // set subscriber handles to zero for all not triggered conditions
  for (unsigned long i = 0; i < subscriptions->subscriber_count; ++i) {
    void * data = subscriptions->subscribers[i];
    CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo *)data;
    DDSDynamicDataReader * dynamic_reader = custom_subscriber_info->dynamic_reader_;
    DDSCondition * condition = dynamic_reader->get_statuscondition();

    // search for subscriber condition in active set
    unsigned long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == condition) {
        break;
      }
    }
    // if subscriber condition is not found in the active set
    // reset the subscriber handle
    if (!(j < active_conditions.length())) {
      subscriptions->subscribers[i] = 0;
    }
  }

  // set guard condition handles to zero for all not triggered conditions
  for (unsigned long i = 0; i < guard_conditions->guard_condition_count; ++i) {
    void * data = guard_conditions->guard_conditions[i];
    DDSCondition * condition = (DDSCondition *)data;

    // search for guard condition in active set
    unsigned long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == condition) {
        DDSGuardCondition * guard = (DDSGuardCondition *)condition;
        guard->set_trigger_value(DDS_BOOLEAN_FALSE);
        break;
      }
    }
    // if guard condition is not found in the active set
    // reset the guard handle
    if (!(j < active_conditions.length())) {
      guard_conditions->guard_conditions[i] = 0;
    }
  }

  // set service handles to zero for all not triggered conditions
  for (unsigned long i = 0; i < services->service_count; ++i) {
    void * data = services->services[i];
    ConnextDynamicServiceInfo * custom_service_info = (ConnextDynamicServiceInfo *)data;
    DDSDataReader * dynamic_reader = custom_service_info->request_datareader_;
    DDSCondition * condition = dynamic_reader->get_statuscondition();

    // search for service condition in active set
    unsigned long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == condition) {
        break;
      }
    }
    // if service condition is not found in the active set
    // reset the service handle
    if (!(j < active_conditions.length())) {
      services->services[i] = 0;
    }
  }

  // set client handles to zero for all not triggered conditions
  for (unsigned long i = 0; i < clients->client_count; ++i) {
    void * data = clients->clients[i];
    ConnextDynamicClientInfo * custom_client_info = (ConnextDynamicClientInfo *)data;
    DDSDataReader * dynamic_reader = custom_client_info->response_datareader_;
    DDSCondition * condition = dynamic_reader->get_statuscondition();

    // search for client condition in active set
    unsigned long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == condition) {
        break;
      }
    }
    // if client condition is not found in the active set
    // reset the service handle
    if (!(j < active_conditions.length())) {
      clients->clients[i] = 0;
    }
  }

  return RMW_RET_OK;
}

rmw_client_t *
rmw_create_client(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name)
{
  if (node->implementation_identifier != rti_connext_dynamic_identifier) {
    rmw_set_error_string("node handle not from this implementation");
    // printf("but from: %s\n", node->implementation_identifier);
    return NULL;
  }

  DDSDomainParticipant * participant = static_cast<DDSDomainParticipant *>(node->data);

  if (type_support->typesupport_identifier !=
    rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier)
  {
    rmw_set_error_string("type support not from this implementation");
    // printf("but from: %s\n", type_support->typesupport_identifier);
    return NULL;
  }

  const rosidl_typesupport_introspection_cpp::ServiceMembers * service_members =
    (rosidl_typesupport_introspection_cpp::ServiceMembers *)type_support->data;
  const rosidl_typesupport_introspection_cpp::MessageMembers * request_members =
    service_members->request_members_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * response_members =
    service_members->response_members_;
  std::string request_type_name = std::string(request_members->package_name_) + "::srv::dds_::" +
    request_members->message_name_ + "_";
  std::string response_type_name = std::string(response_members->package_name_) + "::srv::dds_::" +
    response_members->message_name_ + "_";

  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = participant->get_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get participant qos");
    // printf("get_qos() failed. Status = %d\n", status);
    return NULL;
  }

  DDS_TypeCode * request_type_code = create_type_code(
    request_type_name, request_members, participant_qos);
  DDS::DynamicDataTypeSupport * request_type_support = new DDS::DynamicDataTypeSupport(
    request_type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);

  DDS_TypeCode * response_type_code = create_type_code(
    response_type_name, response_members, participant_qos);
  DDS::DynamicDataTypeSupport * response_type_support = new DDS::DynamicDataTypeSupport(
    response_type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);

  // create requester
  connext::RequesterParams requester_params(participant);
  requester_params.service_name(service_name);
  requester_params.request_type_support(request_type_support);
  requester_params.reply_type_support(response_type_support);

  connext::Requester<DDS_DynamicData, DDS_DynamicData> * requester(
    new connext::Requester<DDS_DynamicData, DDS_DynamicData>(requester_params));

  DDSDataReader * response_datareader = requester->get_reply_datareader();

  ConnextDynamicClientInfo * client_info = new ConnextDynamicClientInfo();
  client_info->requester_ = requester;
  client_info->response_datareader_ = response_datareader;
  client_info->request_type_support_ = request_type_support;
  client_info->request_members_ = request_members;
  client_info->response_members_ = response_members;

  rmw_client_t * client = new rmw_client_t;
  client->implementation_identifier = rti_connext_dynamic_identifier;
  client->data = client_info;
  return client;
}

rmw_ret_t
rmw_send_request(
  const rmw_client_t * client, const void * ros_request,
  int64_t * sequence_id)
{
  if (client->implementation_identifier != rti_connext_dynamic_identifier) {
    rmw_set_error_string("client handle not from this implementation");
    // printf("but from: %s\n", client->implementation_identifier);
    return RMW_RET_ERROR;
  }

  ConnextDynamicClientInfo * client_info = static_cast<ConnextDynamicClientInfo *>(client->data);
  connext::Requester<DDS_DynamicData, DDS_DynamicData> * requester = client_info->requester_;

  DDS::DynamicData * sample = client_info->request_type_support_->create_data();
  DDS::WriteParams_t writeParams;
  connext::WriteSampleRef<DDS::DynamicData> request(*sample, writeParams);

  _publish(sample, ros_request, client_info->request_members_);

  requester->send_request(request);
  *sequence_id = ((int64_t)request.identity().sequence_number.high) << 32 |
    request.identity().sequence_number.low;

  return RMW_RET_OK;
}

rmw_ret_t
rmw_take_request(
  const rmw_service_t * service,
  void * ros_request_header, void * ros_request, bool * taken)
{
  if (taken == NULL) {
    rmw_set_error_string("taken argument can't be null");
    return RMW_RET_ERROR;
  }

  if (service->implementation_identifier != rti_connext_dynamic_identifier) {
    rmw_set_error_string("service handle not from this implementation");
    // printf("but from: %s\n", service->implementation_identifier);
    return RMW_RET_ERROR;
  }

  ConnextDynamicServiceInfo * service_info = \
    static_cast<ConnextDynamicServiceInfo *>(service->data);
  connext::Replier<DDS_DynamicData, DDS_DynamicData> * replier = service_info->replier_;

  rmw_request_id_t & req_id = *(static_cast<rmw_request_id_t *>(ros_request_header));

  connext::LoanedSamples<DDS::DynamicData> requests = replier->take_requests(1);
  if (requests.begin() != requests.end() && requests.begin()->info().valid_data) {
    _take(&requests.begin()->data(), ros_request, service_info->request_members_);

    size_t SAMPLE_IDENTITY_SIZE = 16;
    memcpy(
      &req_id.writer_guid[0], requests.begin()->identity().writer_guid.value,
      SAMPLE_IDENTITY_SIZE);

    req_id.sequence_number = ((int64_t)requests.begin()->identity().sequence_number.high) << 32 |
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
  void * ros_request_header, void * ros_response, bool * taken)
{
  if (taken == NULL) {
    rmw_set_error_string("taken argument can't be null");
    return RMW_RET_ERROR;
  }

  if (client->implementation_identifier != rti_connext_dynamic_identifier) {
    rmw_set_error_string("client handle not from this implementation");
    // printf("but from: %s\n", client->implementation_identifier);
    return RMW_RET_ERROR;
  }

  ConnextDynamicClientInfo * client_info = \
    static_cast<ConnextDynamicClientInfo *>(client->data);
  connext::Requester<DDS_DynamicData, DDS_DynamicData> * requester = client_info->requester_;

  connext::LoanedSamples<DDS::DynamicData> replies = requester->take_replies(1);
  if (replies.begin() != replies.end() && replies.begin()->info().valid_data) {
    _take(&replies.begin()->data(), ros_response, client_info->response_members_);

    rmw_request_id_t & req_id = *(reinterpret_cast<rmw_request_id_t *>(ros_request_header));
    req_id.sequence_number =
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
  void * ros_request_header, void * ros_response)
{
  if (service->implementation_identifier != rti_connext_dynamic_identifier) {
    rmw_set_error_string("service handle not from this implementation");
    // printf("but from: %s\n", service->implementation_identifier);
    return RMW_RET_ERROR;
  }

  ConnextDynamicServiceInfo * service_info =
    static_cast<ConnextDynamicServiceInfo *>(service->data);
  connext::Replier<DDS_DynamicData, DDS_DynamicData> * replier = service_info->replier_;

  DDS::DynamicData * sample = service_info->response_type_support_->create_data();
  DDS::WriteParams_t writeParams;
  connext::WriteSampleRef<DDS::DynamicData> response(*sample, writeParams);

  _publish(sample, ros_response, service_info->response_members_);

  const rmw_request_id_t & req_id =
    *(reinterpret_cast<const rmw_request_id_t *>(ros_request_header));

  DDS_SampleIdentity_t request_identity;

  size_t SAMPLE_IDENTITY_SIZE = 16;
  memcpy(request_identity.writer_guid.value, &req_id.writer_guid[0], SAMPLE_IDENTITY_SIZE);

  request_identity.sequence_number.high = (int32_t)(
    (req_id.sequence_number & 0xFFFFFFFF00000000) >> 32);
  request_identity.sequence_number.low = (uint32_t)(req_id.sequence_number & 0xFFFFFFFF);

  replier->send_reply(response, request_identity);

  return RMW_RET_OK;
}

rmw_service_t *
rmw_create_service(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name)
{
  if (node->implementation_identifier != rti_connext_dynamic_identifier) {
    rmw_set_error_string("node handle not from this implementation");
    // printf("but from: %s\n", node->implementation_identifier);
    return NULL;
  }

  DDSDomainParticipant * participant = static_cast<DDSDomainParticipant *>(node->data);

  if (type_support->typesupport_identifier !=
    rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier)
  {
    rmw_set_error_string("type support not from this implementation");
    // printf("but from: %s\n", type_support->typesupport_identifier);
    return NULL;
  }

  const rosidl_typesupport_introspection_cpp::ServiceMembers * service_members =
    (rosidl_typesupport_introspection_cpp::ServiceMembers *)type_support->data;
  const rosidl_typesupport_introspection_cpp::MessageMembers * request_members =
    service_members->request_members_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * response_members =
    service_members->response_members_;
  std::string request_type_name = std::string(request_members->package_name_) + "::srv::dds_::" +
    request_members->message_name_ + "_";
  std::string response_type_name = std::string(response_members->package_name_) + "::srv::dds_::" +
    response_members->message_name_ + "_";

  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = participant->get_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get participant qos");
    // printf("get_qos() failed. Status = %d\n", status);
    return NULL;
  }

  DDS_TypeCode * request_type_code = create_type_code(
    request_type_name, request_members, participant_qos);
  DDS::DynamicDataTypeSupport * request_type_support = new DDS::DynamicDataTypeSupport(
    request_type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);

  DDS_TypeCode * response_type_code = create_type_code(
    response_type_name, response_members, participant_qos);
  DDS::DynamicDataTypeSupport * response_type_support = new DDS::DynamicDataTypeSupport(
    response_type_code, DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);

  // create requester
  connext::ReplierParams<DDS_DynamicData, DDS_DynamicData> replier_params(participant);
  replier_params.service_name(service_name);
  replier_params.request_type_support(request_type_support);
  replier_params.reply_type_support(response_type_support);

  connext::Replier<DDS_DynamicData, DDS_DynamicData> * replier(
    new connext::Replier<DDS_DynamicData, DDS_DynamicData>(replier_params));

  DDSDataReader * request_datareader = replier->get_request_datareader();

  ConnextDynamicServiceInfo * server_info = new ConnextDynamicServiceInfo();
  server_info->replier_ = replier;
  server_info->request_datareader_ = request_datareader;
  server_info->response_type_support_ = response_type_support;
  server_info->request_members_ = request_members;
  server_info->response_members_ = response_members;

  rmw_service_t * service = rmw_service_allocate();
  service->implementation_identifier = rti_connext_dynamic_identifier;
  service->data = server_info;
  return service;
}

rmw_ret_t
rmw_destroy_service(rmw_service_t * service)
{
  if (service) {
    // TODO de-allocate Replier and request DataReader
    delete static_cast<ConnextDynamicServiceInfo *>(service->data);
    delete service;
    return RMW_RET_OK;
  }

  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_destroy_client(rmw_client_t * client)
{
  if (client) {
    // TODO de-allocate Requester and response DataReader
    delete static_cast<ConnextDynamicClientInfo *>(client->data);
    delete client;
    return RMW_RET_OK;
  }

  return RMW_RET_ERROR;
}

}  // extern "C"
