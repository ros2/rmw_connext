// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#include <map>
#include <set>
#include <string>

#include "rmw/allocators.h"

#include "rmw_connext_shared_cpp/shared_functions.hpp"

void CustomDataReaderListener::add_information(
  const DDS_SampleInfo & sample_info,
  const std::string & topic_name,
  const std::string & type_name)
{
  // store topic name and type name
  auto & topic_types = topic_names_and_types[topic_name];
  topic_types.insert(type_name);
  // store mapping to instance handle
  TopicDescriptor topic_descriptor;
  topic_descriptor.instance_handle = sample_info.instance_handle;
  topic_descriptor.name = topic_name;
  topic_descriptor.type = type_name;
  topic_descriptors.push_back(topic_descriptor);
}

void CustomDataReaderListener::remove_information(
  const DDS_SampleInfo & sample_info)
{
  // find entry by instance handle
  for (auto it = topic_descriptors.begin(); it != topic_descriptors.end(); ++it) {
    if (DDS_InstanceHandle_equals(&it->instance_handle, &sample_info.instance_handle)) {
      // remove entries
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
      add_information(info_seq[i], data_seq[i].topic_name, data_seq[i].type_name);
    } else {
      remove_information(info_seq[i]);
    }
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
      add_information(info_seq[i], data_seq[i].topic_name, data_seq[i].type_name);
    } else {
      remove_information(info_seq[i]);
    }
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

rmw_ret_t check_attach_condition_error(DDS::ReturnCode_t retcode)
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

  return true;
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

rmw_node_t *
create_node(const char * implementation_identifier, const char * name, size_t domain_id)
{
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

  buf = rmw_allocate(sizeof(ConnextNodeInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(node_info, buf, goto fail, ConnextNodeInfo)
  buf = nullptr;
  node_info->participant = participant;
  node_info->publisher_listener = publisher_listener;
  node_info->subscriber_listener = subscriber_listener;

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

  rmw_free(node_info);
  node->data = nullptr;
  rmw_free(const_cast<char *>(node->name));
  node->name = nullptr;
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
  RMW_TRY_PLACEMENT_NEW(dds_guard_condition, buf, goto fail, DDSGuardCondition)
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
  rmw_guard_condition_free(guard_condition);
  return result;
}

rmw_waitset_t *
create_waitset(const char * implementation_identifier,
  rmw_guard_conditions_t * fixed_guard_conditions, size_t max_conditions)
{
  rmw_waitset_t * waitset = rmw_waitset_allocate();

  ConnextWaitSetInfo * waitset_info = nullptr;
  DDSGuardCondition * dds_guard_cond = nullptr;
  DDS_ReturnCode_t ret;

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
    waitset_info->waitset, waitset_info->waitset, goto fail, DDSWaitSet)

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
      waitset_info->active_conditions, waitset_info->active_conditions, goto fail, DDSConditionSeq)

    RMW_TRY_PLACEMENT_NEW(
      waitset_info->attached_conditions, waitset_info->attached_conditions, goto fail,
      DDSConditionSeq)
  }

  waitset->fixed_guard_conditions = fixed_guard_conditions;
  if (fixed_guard_conditions && fixed_guard_conditions->guard_condition_count > 0) {
    if (!fixed_guard_conditions->guard_conditions) {
      RMW_SET_ERROR_MSG("Received invalid guard condition array");
      goto fail;
    }
    // We also need to attach the fixed guard conditions to the waitset (and detach them in destroy)
    for (size_t i = 0; i < fixed_guard_conditions->guard_condition_count; ++i) {
      void * guard_cond = fixed_guard_conditions->guard_conditions[i];
      if (!guard_cond) {
        RMW_SET_ERROR_MSG("Received invalid guard condition");
        goto fail;
      }
      dds_guard_cond = static_cast<DDSGuardCondition *>(guard_cond);
      ret = waitset_info->waitset->attach_condition(dds_guard_cond);
      if (ret != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("Failed to attach condition to waitset");
      }
    }
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
  DDS::WaitSet * dds_waitset =
    static_cast<DDSWaitSet *>(waitset_info->waitset);

  const rmw_guard_conditions_t * fixed_guard_conditions = waitset->fixed_guard_conditions;
  if (fixed_guard_conditions && fixed_guard_conditions->guard_conditions && dds_waitset) {
    // We also need to attach the fixed guard conditions to the waitset (and detach them in destroy)
    for (size_t i = 0; i < fixed_guard_conditions->guard_condition_count; ++i) {
      void * guard_cond = static_cast<rmw_guard_condition_t *>(
        fixed_guard_conditions->guard_conditions[i]);
      if (!guard_cond) {
        continue;
      }
      DDS::GuardCondition * dds_guard_cond = static_cast<DDSGuardCondition *>(guard_cond);
      if (!dds_guard_cond) {
        continue;
      }
      DDS_ReturnCode_t ret = dds_waitset->detach_condition(dds_guard_cond);
      if (ret != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("Failed to detach condition from waitset");
      }
    }
  }

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

rmw_ret_t
get_topic_names_and_types(const char * implementation_identifier,
  const rmw_node_t * node,
  rmw_topic_names_and_types_t * topic_names_and_types)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != implementation_identifier) {
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
  auto it = topic_names_and_types.find(topic_name);
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
  auto it = topic_names_and_types.find(topic_name);
  if (it == topic_names_and_types.end()) {
    *count = 0;
  } else {
    *count = it->second.size();
  }
  return RMW_RET_OK;
}
