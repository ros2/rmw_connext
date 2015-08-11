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

#include <cassert>
#include <exception>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
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

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>
#include <rmw/types.h>
// This header is in the rosidl_typesupport_connext_cpp package and
// is in the include/rosidl_typesupport_connext_cpp/impl folder.
#include <rosidl_generator_cpp/message_type_support.hpp>

#include <rmw/impl/cpp/macros.hpp>

#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/service_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_LOCAL
inline std::string
_create_type_name(
  const rosidl_typesupport_introspection_cpp::MessageMembers * members,
  const std::string & sep)
{
  return
    std::string(members->package_name_) + "::" + sep + "::dds_::" + members->message_name_ + "_";
}

// This extern "C" prevents accidental overloading of functions. With this in
// place, overloading produces an error rather than a new C++ symbol.
extern "C"
{

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_EXPORT
const char * rti_connext_dynamic_identifier = "connext_dynamic";

class CustomPublisherListener
  : public DDSDataReaderListener
{
public:
  virtual void on_data_available(DDSDataReader * reader)
  {
    DDSPublicationBuiltinTopicDataDataReader * builtin_reader =
      static_cast<DDSPublicationBuiltinTopicDataDataReader *>(reader);

    DDS_PublicationBuiltinTopicDataSeq data_seq;
    DDS_SampleInfoSeq info_seq;
    DDS_ReturnCode_t retcode = builtin_reader->take(
      data_seq, info_seq, DDS_LENGTH_UNLIMITED,
      DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);

    if (retcode == DDS_RETCODE_NO_DATA) {
      return;
    }
    if (retcode != DDS_RETCODE_OK) {
      fprintf(stderr, "failed to access data from the built-in reader\n");
      return;
    }

    for (auto i = 0; i < data_seq.length(); ++i) {
      if (info_seq[i].valid_data) {
        auto & topic_types = topic_names_and_types[data_seq[i].topic_name];
        topic_types.push_back(data_seq[i].type_name);
      } else {
        // TODO(dirk-thomas) remove related topic name / type
      }
    }

    builtin_reader->return_loan(data_seq, info_seq);
  }
  std::map<std::string, std::vector<std::string>> topic_names_and_types;
};

class CustomSubscriberListener
  : public DDSDataReaderListener
{
public:
  virtual void on_data_available(DDSDataReader * reader)
  {
    DDSSubscriptionBuiltinTopicDataDataReader * builtin_reader =
      static_cast<DDSSubscriptionBuiltinTopicDataDataReader *>(reader);

    DDS_SubscriptionBuiltinTopicDataSeq data_seq;
    DDS_SampleInfoSeq info_seq;
    DDS_ReturnCode_t retcode = builtin_reader->take(
      data_seq, info_seq, DDS_LENGTH_UNLIMITED,
      DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);

    if (retcode == DDS_RETCODE_NO_DATA) {
      return;
    }
    if (retcode != DDS_RETCODE_OK) {
      fprintf(stderr, "failed to access data from the built-in reader\n");
      return;
    }

    for (auto i = 0; i < data_seq.length(); ++i) {
      if (info_seq[i].valid_data) {
        auto & topic_types = topic_names_and_types[data_seq[i].topic_name];
        topic_types.push_back(data_seq[i].type_name);
      } else {
        // TODO(dirk-thomas) remove related topic name / type
      }
    }

    builtin_reader->return_loan(data_seq, info_seq);
  }
  std::map<std::string, std::vector<std::string>> topic_names_and_types;
};

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
    RMW_SET_ERROR_MSG("failed to get participant factory");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

struct CustomNodeInfo
{
  DDSDomainParticipant * participant;
  CustomPublisherListener * publisher_listener;
  CustomSubscriberListener * subscriber_listener;
};

rmw_node_t *
rmw_create_node(const char * name, size_t domain_id)
{
  (void)name;

  DDSDomainParticipantFactory * dpf_ = DDSDomainParticipantFactory::get_instance();
  if (!dpf_) {
    RMW_SET_ERROR_MSG("failed to get participant factory");
    return NULL;
  }

  // use loopback interface to enable cross vendor communication
  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = dpf_->get_default_participant_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default participant qos");
    return NULL;
  }
  status = DDSPropertyQosPolicyHelper::add_property(
    participant_qos.property,
    "dds.transport.UDPv4.builtin.ignore_loopback_interface",
    "0",
    DDS_BOOLEAN_FALSE);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to add qos property");
    return NULL;
  }
  status = DDSPropertyQosPolicyHelper::add_property(
    participant_qos.property,
    "dds.transport.use_510_compatible_locator_kinds",
    "1",
    DDS_BOOLEAN_FALSE);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to add qos property");
    return NULL;
  }

  DDS_DomainId_t domain = static_cast<DDS_DomainId_t>(domain_id);

  DDSDomainParticipant * participant = dpf_->create_participant(
    domain, participant_qos, NULL,
    DDS_STATUS_MASK_NONE);
  if (!participant) {
    RMW_SET_ERROR_MSG("failed to create participant");
    return NULL;
  }

  rmw_node_t * node = nullptr;
  CustomNodeInfo * node_info = nullptr;
  CustomPublisherListener * publisher_listener = nullptr;
  CustomSubscriberListener * subscriber_listener = nullptr;
  void * buf = nullptr;

  DDSDataReader * data_reader = nullptr;
  DDSPublicationBuiltinTopicDataDataReader * builtin_publication_datareader = nullptr;
  DDSSubscriptionBuiltinTopicDataDataReader * builtin_subscription_datareader = nullptr;
  DDSSubscriber * builtin_subscriber = participant->get_builtin_subscriber();
  if (!builtin_subscriber) {
    RMW_SET_ERROR_MSG("builtin subscriber handle is null");
    goto fail;
  }

  // setup publisher listener
  data_reader = builtin_subscriber->lookup_datareader(DDS_PUBLICATION_TOPIC_NAME);
  builtin_publication_datareader =
    static_cast<DDSPublicationBuiltinTopicDataDataReader *>(data_reader);
  if (!builtin_publication_datareader) {
    RMW_SET_ERROR_MSG("builtin publication datareader handle is null");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CustomPublisherListener));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(publisher_listener, buf, goto fail, CustomPublisherListener)
  buf = nullptr;
  builtin_publication_datareader->set_listener(publisher_listener, DDS_DATA_AVAILABLE_STATUS);

  data_reader = builtin_subscriber->lookup_datareader(DDS_SUBSCRIPTION_TOPIC_NAME);
  builtin_subscription_datareader =
    static_cast<DDSSubscriptionBuiltinTopicDataDataReader *>(data_reader);
  if (!builtin_subscription_datareader) {
    RMW_SET_ERROR_MSG("builtin subscription datareader handle is null");
    goto fail;
  }

  // setup subscriber listener
  buf = rmw_allocate(sizeof(CustomSubscriberListener));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(subscriber_listener, buf, goto fail, CustomSubscriberListener)
  buf = nullptr;
  builtin_subscription_datareader->set_listener(subscriber_listener, DDS_DATA_AVAILABLE_STATUS);

  node = rmw_node_allocate();
  if (!node) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node handle");
    goto fail;
  }
  node->implementation_identifier = rti_connext_dynamic_identifier;
  node->data = participant;


  buf = rmw_allocate(sizeof(CustomNodeInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(node_info, buf, goto fail, CustomNodeInfo)
  buf = nullptr;
  node_info->participant = participant;
  node_info->publisher_listener = publisher_listener;
  node_info->subscriber_listener = subscriber_listener;

  node->implementation_identifier = rti_connext_dynamic_identifier;
  node->data = node_info;
  return node;
fail:
  status = dpf_->delete_participant(participant);
  if (status != DDS_RETCODE_OK) {
    std::stringstream ss;
    ss << "leaking participant while handling failure at " <<
      __FILE__ << ":" << __LINE__;
    (std::cerr << ss.str()).flush();
  }
  if (publisher_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      publisher_listener->~CustomPublisherListener(), CustomPublisherListener)
    rmw_free(publisher_listener);
  }
  if (subscriber_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      subscriber_listener->~CustomSubscriberListener(), CustomSubscriberListener)
    rmw_free(subscriber_listener);
  }
  if (node) {
    rmw_free(node);
  }
  if (node_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->~CustomNodeInfo(), CustomNodeInfo)
    rmw_free(node_info);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

