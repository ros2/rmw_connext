// Copyright 2015-2017 Open Source Robotics Foundation, Inc.
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

#include "rmw_connext_shared_cpp/shared_functions.hpp"

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "rcutils/filesystem.h"
#include "rcutils/logging_macros.h"
#include "rcutils/strdup.h"
#include "rcutils/types/string_array.h"
#include "rmw/allocators.h"
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/sanity_checks.h"

// Uncomment this to get extra console output about discovery.
// #define DISCOVERY_DEBUG_LOGGING 1

void CustomDataReaderListener::add_information(
  const DDS_InstanceHandle_t & instance_handle,
  const std::string & topic_name,
  const std::string & type_name,
  EntityType entity_type)
{
  (void)entity_type;
  std::lock_guard<std::mutex> topic_descriptor_lock(topic_descriptor_mutex_);
  // store topic name and type name
  auto & topic_types = topic_names_and_types[topic_name];
  topic_types.insert(type_name);
  // store mapping to instance handle
  TopicDescriptor topic_descriptor;
  topic_descriptor.instance_handle = instance_handle;
  topic_descriptor.name = topic_name;
  topic_descriptor.type = type_name;
  topic_descriptors.push_back(topic_descriptor);
#ifdef DISCOVERY_DEBUG_LOGGING
  printf("+%s %s <%s>\n",
    entity_type == EntityType::Publisher ? "P" : "S",
    topic_name.c_str(),
    type_name.c_str());
#endif
}

void CustomDataReaderListener::remove_information(
  const DDS_InstanceHandle_t & instance_handle,
  EntityType entity_type)
{
  (void)entity_type;
  std::lock_guard<std::mutex> topic_descriptor_lock(topic_descriptor_mutex_);
  // find entry by instance handle
  for (auto it = topic_descriptors.begin(); it != topic_descriptors.end(); ++it) {
    if (DDS_InstanceHandle_equals(&it->instance_handle, &instance_handle)) {
      // remove entries
#ifdef DISCOVERY_DEBUG_LOGGING
      printf("-%s %s <%s>\n",
        entity_type == EntityType::Publisher ? "P" : "S",
        it->name.c_str(),
        it->type.c_str());
#endif
      auto & topic_types = topic_names_and_types[it->name];
      topic_types.erase(topic_types.find(it->type));
      if (topic_types.empty()) {
        topic_names_and_types.erase(it->name);
      }
      topic_descriptors.erase(it);
      break;
    }
  }
}

void CustomDataReaderListener::trigger_graph_guard_condition()
{
#ifdef DISCOVERY_DEBUG_LOGGING
  printf("graph guard condition triggered...\n");
#endif
  rmw_ret_t ret = trigger_guard_condition(implementation_identifier_, graph_guard_condition_);
  if (ret != RMW_RET_OK) {
    fprintf(stderr, "failed to trigger graph guard condition: %s\n", rmw_get_error_string_safe());
  }
}

void CustomPublisherListener::on_data_available(DDSDataReader * reader)
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
      auto pub_fqdn = std::string("");
      for (int j = 0; j < data_seq[i].partition.name.length(); ++j) {
        pub_fqdn += data_seq[i].partition.name[j];
        pub_fqdn += "/";
      }
      pub_fqdn += data_seq[i].topic_name;

      add_information(
        info_seq[i].instance_handle,
        pub_fqdn,
        data_seq[i].type_name,
        EntityType::Publisher);
    } else {
      remove_information(info_seq[i].instance_handle, EntityType::Publisher);
    }
  }

  if (data_seq.length() > 0) {
    this->trigger_graph_guard_condition();
  }

  builtin_reader->return_loan(data_seq, info_seq);
}

void CustomSubscriberListener::on_data_available(DDSDataReader * reader)
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
      auto sub_fqdn = std::string("");
      for (int j = 0; j < data_seq[i].partition.name.length(); ++j) {
        sub_fqdn += data_seq[i].partition.name[j];
        sub_fqdn += "/";
      }
      sub_fqdn += data_seq[i].topic_name;
      add_information(
        info_seq[i].instance_handle,
        sub_fqdn,
        data_seq[i].type_name,
        EntityType::Subscriber);
    } else {
      remove_information(info_seq[i].instance_handle, EntityType::Subscriber);
    }
  }

  if (data_seq.length() > 0) {
    this->trigger_graph_guard_condition();
  }

  builtin_reader->return_loan(data_seq, info_seq);
}