rmw_ret_t
rmw_destroy_node(rmw_node_t * node)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  DDSDomainParticipantFactory * dpf_ = DDSDomainParticipantFactory::get_instance();
  if (!dpf_) {
    RMW_SET_ERROR_MSG("failed to get participant factory");
    return RMW_RET_ERROR;
  }

  auto node_info = static_cast<CustomNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  auto participant = static_cast<DDSDomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
  }
  // This unregisters types and destroys topics which were shared between
  // publishers and subscribers and could not be cleaned up in the delete functions.
  participant->delete_contained_entities();

  DDS_ReturnCode_t ret = dpf_->delete_participant(participant);
  if (ret != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete participant");
    return RMW_RET_ERROR;
  }

  if (node_info->publisher_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->publisher_listener->~CustomPublisherListener(), CustomPublisherListener)
    rmw_free(node_info->publisher_listener);
    node_info->publisher_listener = nullptr;
  }
  if (node_info->subscriber_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->subscriber_listener->~CustomSubscriberListener(), CustomSubscriberListener)
    rmw_free(node_info->subscriber_listener);
    node_info->subscriber_listener = nullptr;
  }

  rmw_free(node_info);
  node->data = nullptr;
  rmw_node_free(node);

  return RMW_RET_OK;
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

DDS_TypeCode * create_type_code(
  std::string type_name, const rosidl_typesupport_introspection_cpp::MessageMembers * members,
  DDS_DomainParticipantQos & participant_qos)
{
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
  for (unsigned long i = 0; i < members->member_count_; ++i) {
    const rosidl_typesupport_introspection_cpp::MessageMember * member = members->members_ + i;
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
            // TODO use std::string().max_size() as soon as Connext supports dynamic allocation
            max_string_size = 255;
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
          auto sub_members =
            static_cast<const::rosidl_typesupport_introspection_cpp::MessageMembers *>(
            member->members_->data);
          if (!sub_members) {
            RMW_SET_ERROR_MSG("sub members handle is null");
            return NULL;
          }
          std::string field_type_name = _create_type_name(sub_members, "msg");
          member_type_code = create_type_code(field_type_name, sub_members, participant_qos);
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
        // TODO(dirk-thomas) the default bound of sequences is 100
        member_type_code_non_const = factory->create_sequence_tc(100, member_type_code, ex);
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

bool
get_datareader_qos(DDSDomainParticipant * participant, DDS_DataReaderQos & datareader_qos)
{
  DDS_ReturnCode_t status = participant->get_default_datareader_qos(datareader_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default datareader qos");
    return false;
  }

  status = DDSPropertyQosPolicyHelper::add_property(
    datareader_qos.property,
    "dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size",
    "4096",
    DDS_BOOLEAN_FALSE);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to add qos property");
    return false;
  }

  status = DDSPropertyQosPolicyHelper::add_property(
    datareader_qos.property,
    "reader_resource_limits.dynamically_allocate_fragmented_samples",
    "1",
    DDS_BOOLEAN_FALSE);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to add qos property");
    return false;
  }
  return true;
}

bool
get_datawriter_qos(DDSDomainParticipant * participant, DDS_DataWriterQos & datawriter_qos)
{
  DDS_ReturnCode_t status = participant->get_default_datawriter_qos(datawriter_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default datawriter qos");
    return false;
  }

  status = DDSPropertyQosPolicyHelper::add_property(
    datawriter_qos.property,
    "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
    "4096",
    DDS_BOOLEAN_FALSE);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to add qos property");
    return false;
  }
  return true;
}

struct CustomPublisherInfo
{
  DDSDynamicDataTypeSupport * dynamic_data_type_support_;
  DDSPublisher * dds_publisher_;
  DDSDataWriter * data_writer_;
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
  const rmw_qos_profile_t & qos_profile)
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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support handle,
    type_support->typesupport_identifier,
    rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier,
    return NULL)

  auto node_info = static_cast<CustomNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDSDomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  auto members = static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
    type_support->data);
  if (!members) {
    RMW_SET_ERROR_MSG("members handle is null");
    return NULL;
  }
  std::string type_name = _create_type_name(members, "msg");

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
  // Start initializing elements.
  publisher = rmw_publisher_allocate();
  if (!publisher) {
    RMW_SET_ERROR_MSG("failed to allocate memory for publisher");
    goto fail;
  }

  type_code = create_type_code(type_name, members, participant_qos);
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

  if (!get_datawriter_qos(participant, datawriter_qos)) {
    // error string was set within the function
    goto fail;
  }

  switch (qos_profile.history) {
    case RMW_QOS_POLICY_KEEP_LAST_HISTORY:
      datawriter_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_KEEP_ALL_HISTORY:
      datawriter_qos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS history policy");
      goto fail;
  }

  switch (qos_profile.reliability) {
    case RMW_QOS_POLICY_BEST_EFFORT:
      datawriter_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABLE:
      datawriter_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS reliability policy");
      goto fail;
  }

  // ensure the history depth is at least the requested queue size
  assert(datawriter_qos.history.depth >= 0);
  if (
    datawriter_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    static_cast<size_t>(datawriter_qos.history.depth) < qos_profile.depth
  )
  {
    if (qos_profile.depth > (std::numeric_limits<DDS_Long>::max)()) {
      RMW_SET_ERROR_MSG("requested queue size exceeds maximum DDS history depth");
      goto fail;
    }
    datawriter_qos.history.depth = static_cast<DDS_Long>(qos_profile.depth);
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
  custom_publisher_info->members_ = members;
  custom_publisher_info->dynamic_data = dynamic_data;

  publisher->implementation_identifier = rti_connext_dynamic_identifier;
  publisher->data = custom_publisher_info;
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
      // Cannot unregister type here, in case another topic is using the same type.
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
       */
      // Call destructor directly since we used a placement new to construct it.
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
        ddts->~DDSDynamicDataTypeSupport(), DDSDynamicDataTypeSupport)
      rmw_free(ddts);
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

  auto node_info = static_cast<CustomNodeInfo *>(node->data);
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
      std::string type_name = _create_type_name(custom_publisher_info->members_, "msg");
      // Cannot unregister type here, in case another topic is using the same type.
      // Should be cleaned up when the node is destroyed.
      // If we figure out a way to unregister types when they are not being used, then we can
      // add this code back in:
      /*
         if (ddts->unregister_type(participant, type_name.c_str()) != DDS_RETCODE_OK) {
         RMW_SET_ERROR_MSG(("failed to unregister type: " + type_name).c_str());
         return RMW_RET_ERROR;
         }
       */
      if (custom_publisher_info->dynamic_data) {
        if (ddts->delete_data(custom_publisher_info->dynamic_data) != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete dynamic data");
          return RMW_RET_ERROR;
        }
      }
      custom_publisher_info->dynamic_data = nullptr;
      rmw_free(ddts);
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
  publisher->data = nullptr;
  rmw_publisher_free(publisher);
  return RMW_RET_OK;
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
          reinterpret_cast<const TYPE *>(static_cast<const char *>(ros_message) + member->offset_); \
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

#define SET_STRING_VALUE(TYPE, METHOD_NAME) \
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
          ros_values[j].c_str()); \
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
        value->c_str()); \
      if (status != DDS_RETCODE_OK) { \
        RMW_SET_ERROR_MSG("failed to set value using " #METHOD_NAME); \
        return false; \
      } \
    } \
  }

#define SET_SUBMESSAGE_VALUE(dynamic_data, i) \
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
  auto sub_members = static_cast<const::rosidl_typesupport_introspection_cpp::MessageMembers *>( \
    member->members_->data); \
  if (!sub_members) { \
    RMW_SET_ERROR_MSG("sub members handle is null"); \
    return false; \
  } \
  bool published = _publish(&sub_dynamic_data, sub_ros_message, sub_members); \
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