rmw_ret_t
init()
{
  DDSDomainParticipantFactory * dpf_ = DDSDomainParticipantFactory::get_instance();
  if (!dpf_) {
    RMW_SET_ERROR_MSG("failed to get participant factory");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_ret_t
check_attach_condition_error(DDS::ReturnCode_t retcode)
{
  if (retcode == DDS_RETCODE_OK) {
    return RMW_RET_OK;
  }
  if (retcode == DDS_RETCODE_OUT_OF_RESOURCES) {
    RMW_SET_ERROR_MSG("failed to attach condition to waitset: out of resources");
  } else if (retcode == DDS_RETCODE_BAD_PARAMETER) {
    RMW_SET_ERROR_MSG("failed to attach condition to waitset: condition pointer was invalid");
  } else {
    RMW_SET_ERROR_MSG("failed to attach condition to waitset");
  }
  return RMW_RET_ERROR;
}

bool
get_datareader_qos(
  DDSDomainParticipant * participant,
  const rmw_qos_profile_t & qos_profile,
  DDS_DataReaderQos & datareader_qos)
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

  if (!set_entity_qos_from_profile(qos_profile, datareader_qos)) {
    return false;
  }

  return true;
}

bool
get_datawriter_qos(
  DDSDomainParticipant * participant,
  const rmw_qos_profile_t & qos_profile,
  DDS_DataWriterQos & datawriter_qos)
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

  if (!set_entity_qos_from_profile(qos_profile, datawriter_qos)) {
    return false;
  }

  // TODO(wjwwood): conditionally use the async publish mode using a heuristic:
  //  https://github.com/ros2/rmw_connext/issues/190
  datawriter_qos.publish_mode.kind = DDS::ASYNCHRONOUS_PUBLISH_MODE_QOS;

  return true;
}

rmw_node_t *
create_node(
  const char * implementation_identifier,
  const char * name,
  const char * namespace_,
  size_t domain_id,
  const rmw_node_security_options_t * security_options)
{
  if (!security_options) {
    RMW_SET_ERROR_MSG("security_options is null");
    return nullptr;
  }
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
  // This String_dup is not matched with a String_free because DDS appears to
  // free this automatically.
  participant_qos.participant_name.name = DDS::String_dup(name);
  // forces local traffic to be sent over loopback,
  // even if a more efficient transport (such as shared memory) is installed
  // (in which case traffic will be sent over both transports)
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

  if (security_options->security_root_path) {
    // enable some security stuff
    status = DDSPropertyQosPolicyHelper::add_property(
      participant_qos.property,
      "com.rti.serv.load_plugin",
      "com.rti.serv.secure",
      DDS_BOOLEAN_FALSE);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to add security property");
      return NULL;
    }
    status = DDSPropertyQosPolicyHelper::add_property(
      participant_qos.property,
      "com.rti.serv.secure.library",
      "nddssecurity",
      DDS_BOOLEAN_FALSE);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to add security property");
      return NULL;
    }
    status = DDSPropertyQosPolicyHelper::add_property(
      participant_qos.property,
      "com.rti.serv.secure.create_function",
      "RTI_Security_PluginSuite_create",
      DDS_BOOLEAN_FALSE);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to add security property");
      return NULL;
    }

    const char * srp = security_options->security_root_path;  // save some typing
    std::string ca_cert_fn = rcutils_join_path(srp, "ca.cert.pem");
    std::string cert_fn = rcutils_join_path(srp, "cert.pem");
    std::string key_fn = rcutils_join_path(srp, "key.pem");
    std::string gov_fn = rcutils_join_path(srp, "governance.p7s");
    std::string perm_fn = rcutils_join_path(srp, "permissions.p7s");

    // now try to pass these filenames to the Authentication plugin
    status = DDSPropertyQosPolicyHelper::add_property(
      participant_qos.property,
      "com.rti.serv.secure.authentication.ca_file",
      ca_cert_fn.c_str(),
      DDS_BOOLEAN_FALSE);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to add security property");
      return NULL;
    }
    status = DDSPropertyQosPolicyHelper::add_property(
      participant_qos.property,
      "com.rti.serv.secure.authentication.certificate_file",
      cert_fn.c_str(),
      DDS_BOOLEAN_FALSE);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to add security property");
      return NULL;
    }
    status = DDSPropertyQosPolicyHelper::add_property(
      participant_qos.property,
      "com.rti.serv.secure.authentication.private_key_file",
      key_fn.c_str(),
      DDS_BOOLEAN_FALSE);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to add security property");
      return NULL;
    }

    // pass filenames to the Access Control plugin
    status = DDSPropertyQosPolicyHelper::add_property(
      participant_qos.property,
      "com.rti.serv.secure.access_control.permissions_authority_file",
      ca_cert_fn.c_str(),
      DDS_BOOLEAN_FALSE);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to add security property");
      return NULL;
    }

    status = DDSPropertyQosPolicyHelper::add_property(
      participant_qos.property,
      "com.rti.serv.secure.access_control.governance_file",
      gov_fn.c_str(),
      DDS_BOOLEAN_FALSE);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to add security property");
      return NULL;
    }

    status = DDSPropertyQosPolicyHelper::add_property(
      participant_qos.property,
      "com.rti.serv.secure.access_control.permissions_file",
      perm_fn.c_str(),
      DDS_BOOLEAN_FALSE);
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to add security property");
      return NULL;
    }
  }
  DDS_DomainId_t domain = static_cast<DDS_DomainId_t>(domain_id);

  DDSDomainParticipant * participant = dpf_->create_participant(
    domain, participant_qos, NULL,
    DDS_STATUS_MASK_NONE);
  if (!participant) {
    RMW_SET_ERROR_MSG("failed to create participant");
    return NULL;
  }

  rmw_node_t * node_handle = nullptr;
  ConnextNodeInfo * node_info = nullptr;
  rmw_guard_condition_t * graph_guard_condition = nullptr;
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

  graph_guard_condition = create_guard_condition(implementation_identifier);
  if (!graph_guard_condition) {
    RMW_SET_ERROR_MSG("failed to create graph guard condition");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CustomPublisherListener));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(
    publisher_listener, buf, goto fail, CustomPublisherListener,
    implementation_identifier, graph_guard_condition)
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
  RMW_TRY_PLACEMENT_NEW(
    subscriber_listener, buf, goto fail, CustomSubscriberListener,
    implementation_identifier, graph_guard_condition)
  buf = nullptr;
  builtin_subscription_datareader->set_listener(subscriber_listener, DDS_DATA_AVAILABLE_STATUS);

  node_handle = rmw_node_allocate();
  if (!node_handle) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node handle");
    goto fail;
  }
  node_handle->implementation_identifier = implementation_identifier;
  node_handle->data = participant;

  node_handle->name =
    reinterpret_cast<const char *>(rmw_allocate(sizeof(char) * strlen(name) + 1));
  if (!node_handle->name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  memcpy(const_cast<char *>(node_handle->name), name, strlen(name) + 1);

  node_handle->namespace_ =
    reinterpret_cast<const char *>(rmw_allocate(sizeof(char) * strlen(namespace_) + 1));
  if (!node_handle->namespace_) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node namespace");
    goto fail;
  }
  memcpy(const_cast<char *>(node_handle->namespace_), namespace_, strlen(namespace_) + 1);

  buf = rmw_allocate(sizeof(ConnextNodeInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(node_info, buf, goto fail, ConnextNodeInfo, )
  buf = nullptr;
  node_info->participant = participant;
  node_info->publisher_listener = publisher_listener;
  node_info->subscriber_listener = subscriber_listener;
  node_info->graph_guard_condition = graph_guard_condition;

  node_handle->implementation_identifier = implementation_identifier;
  node_handle->data = node_info;
  return node_handle;
fail:
  status = dpf_->delete_participant(participant);
  if (status != DDS_RETCODE_OK) {
    std::stringstream ss;
    ss << "leaking participant while handling failure at " <<
      __FILE__ << ":" << __LINE__;
    (std::cerr << ss.str()).flush();
  }
  if (graph_guard_condition) {
    rmw_ret_t ret = destroy_guard_condition(implementation_identifier, graph_guard_condition);
    if (ret != RMW_RET_OK) {
      std::stringstream ss;
      ss << "failed to destroy guard condition while handling failure at " <<
        __FILE__ << ":" << __LINE__;
      (std::cerr << ss.str()).flush();
    }
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
  if (node_handle) {
    if (node_handle->name) {
      rmw_free(const_cast<char *>(node_handle->name));
    }
    if (node_handle->namespace_) {
      rmw_free(const_cast<char *>(node_handle->namespace_));
    }
    rmw_free(node_handle);
  }
  if (node_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->~ConnextNodeInfo(), ConnextNodeInfo)
    rmw_free(node_info);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

rmw_ret_t
destroy_node(const char * implementation_identifier, rmw_node_t * node)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, implementation_identifier,
    return RMW_RET_ERROR)

  DDSDomainParticipantFactory * dpf_ = DDSDomainParticipantFactory::get_instance();
  if (!dpf_) {
    RMW_SET_ERROR_MSG("failed to get participant factory");
    return RMW_RET_ERROR;
  }

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
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
  if (participant->delete_contained_entities() != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete contained entities of participant");
    return RMW_RET_ERROR;
  }

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
  if (node_info->graph_guard_condition) {
    rmw_ret_t rmw_ret =
      destroy_guard_condition(implementation_identifier, node_info->graph_guard_condition);
    if (rmw_ret != RMW_RET_OK) {
      RMW_SET_ERROR_MSG("failed to delete graph guard condition");
      return RMW_RET_ERROR;
    }
    node_info->graph_guard_condition = nullptr;
  }

  rmw_free(node_info);
  node->data = nullptr;
  rmw_free(const_cast<char *>(node->name));
  node->name = nullptr;
  rmw_free(const_cast<char *>(node->namespace_));
  node->namespace_ = nullptr;
  rmw_node_free(node);

  return RMW_RET_OK;
}

rmw_guard_condition_t *
create_guard_condition(const char * implementation_identifier)
{
  rmw_guard_condition_t * guard_condition = rmw_guard_condition_allocate();
  if (!guard_condition) {
    RMW_SET_ERROR_MSG("failed to allocate guard condition");
    return NULL;
  }
  // Allocate memory for the DDSGuardCondition object.
  DDSGuardCondition * dds_guard_condition = nullptr;
  void * buf = rmw_allocate(sizeof(DDSGuardCondition));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the DDSGuardCondition in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(dds_guard_condition, buf, goto fail, DDSGuardCondition, )
  buf = nullptr;  // Only free the dds_guard_condition pointer; don't need the buf pointer anymore.
  guard_condition->implementation_identifier = implementation_identifier;
  guard_condition->data = dds_guard_condition;
  return guard_condition;
fail:
  if (guard_condition) {
    rmw_guard_condition_free(guard_condition);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

rmw_ret_t
destroy_guard_condition(const char * implementation_identifier,
  rmw_guard_condition_t * guard_condition)
{
  if (!guard_condition) {
    RMW_SET_ERROR_MSG("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    guard condition handle,
    guard_condition->implementation_identifier, implementation_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  RMW_TRY_DESTRUCTOR(
    static_cast<DDSGuardCondition *>(guard_condition->data)->~DDSGuardCondition(),
    DDSGuardCondition, result = RMW_RET_ERROR)
  rmw_free(guard_condition->data);
  rmw_guard_condition_free(guard_condition);
  return result;
}

rmw_waitset_t *
create_waitset(const char * implementation_identifier, size_t max_conditions)
{
  rmw_waitset_t * waitset = rmw_waitset_allocate();

  ConnextWaitSetInfo * waitset_info = nullptr;

  // From here onward, error results in unrolling in the goto fail block.
  if (!waitset) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }
  waitset->implementation_identifier = implementation_identifier;
  waitset->data = rmw_allocate(sizeof(ConnextWaitSetInfo));
  waitset_info = static_cast<ConnextWaitSetInfo *>(waitset->data);

  if (!waitset_info) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }

  waitset_info->waitset = static_cast<DDSWaitSet *>(rmw_allocate(sizeof(DDSWaitSet)));
  if (!waitset_info->waitset) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }

  RMW_TRY_PLACEMENT_NEW(
    waitset_info->waitset, waitset_info->waitset, goto fail, DDSWaitSet, )

  // Now allocate storage for the ConditionSeq objects
  waitset_info->active_conditions =
    static_cast<DDSConditionSeq *>(rmw_allocate(sizeof(DDSConditionSeq)));
  if (!waitset_info->active_conditions) {
    RMW_SET_ERROR_MSG("failed to allocate active_conditions sequence");
    goto fail;
  }

  waitset_info->attached_conditions =
    static_cast<DDSConditionSeq *>(rmw_allocate(sizeof(DDSConditionSeq)));
  if (!waitset_info->attached_conditions) {
    RMW_SET_ERROR_MSG("failed to allocate attached_conditions sequence");
    goto fail;
  }

  // If max_conditions is greater than zero, re-allocate both ConditionSeqs to max_conditions
  if (max_conditions > 0) {
    RMW_TRY_PLACEMENT_NEW(
      waitset_info->active_conditions, waitset_info->active_conditions, goto fail,
      DDSConditionSeq, static_cast<DDS_Long>(max_conditions))

    RMW_TRY_PLACEMENT_NEW(
      waitset_info->attached_conditions, waitset_info->attached_conditions, goto fail,
      DDSConditionSeq,
      static_cast<DDS_Long>(max_conditions))
  } else {
    // Else, don't preallocate: the vectors will size dynamically when rmw_wait is called.
    // Default-construct the ConditionSeqs.
    RMW_TRY_PLACEMENT_NEW(
      waitset_info->active_conditions, waitset_info->active_conditions,
      goto fail, DDSConditionSeq, )

    RMW_TRY_PLACEMENT_NEW(
      waitset_info->attached_conditions, waitset_info->attached_conditions, goto fail,
      DDSConditionSeq, )
  }

  return waitset;

fail:
  if (waitset_info) {
    if (waitset_info->active_conditions) {
      // How to know which constructor threw?
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
        waitset_info->active_conditions->~DDSConditionSeq(), DDSConditionSeq)
      rmw_free(waitset_info->active_conditions);
    }
    if (waitset_info->attached_conditions) {
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
        waitset_info->attached_conditions->~DDSConditionSeq(), DDSConditionSeq)
      rmw_free(waitset_info->attached_conditions);
    }
    if (waitset_info->waitset) {
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(waitset_info->waitset->~DDSWaitSet(), DDSWaitSet)
      rmw_free(waitset_info->waitset);
    }
    waitset_info = nullptr;
  }
  if (waitset) {
    if (waitset->data) {
      rmw_free(waitset->data);
    }
    rmw_waitset_free(waitset);
  }
  return nullptr;
}