bool _publish(
  DDS_DynamicData * dynamic_data, const void * ros_message,
  const rosidl_typesupport_introspection_cpp::MessageMembers * members)
{
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
              SET_SUBMESSAGE_VALUE(array_data_ptr, j)
            }
            status = dynamic_data->unbind_complex_member(array_data);
            if (status != DDS_RETCODE_OK) {
              RMW_SET_ERROR_MSG("failed to unbind complex member");
              return false;
            }
          } else {
            SET_SUBMESSAGE_VALUE(dynamic_data, i)
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
  const rosidl_typesupport_introspection_cpp::MessageMembers * members = publisher_info->members_;
  if (!members) {
    RMW_SET_ERROR_MSG("members handle is null");
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
  bool published = _publish(dynamic_data, ros_message, members);
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

struct CustomSubscriberInfo
{
  DDSDynamicDataTypeSupport * dynamic_data_type_support_;
  DDSDynamicDataReader * dynamic_reader_;
  DDSDataReader * data_reader_;
  DDSReadCondition * read_condition_;
  DDSSubscriber * dds_subscriber_;
  bool ignore_local_publications;
  DDS_TypeCode * type_code_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * members_;
  DDS_DynamicData * dynamic_data;
};

rmw_subscription_t *
rmw_create_subscription(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t & qos_profile,
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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support handle,
    type_support->typesupport_identifier,
    rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier,
    return NULL)

  auto node_info = static_cast<CustomNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDSDomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  auto members = static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
    type_support->data);
  if (!members) {
    RMW_SET_ERROR_MSG("members handle is null");
    return NULL;
  }
  std::string type_name = _create_type_name(members, "msg");

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

  type_code = create_type_code(type_name, members, participant_qos);
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

  if (!get_datareader_qos(participant, datareader_qos)) {
    // error string was set within the function
    goto fail;
  }

  switch (qos_profile.history) {
    case RMW_QOS_POLICY_KEEP_LAST_HISTORY:
      datareader_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_KEEP_ALL_HISTORY:
      datareader_qos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS history policy");
      goto fail;
  }

  switch (qos_profile.reliability) {
    case RMW_QOS_POLICY_BEST_EFFORT:
      datareader_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABLE:
      datareader_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS reliability policy");
      goto fail;
  }

  // ensure the history depth is at least the requested queue size
  assert(datareader_qos.history.depth >= 0);
  if (
    datareader_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    static_cast<size_t>(datareader_qos.history.depth) < qos_profile.depth)
  {
    if (qos_profile.depth > (std::numeric_limits<DDS_Long>::max)()) {
      RMW_SET_ERROR_MSG("requested queue size exceeds maximum DDS history depth");
      goto fail;
    }
    datareader_qos.history.depth = static_cast<DDS_Long>(qos_profile.depth);
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
  custom_subscriber_info->members_ = members;
  custom_subscriber_info->dynamic_data = dynamic_data;

  subscription->implementation_identifier = rti_connext_dynamic_identifier;
  subscription->data = custom_subscriber_info;
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
      // Cannot unregister type here, in case another topic is using the same type.
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
       */
      // Call destructor directly since we used a placement new to construct it.
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
        ddts->~DDSDynamicDataTypeSupport(), DDSDynamicDataTypeSupport)
      rmw_free(ddts);
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

  auto node_info = static_cast<CustomNodeInfo *>(node->data);
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
      std::string type_name = _create_type_name(custom_subscription_info->members_, "msg");
      // Cannot unregister type here, in case another topic is using the same type.
      // Should be cleaned up when the node is destroyed.
      // If we figure out a way to unregister types when they are not being used, then we can
      // add this code back in:
      /*
         if (ddts->unregister_type(participant, type_name.c_str()) != DDS_RETCODE_OK) {
         RMW_SET_ERROR_MSG(("failed to unregister type: " + type_name).c_str());
         return RMW_RET_ERROR;
         }
       */
      if (custom_subscription_info->dynamic_data) {
        if (ddts->delete_data(custom_subscription_info->dynamic_data) != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete dynamic data");
          return RMW_RET_ERROR;
        }
      }
      custom_subscription_info->dynamic_data = nullptr;
      rmw_free(ddts);
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
  rmw_subscription_free(subscription);
  return RMW_RET_OK;
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
      auto ros_value = reinterpret_cast<TYPE *>(static_cast<char *>(ros_message) + member->offset_); \
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
      auto ros_value = reinterpret_cast<TYPE *>(static_cast<char *>(ros_message) + member->offset_); \
      *ros_value = value == DDS_BOOLEAN_TRUE; \
    } \
  }

#define GET_STRING_VALUE(TYPE, METHOD_NAME) \
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
        ros_values[j] = value; \
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
      auto ros_value = reinterpret_cast<TYPE *>(static_cast<char *>(ros_message) + member->offset_); \
      *ros_value = value; \
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
  auto sub_members = static_cast<const::rosidl_typesupport_introspection_cpp::MessageMembers *>( \
    member->members_->data); \
  if (!sub_members) { \
    RMW_SET_ERROR_MSG("sub members handle is null"); \
    return false; \
  } \
  bool success = _take(&sub_dynamic_data, sub_ros_message, sub_members); \
  status = dynamic_data->unbind_complex_member(sub_dynamic_data); \
  if (!success) { \
    return false; \
  } \
  if (status != DDS_RETCODE_OK) { \
    RMW_SET_ERROR_MSG("failed to unbind complex member"); \
    return false; \
  }