rmw_ret_t
destroy_waitset(const char * implementation_identifier,
  rmw_waitset_t * waitset)
{
  if (!waitset) {
    RMW_SET_ERROR_MSG("waitset handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    waitset handle,
    waitset->implementation_identifier, implementation_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  ConnextWaitSetInfo * waitset_info = static_cast<ConnextWaitSetInfo *>(waitset->data);

  // Explicitly call destructor since the "placement new" was used
  if (waitset_info->active_conditions) {
    RMW_TRY_DESTRUCTOR(
      waitset_info->active_conditions->~ConditionSeq(), ConditionSeq, result = RMW_RET_ERROR)
    rmw_free(waitset_info->active_conditions);
  }
  if (waitset_info->attached_conditions) {
    RMW_TRY_DESTRUCTOR(
      waitset_info->attached_conditions->~ConditionSeq(), ConditionSeq, result = RMW_RET_ERROR)
    rmw_free(waitset_info->attached_conditions);
  }
  if (waitset_info->waitset) {
    RMW_TRY_DESTRUCTOR(waitset_info->waitset->~WaitSet(), WaitSet, result = RMW_RET_ERROR)
    rmw_free(waitset_info->waitset);
  }
  waitset_info = nullptr;
  if (waitset->data) {
    rmw_free(waitset->data);
  }
  if (waitset) {
    rmw_waitset_free(waitset);
  }
  return result;
}

rmw_ret_t
trigger_guard_condition(const char * implementation_identifier,
  const rmw_guard_condition_t * guard_condition_handle)
{
  if (!guard_condition_handle) {
    RMW_SET_ERROR_MSG("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    guard condition handle,
    guard_condition_handle->implementation_identifier, implementation_identifier,
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

static std::vector<std::string> _ros_prefixes =
  {ros_topic_prefix, ros_service_requester_prefix, ros_service_response_prefix};

/// Return the ROS specific prefix if it exists, otherwise "".
static inline
std::string
_get_ros_prefix_if_exists(const std::string & topic_name)
{
  for (auto prefix : _ros_prefixes) {
    if (topic_name.rfind(std::string(prefix) + "/", 0) == 0) {
      return prefix;
    }
  }
  return "";
}

/// Return the demangle ROS topic or the original if not a ROS topic.
static inline
std::string
_demangle_if_ros_topic(const std::string & topic_name)
{
  std::string prefix = _get_ros_prefix_if_exists(topic_name);
  if (prefix.length()) {
    return topic_name.substr(strlen(ros_topic_prefix));
  }
  return topic_name;
}

/// Return the demangled ROS type or the original if not a ROS type.
static inline
std::string
_demangle_if_ros_type(const std::string & dds_type_string)
{
  std::string substring = "::msg::dds_::";
  size_t substring_position = dds_type_string.find(substring);
  if (
    dds_type_string[dds_type_string.size() - 1] == '_' &&
    substring_position != std::string::npos)
  {
    std::string pkg = dds_type_string.substr(0, substring_position);
    size_t start = substring_position + substring.size();
    std::string type_name = dds_type_string.substr(start, dds_type_string.length() - 1 - start);
    return pkg + "/" + type_name;
  }
  // not a ROS type
  return dds_type_string;
}

rmw_ret_t
get_topic_names_and_types(
  const char * implementation_identifier,
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  bool no_demangle,
  rmw_names_and_types_t * topic_names_and_types)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != implementation_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  rmw_ret_t rmw_ret;
  rmw_ret = rmw_names_and_types_check_zero(topic_names_and_types);
  if (rmw_ret != RMW_RET_OK) {
    return rmw_ret;
  }

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
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
  std::map<std::string, std::set<std::string>> topics;
  for (auto it : node_info->publisher_listener->topic_names_and_types) {
    if (!no_demangle && _get_ros_prefix_if_exists(it.first) != ros_topic_prefix) {
      // if we are demangling and this is not prefixed with rt/, skip it
      continue;
    }
    for (auto & jt : it.second) {
      topics[it.first].insert(jt);
    }
  }
  for (auto it : node_info->subscriber_listener->topic_names_and_types) {
    if (!no_demangle && _get_ros_prefix_if_exists(it.first) != ros_topic_prefix) {
      // if we are demangling and this is not prefixed with rt/, skip it
      continue;
    }
    for (auto & jt : it.second) {
      topics[it.first].insert(jt);
    }
  }

  // setup demangle functions based on no_demangle
  auto demangle_topic = _demangle_if_ros_topic;
  auto demangle_type = _demangle_if_ros_type;
  if (no_demangle) {
    auto noop = [](const std::string & in) -> std::string {return in;};
    demangle_topic = noop;
    demangle_type = noop;
  }

  // Copy data to results handle
  if (topics.size() > 0) {
    // Setup string array to store names
    rmw_ret_t rmw_ret = rmw_names_and_types_init(topic_names_and_types, topics.size(), allocator);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }
    // Setup cleanup function, in case of failure below
    auto fail_cleanup = [&topic_names_and_types]() {
        rmw_ret_t rmw_ret = rmw_names_and_types_fini(topic_names_and_types);
        if (rmw_ret != RMW_RET_OK) {
          RCUTILS_LOG_ERROR("error during report of error: %s", rmw_get_error_string_safe())
        }
      };
    // Setup demangling functions based on no_demangle option
    auto demangle_topic = _demangle_if_ros_topic;
    auto demangle_type = _demangle_if_ros_type;
    if (no_demangle) {
      auto noop = [](const std::string & in) {
          return in;
        };
      demangle_topic = noop;
      demangle_type = noop;
    }
    // For each topic, store the name, initialize the string array for types, and store all types
    size_t index = 0;
    for (const auto & topic_n_types : topics) {
      // Duplicate and store the topic_name
      char * topic_name = rcutils_strdup(demangle_topic(topic_n_types.first).c_str(), *allocator);
      if (!topic_name) {
        RMW_SET_ERROR_MSG_ALLOC("failed to allocate memory for topic name", *allocator);
        fail_cleanup();
        return RMW_RET_BAD_ALLOC;
      }
      topic_names_and_types->names.data[index] = topic_name;
      // Setup storage for types
      {
        rcutils_ret_t rcutils_ret = rcutils_string_array_init(
          &topic_names_and_types->types[index],
          topic_n_types.second.size(),
          allocator);
        if (rcutils_ret != RCUTILS_RET_OK) {
          RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
          fail_cleanup();
          return rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
        }
      }
      // Duplicate and store each type for the topic
      size_t type_index = 0;
      for (const auto & type : topic_n_types.second) {
        char * type_name = rcutils_strdup(demangle_type(type).c_str(), *allocator);
        if (!type_name) {
          RMW_SET_ERROR_MSG_ALLOC("failed to allocate memory for type name", *allocator)
          fail_cleanup();
          return RMW_RET_BAD_ALLOC;
        }
        topic_names_and_types->types[index].data[type_index] = type_name;
        ++type_index;
      }  // for each type
      ++index;
    }  // for each topic
  }
  return RMW_RET_OK;
}

/// Return the service name for a given topic if it is part of one, else "".
static inline
std::string
_demangle_service_from_topic(const std::string & topic_name)
{
  std::string prefix = _get_ros_prefix_if_exists(topic_name);
  if (!prefix.length()) {
    // not a ROS topic or service
    return "";
  }
  std::vector<std::string> prefixes = {
    ros_service_response_prefix,
    ros_service_requester_prefix,
  };
  if (std::none_of(prefixes.cbegin(), prefixes.cend(), [&prefix](auto x) {
    return prefix == x;
  }))
  {
    // not a ROS service topic
    return "";
  }
  std::vector<std::string> suffixes = {
    "Reply",
    "Request",
  };
  std::string found_suffix;
  size_t suffix_position;
  for (auto suffix : suffixes) {
    suffix_position = topic_name.rfind(suffix);
    if (suffix_position != std::string::npos) {
      if (topic_name.length() - suffix_position - suffix.length() != 0) {
        RCUTILS_LOG_WARN_NAMED("rmw_connext_shared_cpp",
          "service topic has service prefix and a suffix, but not at the end"
          ", report this: '%s'", topic_name.c_str())
        continue;
      }
      found_suffix = suffix;
      break;
    }
  }
  if (suffix_position == std::string::npos) {
    RCUTILS_LOG_WARN_NAMED("rmw_connext_shared_cpp",
      "service topic has prefix but no suffix"
      ", report this: '%s'", topic_name.c_str())
    return "";
  }
  // strip off the suffix first
  std::string service_name = topic_name.substr(0, suffix_position + 1);
  // then the prefix
  size_t start = prefix.length();  // explicitly leave / after prefix
  return service_name.substr(start, service_name.length() - 1 - start);
}

/// Return the demangled service type if it is a ROS srv type, else "".
static inline
std::string
_demangle_service_type_only(const std::string & dds_type_name)
{
  std::string ns_substring = "::srv::dds_::";
  size_t ns_substring_position = dds_type_name.find(ns_substring);
  if (ns_substring_position == std::string::npos) {
    // not a ROS service type
    return "";
  }
  auto suffixes = {
    std::string("_Response_"),
    std::string("_Request_"),
  };
  std::string found_suffix = "";
  size_t suffix_position = 0;
  for (auto suffix : suffixes) {
    suffix_position = dds_type_name.rfind(suffix);
    if (suffix_position != std::string::npos) {
      if (dds_type_name.length() - suffix_position - suffix.length() != 0) {
        RCUTILS_LOG_WARN_NAMED("rmw_connext_shared_cpp",
          "service type contains '::srv::dds_::' and a suffix, but not at the end"
          ", report this: '%s'", dds_type_name.c_str())
        continue;
      }
      found_suffix = suffix;
      break;
    }
  }
  if (suffix_position == std::string::npos) {
    RCUTILS_LOG_WARN_NAMED("rmw_connext_shared_cpp",
      "service type contains '::srv::dds_::' but does not have a suffix"
      ", report this: '%s'", dds_type_name.c_str())
    return "";
  }
  // everything checks out, reformat it from '<pkg>::srv::dds_::<type><suffix>' to '<pkg>/<type>'
  std::string pkg = dds_type_name.substr(0, ns_substring_position);
  size_t start = ns_substring_position + ns_substring.length();
  std::string type_name = dds_type_name.substr(start, suffix_position - start);
  return pkg + "/" + type_name;
}

rmw_ret_t
get_service_names_and_types(
  const char * implementation_identifier,
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  rmw_names_and_types_t * service_names_and_types)
{
  if (!allocator) {
    RMW_SET_ERROR_MSG("allocator is null")
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (!node) {
    RMW_SET_ERROR_MSG_ALLOC("null node handle", *allocator)
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (node->implementation_identifier != implementation_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  rmw_ret_t ret = rmw_names_and_types_check_zero(service_names_and_types);
  if (ret != RMW_RET_OK) {
    return ret;
  }

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
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
  std::map<std::string, std::set<std::string>> services;
  {
    for (auto it : node_info->publisher_listener->topic_names_and_types) {
      std::string service_name = _demangle_service_from_topic(it.first);
      if (!service_name.length()) {
        // not a service
        continue;
      }
      for (auto & itt : it.second) {
        std::string service_type = _demangle_service_type_only(itt);
        if (service_type.length()) {
          services[service_name].insert(service_type);
        }
      }
    }
  }
  {
    for (auto it : node_info->subscriber_listener->topic_names_and_types) {
      std::string service_name = _demangle_service_from_topic(it.first);
      if (!service_name.length()) {
        // not a service
        continue;
      }
      for (auto & itt : it.second) {
        std::string service_type = _demangle_service_type_only(itt);
        if (service_type.length()) {
          services[service_name].insert(service_type);
        }
      }
    }
  }

  // Fill out service_names_and_types
  if (services.size()) {
    // Setup string array to store names
    rmw_ret_t rmw_ret =
      rmw_names_and_types_init(service_names_and_types, services.size(), allocator);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }
    // Setup cleanup function, in case of failure below
    auto fail_cleanup = [&service_names_and_types]() {
        rmw_ret_t rmw_ret = rmw_names_and_types_fini(service_names_and_types);
        if (rmw_ret != RMW_RET_OK) {
          RCUTILS_LOG_ERROR("error during report of error: %s", rmw_get_error_string_safe())
        }
      };
    // For each service, store the name, initialize the string array for types, and store all types
    size_t index = 0;
    for (const auto & service_n_types : services) {
      // Duplicate and store the service_name
      char * service_name = rcutils_strdup(service_n_types.first.c_str(), *allocator);
      if (!service_name) {
        RMW_SET_ERROR_MSG_ALLOC("failed to allocate memory for service name", *allocator);
        fail_cleanup();
        return RMW_RET_BAD_ALLOC;
      }
      service_names_and_types->names.data[index] = service_name;
      // Setup storage for types
      {
        rcutils_ret_t rcutils_ret = rcutils_string_array_init(
          &service_names_and_types->types[index],
          service_n_types.second.size(),
          allocator);
        if (rcutils_ret != RCUTILS_RET_OK) {
          RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
          fail_cleanup();
          return rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
        }
      }
      // Duplicate and store each type for the service
      size_t type_index = 0;
      for (const auto & type : service_n_types.second) {
        char * type_name = rcutils_strdup(type.c_str(), *allocator);
        if (!type_name) {
          RMW_SET_ERROR_MSG_ALLOC("failed to allocate memory for type name", *allocator)
          fail_cleanup();
          return RMW_RET_BAD_ALLOC;
        }
        service_names_and_types->types[index].data[type_index] = type_name;
        ++type_index;
      }  // for each type
      ++index;
    }  // for each service
  }
  return RMW_RET_OK;
}

rmw_ret_t
get_node_names(const char * implementation_identifier,
  const rmw_node_t * node,
  rcutils_string_array_t * node_names)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != implementation_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (rmw_check_zero_rmw_string_array(node_names) != RMW_RET_OK) {
    return RMW_RET_ERROR;
  }

  DDSDomainParticipant * participant = static_cast<ConnextNodeInfo *>(node->data)->participant;
  DDS_InstanceHandleSeq handles;
  if (participant->get_discovered_participants(handles) != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("unable to fetch discovered participants.");
    return RMW_RET_ERROR;
  }
  auto length = handles.length() + 1;  // add yourself
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_ret_t rcutils_ret = rcutils_string_array_init(node_names, length, &allocator);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
    return rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
  }

  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = participant->get_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default participant qos");
    return RMW_RET_ERROR;
  }
  node_names->data[0] = rcutils_strdup(participant_qos.participant_name.name, allocator);
  if (!node_names->data[0]) {
    RMW_SET_ERROR_MSG("could not allocate memory for node name")
    return RMW_RET_BAD_ALLOC;
  }


  for (auto i = 1; i < length; ++i) {
    ParticipantBuiltinTopicData pbtd;
    auto dds_ret = participant->get_discovered_participant_data(pbtd, handles[i - 1]);
    const char * name = pbtd.participant_name.name;
    if (!name || dds_ret != DDS_RETCODE_OK) {
      name = "(no name)";
    }
    node_names->data[i] = rcutils_strdup(name, allocator);
    if (!node_names->data[i]) {
      RMW_SET_ERROR_MSG("could not allocate memory for node name")
      return RMW_RET_BAD_ALLOC;
    }
  }

  return RMW_RET_OK;
}

rmw_ret_t
count_publishers(const char * implementation_identifier,
  const rmw_node_t * node,
  const char * topic_name,
  size_t * count)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != implementation_identifier) {
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

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->publisher_listener) {
    RMW_SET_ERROR_MSG("publisher listener handle is null");
    return RMW_RET_ERROR;
  }

  const auto & topic_names_and_types = node_info->publisher_listener->topic_names_and_types;
  auto it = std::find_if(
    topic_names_and_types.begin(),
    topic_names_and_types.end(),
    [&](auto tnt) -> bool {
    auto fqdn = _demangle_if_ros_topic(tnt.first);
    if (fqdn == topic_name) {
      return true;
    }
    return false;
  });
  if (it == topic_names_and_types.end()) {
    *count = 0;
  } else {
    *count = it->second.size();
  }
  return RMW_RET_OK;
}

rmw_ret_t
count_subscribers(const char * implementation_identifier,
  const rmw_node_t * node,
  const char * topic_name,
  size_t * count)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != implementation_identifier) {
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

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->subscriber_listener) {
    RMW_SET_ERROR_MSG("subscriber listener handle is null");
    return RMW_RET_ERROR;
  }

  const auto & topic_names_and_types = node_info->subscriber_listener->topic_names_and_types;
  auto it = std::find_if(
    topic_names_and_types.begin(),
    topic_names_and_types.end(),
    [&](auto tnt) -> bool {
    auto fqdn = _demangle_if_ros_topic(tnt.first);
    if (fqdn == topic_name) {
      return true;
    }
    return false;
  });
  if (it == topic_names_and_types.end()) {
    *count = 0;
  } else {
    *count = it->second.size();
  }
  return RMW_RET_OK;
}

RMW_CONNEXT_SHARED_CPP_PUBLIC
const rmw_guard_condition_t *
node_get_graph_guard_condition(const rmw_node_t * node)
{
  // node argument is checked in calling function.

  ConnextNodeInfo * node_info = static_cast<ConnextNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return nullptr;
  }

  return node_info->graph_guard_condition;
}