bool _take(DDS_DynamicData * dynamic_data, void * ros_message,
  const rosidl_typesupport_introspection_cpp::MessageMembers * members)
{
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

rmw_ret_t
rmw_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken)
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
  const rosidl_typesupport_introspection_cpp::MessageMembers * members =
    subscriber_info->members_;
  if (!members) {
    RMW_SET_ERROR_MSG("members handle is null");
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

  bool success = true;
  if (!ignore_sample) {
    success = _take(&dynamic_data_sequence[0], ros_message, members);
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

rmw_guard_condition_t *
rmw_create_guard_condition()
{
  rmw_guard_condition_t * guard_condition_handle = rmw_guard_condition_allocate();
  if (!guard_condition_handle) {
    RMW_SET_ERROR_MSG("failed to allocate memory for guard condition");
    return NULL;
  }
  guard_condition_handle->implementation_identifier = rti_connext_dynamic_identifier;
  // Allocate memory for the DDSGuardCondition object.
  void * buf = rmw_allocate(sizeof(DDSGuardCondition));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the DDSGuardCondition in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(guard_condition_handle->data, buf, goto fail, DDSGuardCondition)
  buf = nullptr;
  return guard_condition_handle;
fail:
  if (guard_condition_handle) {
    rmw_guard_condition_free(guard_condition_handle);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

rmw_ret_t
rmw_destroy_guard_condition(rmw_guard_condition_t * guard_condition)
{
  if (!guard_condition) {
    RMW_SET_ERROR_MSG("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    guard condition handle,
    guard_condition->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  if (guard_condition->data) {
    DDSGuardCondition * dds_gc = static_cast<DDSGuardCondition *>(guard_condition->data);
    RMW_TRY_DESTRUCTOR(dds_gc->~DDSGuardCondition(), DDSGuardCondition, result = RMW_RET_ERROR)
    rmw_free(guard_condition->data);
  }
  rmw_guard_condition_free(guard_condition);
  return result;
}

rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition_handle)
{
  if (!guard_condition_handle) {
    RMW_SET_ERROR_MSG("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    guard condition handle,
    guard_condition_handle->implementation_identifier, rti_connext_dynamic_identifier,
    return RMW_RET_ERROR)

  DDSGuardCondition * guard_condition =
    static_cast<DDSGuardCondition *>(guard_condition_handle->data);
  if (!guard_condition) {
    RMW_SET_ERROR_MSG("guard condition is null");
    return RMW_RET_ERROR;
  }
  DDS_ReturnCode_t status = guard_condition->set_trigger_value(DDS_BOOLEAN_TRUE);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to set trigger value");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

struct ConnextDynamicServiceInfo
{
  connext::Replier<DDS_DynamicData, DDS_DynamicData> * replier_;
  DDSDataReader * request_datareader_;
  DDS::DynamicDataTypeSupport * request_type_support_;
  DDS::DynamicDataTypeSupport * response_type_support_;
  DDS_TypeCode * response_type_code_;
  DDS_TypeCode * request_type_code_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * request_members_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * response_members_;
};

struct ConnextDynamicClientInfo
{
  connext::Requester<DDS_DynamicData, DDS_DynamicData> * requester_;
  DDSDataReader * response_datareader_;
  DDS::DynamicDataTypeSupport * request_type_support_;
  DDS::DynamicDataTypeSupport * response_type_support_;
  DDS_TypeCode * response_type_code_;
  DDS_TypeCode * request_type_code_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * request_members_;
  const rosidl_typesupport_introspection_cpp::MessageMembers * response_members_;
};

rmw_ret_t
rmw_wait(
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_time_t * wait_timeout)
{
  DDSWaitSet waitset;

  // add a condition for each subscriber
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
    CustomSubscriberInfo * subscriber_info =
      static_cast<CustomSubscriberInfo *>(subscriptions->subscribers[i]);
    if (!subscriber_info) {
      RMW_SET_ERROR_MSG("subscriber info handle is null");
      return RMW_RET_ERROR;
    }
    DDSReadCondition * read_condition = subscriber_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }
    DDS_ReturnCode_t status = waitset.attach_condition(read_condition);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to attach condition");
      return RMW_RET_ERROR;
    }
  }

  // add a condition for each guard condition
  for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
    DDSGuardCondition * guard_condition =
      static_cast<DDSGuardCondition *>(guard_conditions->guard_conditions[i]);
    if (!guard_condition) {
      RMW_SET_ERROR_MSG("guard condition handle is null");
      return RMW_RET_ERROR;
    }
    DDS_ReturnCode_t status = waitset.attach_condition(guard_condition);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to attach condition");
      return RMW_RET_ERROR;
    }
  }

  // add a condition for each service
  for (size_t i = 0; i < services->service_count; ++i) {
    ConnextDynamicServiceInfo * service_info =
      static_cast<ConnextDynamicServiceInfo *>(services->services[i]);
    if (!service_info) {
      RMW_SET_ERROR_MSG("service info handle is null");
      return RMW_RET_ERROR;
    }
    DDSDataReader * dynamic_reader = service_info->request_datareader_;
    if (!dynamic_reader) {
      RMW_SET_ERROR_MSG("request datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDSStatusCondition * condition = dynamic_reader->get_statuscondition();
    if (!condition) {
      RMW_SET_ERROR_MSG("condition handle is null");
      return RMW_RET_ERROR;
    }
    DDS_ReturnCode_t status = condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to set enabled statuses");
      return RMW_RET_ERROR;
    }
    status = waitset.attach_condition(condition);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to attach condition");
      return RMW_RET_ERROR;
    }
  }

  // add a condition for each client
  for (size_t i = 0; i < clients->client_count; ++i) {
    ConnextDynamicClientInfo * client_info =
      static_cast<ConnextDynamicClientInfo *>(clients->clients[i]);
    if (!client_info) {
      RMW_SET_ERROR_MSG("client info handle is null");
      return RMW_RET_ERROR;
    }
    DDSDataReader * dynamic_reader = client_info->response_datareader_;
    if (!dynamic_reader) {
      RMW_SET_ERROR_MSG("response datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDSStatusCondition * condition = dynamic_reader->get_statuscondition();
    if (!condition) {
      RMW_SET_ERROR_MSG("condition handle is null");
      return RMW_RET_ERROR;
    }
    DDS_ReturnCode_t status = condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to set enabled statuses");
      return RMW_RET_ERROR;
    }
    status = waitset.attach_condition(condition);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to attach condition");
      return RMW_RET_ERROR;
    }
  }

  // invoke wait until one of the conditions triggers
  DDSConditionSeq active_conditions;
  DDS_Duration_t timeout;
  if (!wait_timeout) {
    timeout = DDS_DURATION_INFINITE;
  } else {
    timeout.sec = static_cast<DDS_Long>(wait_timeout->sec);
    timeout.nanosec = static_cast<DDS_Long>(wait_timeout->nsec);
  }

  DDS_ReturnCode_t status = waitset.wait(active_conditions, timeout);

  if (DDS_RETCODE_TIMEOUT == status) {
    return RMW_RET_TIMEOUT;
  }

  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("fail to wait on waitset");
    return RMW_RET_ERROR;
  }

  // set subscriber handles to zero for all not triggered conditions
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
    CustomSubscriberInfo * subscriber_info =
      static_cast<CustomSubscriberInfo *>(subscriptions->subscribers[i]);
    if (!subscriber_info) {
      RMW_SET_ERROR_MSG("subscriber info handle is null");
      return RMW_RET_ERROR;
    }
    DDSReadCondition * read_condition = subscriber_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }

    // search for subscriber condition in active set
    DDS_Long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == read_condition) {
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
  for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
    DDSCondition * condition =
      static_cast<DDSCondition *>(guard_conditions->guard_conditions[i]);
    if (!condition) {
      RMW_SET_ERROR_MSG("condition handle is null");
      return RMW_RET_ERROR;
    }

    // search for guard condition in active set
    DDS_Long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == condition) {
        auto guard = static_cast<DDSGuardCondition *>(condition);
        DDS_ReturnCode_t status = guard->set_trigger_value(DDS_BOOLEAN_FALSE);
        if (status != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to set trigger value");
          return RMW_RET_ERROR;
        }
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
  for (size_t i = 0; i < services->service_count; ++i) {
    ConnextDynamicServiceInfo * service_info =
      static_cast<ConnextDynamicServiceInfo *>(services->services[i]);
    if (!service_info) {
      RMW_SET_ERROR_MSG("service info handle is null");
      return RMW_RET_ERROR;
    }
    DDSDataReader * dynamic_reader = service_info->request_datareader_;
    if (!dynamic_reader) {
      RMW_SET_ERROR_MSG("request datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDSCondition * condition = dynamic_reader->get_statuscondition();
    if (!condition) {
      RMW_SET_ERROR_MSG("condition handle is null");
      return RMW_RET_ERROR;
    }

    // search for service condition in active set
    DDS_Long j = 0;
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
  for (size_t i = 0; i < clients->client_count; ++i) {
    ConnextDynamicClientInfo * client_info =
      static_cast<ConnextDynamicClientInfo *>(clients->clients[i]);
    if (!client_info) {
      RMW_SET_ERROR_MSG("client info handle is null");
      return RMW_RET_ERROR;
    }
    DDSDataReader * dynamic_reader = client_info->response_datareader_;
    if (!dynamic_reader) {
      RMW_SET_ERROR_MSG("response datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDSCondition * condition = dynamic_reader->get_statuscondition();
    if (!condition) {
      RMW_SET_ERROR_MSG("condition handle is null");
      return RMW_RET_ERROR;
    }

    // search for client condition in active set
    DDS_Long j = 0;
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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support handle,
    type_support->typesupport_identifier,
    rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier,
    return NULL)

  auto node_info = static_cast<CustomNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDSDomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  auto service_members = static_cast<const rosidl_typesupport_introspection_cpp::ServiceMembers *>(
    type_support->data);
  if (!service_members) {
    RMW_SET_ERROR_MSG("service members handle is null");
    return NULL;
  }
  const rosidl_typesupport_introspection_cpp::MessageMembers * request_members =
    service_members->request_members_;
  if (!request_members) {
    RMW_SET_ERROR_MSG("request members handle is null");
    return NULL;
  }
  const rosidl_typesupport_introspection_cpp::MessageMembers * response_members =
    service_members->response_members_;
  if (!response_members) {
    RMW_SET_ERROR_MSG("response members handle is null");
    return NULL;
  }
  std::string request_type_name = _create_type_name(request_members, "srv");
  std::string response_type_name = _create_type_name(response_members, "srv");

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
  ConnextDynamicClientInfo * client_info = nullptr;
  // Begin initializing elements
  client = rmw_client_allocate();
  if (!client) {
    RMW_SET_ERROR_MSG("failed to allocate memory for client");
    goto fail;
  }

  request_type_code = create_type_code(request_type_name, request_members, participant_qos);
  if (!request_type_code) {
    // error string was set within the function
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
  buf = nullptr;  // Only free the casted pointer; don't need the buf pointer anymore.

  response_type_code = create_type_code(response_type_name, response_members, participant_qos);
  if (!request_type_code) {
    // error string was set within the function
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

  // create requester
  {
    if (!get_datareader_qos(participant, datareader_qos)) {
      // error string was set within the function
      goto fail;
    }
    if (!get_datawriter_qos(participant, datawriter_qos)) {
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
    typedef connext::Requester<DDS_DynamicData, DDS_DynamicData> Requester;
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
  client_info->request_type_support_ = request_type_support;
  client_info->response_type_support_ = response_type_support;
  client_info->response_type_code_ = response_type_code;
  client_info->request_type_code_ = request_type_code;
  client_info->request_members_ = request_members;
  client_info->response_members_ = response_members;

  client->implementation_identifier = rti_connext_dynamic_identifier;
  client->data = client_info;
  return client;
fail:
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
    if (client_info->request_type_support_) {
      RMW_TRY_DESTRUCTOR(
        client_info->request_type_support_->~DynamicDataTypeSupport(),
        DynamicDataTypeSupport, result = RMW_RET_ERROR)
      rmw_free(client_info->request_type_support_);
    }
    if (client_info->response_type_support_) {
      RMW_TRY_DESTRUCTOR(
        client_info->response_type_support_->~DynamicDataTypeSupport(),
        DynamicDataTypeSupport, result = RMW_RET_ERROR)
      rmw_free(client_info->response_type_support_);
    }
    if (client_info->requester_) {
      RMW_TRY_DESTRUCTOR(
        client_info->requester_->~Requester(),
        "connext::Requester<DDS_DynamicData, DDS_DynamicData>",
        result = RMW_RET_ERROR)
      rmw_free(client_info->requester_);
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

  bool published = _publish(sample, ros_request, client_info->request_members_);
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

rmw_ret_t
rmw_take_request(
  const rmw_service_t * service,
  void * ros_request_header,
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

  if (!ros_request_header) {
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

  rmw_request_id_t & req_id = *(static_cast<rmw_request_id_t *>(ros_request_header));

  connext::LoanedSamples<DDS::DynamicData> requests = replier->take_requests(1);
  if (requests.begin() != requests.end() && requests.begin()->info().valid_data) {
    bool success = _take(&requests.begin()->data(), ros_request, service_info->request_members_);
    if (!success) {
      // error string was set within the function
      return RMW_RET_ERROR;
    }

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
  void * ros_request_header,
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

  if (!ros_request_header) {
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
    bool success = _take(&replies.begin()->data(), ros_response, client_info->response_members_);
    if (!success) {
      // error string was set within the function
      return RMW_RET_ERROR;
    }

    rmw_request_id_t & req_id = *(static_cast<rmw_request_id_t *>(ros_request_header));
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
  void * ros_request_header,
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

  if (!ros_request_header) {
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

  bool published = _publish(sample, ros_response, service_info->response_members_);
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

  const rmw_request_id_t & req_id =
    *(static_cast<const rmw_request_id_t *>(ros_request_header));

  DDS_SampleIdentity_t request_identity;

  size_t SAMPLE_IDENTITY_SIZE = 16;
  memcpy(request_identity.writer_guid.value, &req_id.writer_guid[0], SAMPLE_IDENTITY_SIZE);

  request_identity.sequence_number.high = (int32_t)(
    (req_id.sequence_number & 0xFFFFFFFF00000000) >> 32);
  request_identity.sequence_number.low = (uint32_t)(req_id.sequence_number & 0xFFFFFFFF);

  replier->send_reply(response, request_identity);

  if (service_info->response_type_support_->delete_data(sample) != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete dynamic data object");
    return RMW_RET_ERROR;
  }

  return RMW_RET_OK;
}

rmw_service_t *
rmw_create_service(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name)
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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support handle,
    type_support->typesupport_identifier,
    rosidl_typesupport_introspection_cpp::typesupport_introspection_identifier,
    return NULL)

  auto node_info = static_cast<CustomNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDSDomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  auto service_members = static_cast<const rosidl_typesupport_introspection_cpp::ServiceMembers *>(
    type_support->data);
  if (!service_members) {
    RMW_SET_ERROR_MSG("service members handle is null");
    return NULL;
  }
  const rosidl_typesupport_introspection_cpp::MessageMembers * request_members =
    service_members->request_members_;
  if (!request_members) {
    RMW_SET_ERROR_MSG("request members handle is null");
    return NULL;
  }
  const rosidl_typesupport_introspection_cpp::MessageMembers * response_members =
    service_members->response_members_;
  if (!response_members) {
    RMW_SET_ERROR_MSG("response members handle is null");
    return NULL;
  }
  std::string request_type_name = _create_type_name(request_members, "srv");
  std::string response_type_name = _create_type_name(response_members, "srv");

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
  ConnextDynamicServiceInfo * server_info = nullptr;
  // Begin initializing elements
  service = rmw_service_allocate();
  if (!service) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }

  request_type_code = create_type_code(request_type_name, request_members, participant_qos);
  if (!request_type_code) {
    // error string was set within the function
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

  response_type_code = create_type_code(response_type_name, response_members, participant_qos);
  if (!response_type_code) {
    // error string was set within the function
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
    if (!get_datareader_qos(participant, datareader_qos)) {
      // error string was set within the function
      goto fail;
    }
    if (!get_datawriter_qos(participant, datawriter_qos)) {
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
    typedef connext::Replier<DDS_DynamicData, DDS_DynamicData> Replier;
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
  server_info->response_type_support_ = response_type_support;
  server_info->request_members_ = request_members;
  server_info->response_members_ = response_members;

  service->implementation_identifier = rti_connext_dynamic_identifier;
  service->data = server_info;
  return service;
fail:
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

void
destroy_topic_names_and_types(
  rmw_topic_names_and_types_t * topic_names_and_types)
{
  if (topic_names_and_types->topic_count) {
    for (size_t i = 0; i < topic_names_and_types->topic_count; ++i) {
      delete topic_names_and_types->topic_names[i];
      delete topic_names_and_types->type_names[i];
      topic_names_and_types->topic_names[i] = nullptr;
      topic_names_and_types->type_names[i] = nullptr;
    }
    if (topic_names_and_types->topic_names) {
      rmw_free(topic_names_and_types->topic_names);
      topic_names_and_types->topic_names = nullptr;
    }
    if (topic_names_and_types->type_names) {
      rmw_free(topic_names_and_types->type_names);
      topic_names_and_types->type_names = nullptr;
    }
    topic_names_and_types->topic_count = 0;
  }
}

rmw_ret_t
rmw_get_topic_names_and_types(
  const rmw_node_t * node,
  rmw_topic_names_and_types_t * topic_names_and_types)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != rti_connext_dynamic_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!topic_names_and_types) {
    RMW_SET_ERROR_MSG("topics handle is null");
    return RMW_RET_ERROR;
  }
  if (topic_names_and_types->topic_count) {
    RMW_SET_ERROR_MSG("topic count is not zero");
    return RMW_RET_ERROR;
  }
  if (topic_names_and_types->topic_names) {
    RMW_SET_ERROR_MSG("topic names is not null");
    return RMW_RET_ERROR;
  }
  if (topic_names_and_types->type_names) {
    RMW_SET_ERROR_MSG("type names is not null");
    return RMW_RET_ERROR;
  }

  auto node_info = static_cast<CustomNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->publisher_listener) {
    RMW_SET_ERROR_MSG("publisher listener handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->subscriber_listener) {
    RMW_SET_ERROR_MSG("subscriber listener handle is null");
    return RMW_RET_ERROR;
  }

  // combine publisher and subscriber information
  std::map<std::string, std::set<std::string>> topics_with_multiple_types;
  for (auto it : node_info->publisher_listener->topic_names_and_types) {
    for (auto & jt : it.second) {
      topics_with_multiple_types[it.first].insert(jt);
    }
  }
  for (auto it : node_info->subscriber_listener->topic_names_and_types) {
    for (auto & jt : it.second) {
      topics_with_multiple_types[it.first].insert(jt);
    }
  }

  // ignore inconsistent types
  std::map<std::string, std::string> topics;
  for (auto & it : topics_with_multiple_types) {
    if (it.second.size() != 1) {
      fprintf(stderr, "topic type mismatch - ignoring topic '%s'\n", it.first.c_str());
      continue;
    }
    topics[it.first] = *it.second.begin();
  }

  // reformat type name
  std::string substr = "::msg::dds_::";
  for (auto & it : topics) {
    size_t substr_pos = it.second.find(substr);
    if (it.second[it.second.size() - 1] == '_' && substr_pos != std::string::npos) {
      it.second = it.second.substr(0, substr_pos) + "/" + it.second.substr(
        substr_pos + substr.size(), it.second.size() - substr_pos - substr.size() - 1);
    }
  }

  // copy data into result handle
  if (topics.size() > 0) {
    topic_names_and_types->topic_names = static_cast<char **>(
      rmw_allocate(sizeof(char *) * topics.size()));
    if (!topic_names_and_types->topic_names) {
      RMW_SET_ERROR_MSG("failed to allocate memory for topic names")
      return RMW_RET_ERROR;
    }
    topic_names_and_types->type_names = static_cast<char **>(
      rmw_allocate(sizeof(char *) * topics.size()));
    if (!topic_names_and_types->type_names) {
      rmw_free(topic_names_and_types->topic_names);
      RMW_SET_ERROR_MSG("failed to allocate memory for type names")
      return RMW_RET_ERROR;
    }
    for (auto it : topics) {
      char * topic_name = strdup(it.first.c_str());
      if (!topic_name) {
        RMW_SET_ERROR_MSG("failed to allocate memory for topic name")
        goto fail;
      }
      char * type_name = strdup(it.second.c_str());
      if (!type_name) {
        rmw_free(topic_name);
        RMW_SET_ERROR_MSG("failed to allocate memory for type name")
        goto fail;
      }
      size_t i = topic_names_and_types->topic_count;
      topic_names_and_types->topic_names[i] = topic_name;
      topic_names_and_types->type_names[i] = type_name;
      ++topic_names_and_types->topic_count;
    }
  }

  return RMW_RET_OK;
fail:
  destroy_topic_names_and_types(topic_names_and_types);
  return RMW_RET_ERROR;
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
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != rti_connext_dynamic_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!topic_name) {
    RMW_SET_ERROR_MSG("topic name is null");
    return RMW_RET_ERROR;
  }
  if (!count) {
    RMW_SET_ERROR_MSG("count handle is null");
    return RMW_RET_ERROR;
  }

  auto node_info = static_cast<CustomNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->publisher_listener) {
    RMW_SET_ERROR_MSG("publisher listener handle is null");
    return RMW_RET_ERROR;
  }

  const auto & topic_names_and_types = node_info->publisher_listener->topic_names_and_types;
  auto it = topic_names_and_types.find(topic_name);
  if (it == topic_names_and_types.end()) {
    *count = 0;
  } else {
    *count = it->second.size();
  }
  return RMW_RET_OK;
}

rmw_ret_t
rmw_count_subscribers(
  const rmw_node_t * node,
  const char * topic_name,
  size_t * count)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != rti_connext_dynamic_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!topic_name) {
    RMW_SET_ERROR_MSG("topic name is null");
    return RMW_RET_ERROR;
  }
  if (!count) {
    RMW_SET_ERROR_MSG("count handle is null");
    return RMW_RET_ERROR;
  }

  auto node_info = static_cast<CustomNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->subscriber_listener) {
    RMW_SET_ERROR_MSG("subscriber listener handle is null");
    return RMW_RET_ERROR;
  }

  const auto & topic_names_and_types = node_info->subscriber_listener->topic_names_and_types;
  auto it = topic_names_and_types.find(topic_name);
  if (it == topic_names_and_types.end()) {
    *count = 0;
  } else {
    *count = it->second.size();
  }
  return RMW_RET_OK;
}

}  // extern "C"
