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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#ifdef __clang__
# pragma clang diagnostic ignored "-Wdeprecated-register"
#endif
#include <ndds/ndds_cpp.h>
#include <ndds/ndds_requestreply_cpp.h>
#pragma GCC diagnostic pop

#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/types.h>

#include "rosidl_typesupport_connext_cpp/identifier.hpp"
#include <rosidl_typesupport_connext_cpp/message_type_support.h>
#include <rosidl_typesupport_connext_cpp/service_type_support.h>

extern "C"
{

const char * rti_connext_identifier = "connext_static";

struct ConnextStaticPublisherInfo
{
  DDSDataWriter * topic_writer_;
  const message_type_support_callbacks_t * callbacks_;
};

struct ConnextStaticSubscriberInfo
{
  DDSDataReader * topic_reader_;
  bool ignore_local_publications;
  const message_type_support_callbacks_t * callbacks_;
};

struct ConnextStaticClientInfo
{
  void * requester_;
  DDSDataReader * response_datareader_;
  const service_type_support_callbacks_t * callbacks_;
};

struct ConnextStaticServiceInfo
{
  void * replier_;
  DDSDataReader * request_datareader_;
  const service_type_support_callbacks_t * callbacks_;
};

const char *
rmw_get_implementation_identifier()
{
  return rti_connext_identifier;
}

rmw_ret_t
rmw_init()
{
  DDSDomainParticipantFactory * dpf_ = DDSDomainParticipantFactory::get_instance();
  if (!dpf_) {
    rmw_set_error_string("failed to get participant factory");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_node_t *
rmw_create_node(const char * name)
{
  DDSDomainParticipantFactory * dpf_ = DDSDomainParticipantFactory::get_instance();
  if (!dpf_) {
    rmw_set_error_string("failed to get participant factory");
    return NULL;
  }

  // use loopback interface to enable cross vendor communication
  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = dpf_->get_default_participant_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default participant qos");
    return NULL;
  }
  status = DDSPropertyQosPolicyHelper::add_property(
    participant_qos.property,
    "dds.transport.UDPv4.builtin.ignore_loopback_interface",
    "0",
    DDS_BOOLEAN_FALSE);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to add qos property");
    return NULL;
  }
  status = DDSPropertyQosPolicyHelper::add_property(
    participant_qos.property,
    "dds.transport.use_510_compatible_locator_kinds",
    "1",
    DDS_BOOLEAN_FALSE);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to add qos property");
    return NULL;
  }

  DDS_DomainId_t domain = 0;

  DDSDomainParticipant * participant = dpf_->create_participant(
    //domain, DDS_PARTICIPANT_QOS_DEFAULT, NULL,
    domain, participant_qos, NULL,
    DDS_STATUS_MASK_NONE);
  if (!participant) {
    rmw_set_error_string("failed to create participant");
    return NULL;
  }

  rmw_node_t * node_handle = new rmw_node_t;
  node_handle->implementation_identifier = rti_connext_identifier;
  node_handle->data = participant;

  return node_handle;
}

rmw_publisher_t *
rmw_create_publisher(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  size_t queue_size)
{
  if (!node) {
    rmw_set_error_string("node handle is null");
    return NULL;
  }
  if (node->implementation_identifier != rti_connext_identifier) {
    rmw_set_error_string("node handle is not from this rmw implementation");
    return NULL;
  }
  if (!type_support) {
    rmw_set_error_string("type support handle is null");
    return NULL;
  }
  if (type_support->typesupport_identifier !=
    rosidl_typesupport_connext_cpp::typesupport_connext_identifier)
  {
    rmw_set_error_string("type support is not from this rmw implementation");
    return NULL;
  }

  DDSDomainParticipant * participant = static_cast<DDSDomainParticipant *>(node->data);
  if (!participant) {
    rmw_set_error_string("participant handle is null");
    return NULL;
  }

  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    rmw_set_error_string("callbacks handle is null");
    return NULL;
  }
  std::string type_name = std::string(callbacks->package_name) + "::msg::dds_::" +
    callbacks->message_name + "_";

  bool registered = callbacks->register_type(participant, type_name.c_str());
  if (!registered) {
    rmw_set_error_string("failed to register type");
    return NULL;
  }


  DDS_PublisherQos publisher_qos;
  DDS_ReturnCode_t status = participant->get_default_publisher_qos(publisher_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default publisher qos");
    return NULL;
  }

  DDSPublisher * dds_publisher = participant->create_publisher(
    publisher_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!dds_publisher) {
    rmw_set_error_string("failed to create publisher");
    return NULL;
  }


  DDSTopic * topic;
  DDSTopicDescription * topic_description = participant->lookup_topicdescription(topic_name);
  if (!topic_description) {
    DDS_TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS_RETCODE_OK) {
      rmw_set_error_string("failed to get default topic qos");
      return NULL;
    }

    topic = participant->create_topic(
      topic_name, type_name.c_str(), default_topic_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!topic) {
      rmw_set_error_string("failed to create topic");
      return NULL;
    }
  } else {
    DDS_Duration_t timeout = DDS_Duration_t::from_seconds(0);
    topic = participant->find_topic(topic_name, timeout);
    if (!topic) {
      rmw_set_error_string("failed to find topic");
      return NULL;
    }
  }


  DDS_DataWriterQos datawriter_qos;
  status = participant->get_default_datawriter_qos(datawriter_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default datawriter qos");
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

  status = DDSPropertyQosPolicyHelper::add_property(
    datawriter_qos.property,
    "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
    "4096",
    DDS_BOOLEAN_FALSE);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to add qos property");
    return NULL;
  }

  DDSDataWriter * topic_writer = dds_publisher->create_datawriter(
    topic, datawriter_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!topic_writer) {
    rmw_set_error_string("failed to create datawriter");
    return NULL;
  }


  ConnextStaticPublisherInfo * publisher_info = new ConnextStaticPublisherInfo();
  publisher_info->topic_writer_ = topic_writer;
  publisher_info->callbacks_ = callbacks;

  rmw_publisher_t * publisher = new rmw_publisher_t;
  publisher->implementation_identifier = rti_connext_identifier;
  publisher->data = publisher_info;
  return publisher;
}

rmw_ret_t
rmw_publish(const rmw_publisher_t * publisher, const void * ros_message)
{
  if (!publisher) {
    rmw_set_error_string("publisher handle is null");
    return RMW_RET_ERROR;
  }
  if (publisher->implementation_identifier != rti_connext_identifier) {
    rmw_set_error_string("publisher handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!ros_message) {
    rmw_set_error_string("ros message handle is null");
    return RMW_RET_ERROR;
  }

  ConnextStaticPublisherInfo * publisher_info =
    static_cast<ConnextStaticPublisherInfo *>(publisher->data);
  if (!publisher_info) {
    rmw_set_error_string("publisher info handle is null");
    return RMW_RET_ERROR;
  }
  const message_type_support_callbacks_t * callbacks = publisher_info->callbacks_;
  if (!callbacks) {
    rmw_set_error_string("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  DDSDataWriter * topic_writer = publisher_info->topic_writer_;
  if (!topic_writer) {
    rmw_set_error_string("topic writer handle is null");
    return RMW_RET_ERROR;
  }

  bool published = callbacks->publish(topic_writer, ros_message);
  if (!published) {
    rmw_set_error_string("failed to publish message");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_subscription_t *
rmw_create_subscription(const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  size_t queue_size,
  bool ignore_local_publications)
{
  if (!node) {
    rmw_set_error_string("node handle is null");
    return NULL;
  }
  if (node->implementation_identifier != rti_connext_identifier) {
    rmw_set_error_string("node handle is not from this rmw implementation");
    return NULL;
  }
  if (!type_support) {
    rmw_set_error_string("type support handle is null");
    return NULL;
  }
  if (type_support->typesupport_identifier !=
    rosidl_typesupport_connext_cpp::typesupport_connext_identifier)
  {
    rmw_set_error_string("type support is not from this rmw implementation");
    return NULL;
  }

  DDSDomainParticipant * participant = static_cast<DDSDomainParticipant *>(node->data);
  if (!participant) {
    rmw_set_error_string("participant handle is null");
    return NULL;
  }


  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    rmw_set_error_string("callbacks handle is null");
    return NULL;
  }
  std::string type_name = std::string(callbacks->package_name) + "::msg::dds_::" +
    callbacks->message_name + "_";

  bool registered = callbacks->register_type(participant, type_name.c_str());
  if (!registered) {
    rmw_set_error_string("failed to register type");
    return NULL;
  }

  DDS_SubscriberQos subscriber_qos;
  DDS_ReturnCode_t status = participant->get_default_subscriber_qos(subscriber_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default subscriber qos");
    return NULL;
  }

  DDSSubscriber * dds_subscriber = participant->create_subscriber(
    subscriber_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!dds_subscriber) {
    rmw_set_error_string("failed to create subscriber");
    return NULL;
  }

  DDSTopic * topic;
  DDSTopicDescription * topic_description = participant->lookup_topicdescription(topic_name);
  if (!topic_description) {
    DDS_TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS_RETCODE_OK) {
      rmw_set_error_string("failed to get default topic qos");
      return NULL;
    }

    topic = participant->create_topic(
      topic_name, type_name.c_str(), default_topic_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!topic) {
      rmw_set_error_string("failed to create topic");
      return NULL;
    }
  } else {
    DDS_Duration_t timeout = DDS_Duration_t::from_seconds(0);
    topic = participant->find_topic(topic_name, timeout);
    if (!topic) {
      rmw_set_error_string("failed to find topic");
      return NULL;
    }
  }

  DDS_DataReaderQos datareader_qos;
  status = participant->get_default_datareader_qos(datareader_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default datareader qos");
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
    topic, datareader_qos,
    NULL, DDS_STATUS_MASK_NONE);
  if (!topic_reader) {
    rmw_set_error_string("failed to create datareader");
    return NULL;
  }


  ConnextStaticSubscriberInfo * subscriber_info = new ConnextStaticSubscriberInfo();
  subscriber_info->topic_reader_ = topic_reader;
  subscriber_info->callbacks_ = callbacks;
  subscriber_info->ignore_local_publications = ignore_local_publications;

  rmw_subscription_t * subscription = new rmw_subscription_t;
  subscription->implementation_identifier = rti_connext_identifier;
  subscription->data = subscriber_info;
  return subscription;
}

rmw_ret_t
rmw_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken)
{
  if (!subscription) {
    rmw_set_error_string("subscription handle is null");
    return RMW_RET_ERROR;
  }
  if (subscription->implementation_identifier != rti_connext_identifier) {
    rmw_set_error_string("subscriber handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!ros_message) {
    rmw_set_error_string("ros message handle is null");
    return RMW_RET_ERROR;
  }
  if (!taken) {
    rmw_set_error_string("taken handle is null");
    return RMW_RET_ERROR;
  }

  ConnextStaticSubscriberInfo * subscriber_info =
    static_cast<ConnextStaticSubscriberInfo *>(subscription->data);
  if (!subscriber_info) {
    rmw_set_error_string("subscriber info handle is null");
    return RMW_RET_ERROR;
  }
  DDSDataReader * topic_reader = subscriber_info->topic_reader_;
  if (!topic_reader) {
    rmw_set_error_string("topic reader handle is null");
    return RMW_RET_ERROR;
  }
  const message_type_support_callbacks_t * callbacks = subscriber_info->callbacks_;
  if (!callbacks) {
    rmw_set_error_string("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  bool success = callbacks->take(
    topic_reader, subscriber_info->ignore_local_publications, ros_message, taken);

  return success ? RMW_RET_OK : RMW_RET_ERROR;
}

rmw_guard_condition_t *
rmw_create_guard_condition()
{
  rmw_guard_condition_t * guard_condition = new rmw_guard_condition_t;
  guard_condition->implementation_identifier = rti_connext_identifier;
  guard_condition->data = new DDSGuardCondition();
  return guard_condition;
}

rmw_ret_t
rmw_destroy_guard_condition(rmw_guard_condition_t * guard_condition)
{
  if (!guard_condition) {
    rmw_set_error_string("guard condition handle is null");
    return RMW_RET_ERROR;
  }

  delete (DDSGuardCondition *)guard_condition->data;
  delete guard_condition;
  return RMW_RET_OK;
}

rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition_handle)
{
  if (!guard_condition_handle) {
    rmw_set_error_string("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  if (guard_condition_handle->implementation_identifier != rti_connext_identifier) {
    rmw_set_error_string("guard condition handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }

  DDSGuardCondition * guard_condition =
    static_cast<DDSGuardCondition *>(guard_condition_handle->data);
  if (!guard_condition) {
    rmw_set_error_string("guard condition is null");
    return RMW_RET_ERROR;
  }
  DDS_ReturnCode_t status = guard_condition->set_trigger_value(DDS_BOOLEAN_TRUE);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to set trigger value");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_ret_t
rmw_wait(rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  bool non_blocking)
{
  DDSWaitSet waitset;

  // add a condition for each subscriber
  for (unsigned long i = 0; i < subscriptions->subscriber_count; ++i) {
    ConnextStaticSubscriberInfo * subscriber_info =
      static_cast<ConnextStaticSubscriberInfo *>(subscriptions->subscribers[i]);
    if (!subscriber_info) {
      rmw_set_error_string("subscriber info handle is null");
      return RMW_RET_ERROR;
    }
    DDSDataReader * topic_reader = subscriber_info->topic_reader_;
    if (!topic_reader) {
      rmw_set_error_string("topic reader handle is null");
      return RMW_RET_ERROR;
    }
    DDSStatusCondition * condition = topic_reader->get_statuscondition();
    if (!condition) {
      rmw_set_error_string("condition handle is null");
      return RMW_RET_ERROR;
    }
    DDS_ReturnCode_t status = condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
    if (status != DDS_RETCODE_OK) {
      rmw_set_error_string("failed to set enabled statuses");
      return RMW_RET_ERROR;
    }
    status = waitset.attach_condition(condition);
    if (status != DDS_RETCODE_OK) {
      rmw_set_error_string("failed to attach condition");
      return RMW_RET_ERROR;
    }
  }

  // add a condition for each guard condition
  for (unsigned long i = 0; i < guard_conditions->guard_condition_count; ++i) {
    DDSGuardCondition * guard_condition =
      static_cast<DDSGuardCondition *>(guard_conditions->guard_conditions[i]);
    if (!guard_condition) {
      rmw_set_error_string("guard condition handle is null");
      return RMW_RET_ERROR;
    }
    DDS_ReturnCode_t status = waitset.attach_condition(guard_condition);
    if (status != DDS_RETCODE_OK) {
      rmw_set_error_string("failed to attach condition");
      return RMW_RET_ERROR;
    }
  }

  // add a condition for each service
  for (unsigned long i = 0; i < services->service_count; ++i) {
    ConnextStaticServiceInfo * service_info =
      static_cast<ConnextStaticServiceInfo *>(services->services[i]);
    if (!service_info) {
      rmw_set_error_string("service info handle is null");
      return RMW_RET_ERROR;
    }
    DDSDataReader * request_datareader = service_info->request_datareader_;
    if (!request_datareader) {
      rmw_set_error_string("request datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDSStatusCondition * condition = request_datareader->get_statuscondition();
    if (!condition) {
      rmw_set_error_string("condition handle is null");
      return RMW_RET_ERROR;
    }
    DDS_ReturnCode_t status = condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
    if (status != DDS_RETCODE_OK) {
      rmw_set_error_string("failed to set enabled statuses");
      return RMW_RET_ERROR;
    }
    status = waitset.attach_condition(condition);
    if (status != DDS_RETCODE_OK) {
      rmw_set_error_string("failed to attach condition");
      return RMW_RET_ERROR;
    }
  }

  // add a condition for each client
  for (unsigned long i = 0; i < clients->client_count; ++i) {
    ConnextStaticClientInfo * client_info =
      static_cast<ConnextStaticClientInfo *>(clients->clients[i]);
    if (!client_info) {
      rmw_set_error_string("client info handle is null");
      return RMW_RET_ERROR;
    }
    DDSDataReader * response_datareader = client_info->response_datareader_;
    if (!response_datareader) {
      rmw_set_error_string("response datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDSStatusCondition * condition = response_datareader->get_statuscondition();
    if (!condition) {
      rmw_set_error_string("condition handle is null");
      return RMW_RET_ERROR;
    }
    DDS_ReturnCode_t status = condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
    if (status != DDS_RETCODE_OK) {
      rmw_set_error_string("failed to set enabled statuses");
      return RMW_RET_ERROR;
    }
    status = waitset.attach_condition(condition);
    if (status != DDS_RETCODE_OK) {
      rmw_set_error_string("failed to attach condition");
      return RMW_RET_ERROR;
    }
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
      rmw_set_error_string("failed to wait on waitset");
      return RMW_RET_ERROR;
    }
  }

  // set subscriber handles to zero for all not triggered conditions
  for (unsigned long i = 0; i < subscriptions->subscriber_count; ++i) {
    ConnextStaticSubscriberInfo * subscriber_info =
      static_cast<ConnextStaticSubscriberInfo *>(subscriptions->subscribers[i]);
    if (!subscriber_info) {
      rmw_set_error_string("subscriber info handle is null");
      return RMW_RET_ERROR;
    }
    DDSDataReader * topic_reader = subscriber_info->topic_reader_;
    if (!topic_reader) {
      rmw_set_error_string("topic reader handle is null");
      return RMW_RET_ERROR;
    }
    DDSStatusCondition * condition = topic_reader->get_statuscondition();
    if (!condition) {
      rmw_set_error_string("condition handle is null");
      return RMW_RET_ERROR;
    }

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
    DDSCondition * condition =
      static_cast<DDSCondition *>(guard_conditions->guard_conditions[i]);
    if (!condition) {
      rmw_set_error_string("condition handle is null");
      return RMW_RET_ERROR;
    }

    // search for guard condition in active set
    unsigned long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == condition) {
        DDSGuardCondition * guard = static_cast<DDSGuardCondition *>(condition);
        DDS_ReturnCode_t status = guard->set_trigger_value(DDS_BOOLEAN_FALSE);
        if (status != DDS_RETCODE_OK) {
          rmw_set_error_string("failed to set trigger value");
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
  for (unsigned long i = 0; i < services->service_count; ++i) {
    ConnextStaticServiceInfo * service_info =
      static_cast<ConnextStaticServiceInfo *>(services->services[i]);
    if (!service_info) {
      rmw_set_error_string("service info handle is null");
      return RMW_RET_ERROR;
    }
    DDSDataReader * request_datareader = service_info->request_datareader_;
    if (!request_datareader) {
      rmw_set_error_string("request datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDSStatusCondition * condition = request_datareader->get_statuscondition();
    if (!condition) {
      rmw_set_error_string("condition handle is null");
      return RMW_RET_ERROR;
    }

    // search for service condition in active set
    unsigned long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == condition) {
        break;
      }
    }
    // if service condition is not found in the active set
    // reset the subscriber handle
    if (!(j < active_conditions.length())) {
      services->services[i] = 0;
    }
  }

  // set client handles to zero for all not triggered conditions
  for (unsigned long i = 0; i < clients->client_count; ++i) {
    ConnextStaticClientInfo * client_info =
      static_cast<ConnextStaticClientInfo *>(clients->clients[i]);
    if (!client_info) {
      rmw_set_error_string("client info handle is null");
      return RMW_RET_ERROR;
    }
    DDSDataReader * response_datareader = client_info->response_datareader_;
    if (!response_datareader) {
      rmw_set_error_string("response datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDSStatusCondition * condition = response_datareader->get_statuscondition();
    if (!condition) {
      rmw_set_error_string("condition handle is null");
      return RMW_RET_ERROR;
    }

    // search for service condition in active set
    unsigned long j = 0;
    for (; j < active_conditions.length(); ++j) {
      if (active_conditions[j] == condition) {
        break;
      }
    }
    // if client condition is not found in the active set
    // reset the subscriber handle
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
    rmw_set_error_string("node handle is null");
    return NULL;
  }
  if (node->implementation_identifier != rti_connext_identifier) {
    rmw_set_error_string("node handle is not from this rmw implementation");
    return NULL;
  }
  if (!type_support) {
    rmw_set_error_string("type support handle is null");
    return NULL;
  }
  if (type_support->typesupport_identifier !=
    rosidl_typesupport_connext_cpp::typesupport_connext_identifier)
  {
    rmw_set_error_string("type support is not from this rmw implementation");
    return NULL;
  }

  DDSDomainParticipant * participant = static_cast<DDSDomainParticipant *>(node->data);
  if (!participant) {
    rmw_set_error_string("participant handle is null");
    return NULL;
  }

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    rmw_set_error_string("callbacks handle is null");
    return NULL;
  }

  DDSDataReader * response_datareader;
  void * requester = callbacks->create_requester(
    participant, service_name, reinterpret_cast<void **>(&response_datareader));
  if (!requester) {
    rmw_set_error_string("failed to create requester");
    return NULL;
  }
  if (!response_datareader) {
    rmw_set_error_string("data reader handle is null");
    return NULL;
  }

  ConnextStaticClientInfo * client_info = new ConnextStaticClientInfo();
  client_info->requester_ = requester;
  client_info->callbacks_ = callbacks;
  client_info->response_datareader_ = response_datareader;

  rmw_client_t * client = new rmw_client_t;
  client->implementation_identifier = rti_connext_identifier;
  client->data = client_info;
  return client;
}

rmw_ret_t
rmw_destroy_client(rmw_client_t * client)
{
  if (!client) {
    rmw_set_error_string("client handle is null");
    return RMW_RET_ERROR;
  }

  // TODO(esteve): de-allocate Requester and response DataReader
  delete static_cast<ConnextStaticClientInfo *>(client->data);
  delete client;
  return RMW_RET_OK;
}

rmw_ret_t
rmw_send_request(
  const rmw_client_t * client,
  const void * ros_request,
  int64_t * sequence_id)
{
  if (!client) {
    rmw_set_error_string("client handle is null");
    return RMW_RET_ERROR;
  }
  if (client->implementation_identifier != rti_connext_identifier) {
    rmw_set_error_string("client handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!ros_request) {
    rmw_set_error_string("ros request handle is null");
    return RMW_RET_ERROR;
  }

  ConnextStaticClientInfo * client_info = static_cast<ConnextStaticClientInfo *>(client->data);
  if (!client_info) {
    rmw_set_error_string("client info handle is null");
    return RMW_RET_ERROR;
  }
  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  if (!callbacks) {
    rmw_set_error_string("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  void * requester = client_info->requester_;
  if (!requester) {
    rmw_set_error_string("requester handle is null");
    return RMW_RET_ERROR;
  }

  *sequence_id = callbacks->send_request(requester, ros_request);
  return RMW_RET_OK;
}

rmw_service_t *
rmw_create_service(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name)
{
  if (!node) {
    rmw_set_error_string("node handle is null");
    return NULL;
  }
  if (node->implementation_identifier != rti_connext_identifier) {
    rmw_set_error_string("node handle is not from this rmw implementation");
    return NULL;
  }
  if (!type_support) {
    rmw_set_error_string("type support handle is null");
    return NULL;
  }
  if (type_support->typesupport_identifier !=
    rosidl_typesupport_connext_cpp::typesupport_connext_identifier)
  {
    rmw_set_error_string("type support is not from this rmw implementation");
    return NULL;
  }

  DDSDomainParticipant * participant = static_cast<DDSDomainParticipant *>(node->data);
  if (!participant) {
    rmw_set_error_string("participant handle is null");
    return NULL;
  }

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    rmw_set_error_string("callbacks handle is null");
    return NULL;
  }

  DDSDataReader * request_datareader;

  void * replier = callbacks->create_replier(
    participant, service_name, reinterpret_cast<void **>(&request_datareader));
  if (!replier) {
    rmw_set_error_string("failed to create replier");
    return NULL;
  }
  if (!request_datareader) {
    rmw_set_error_string("data reader handle is null");
    return NULL;
  }

  ConnextStaticServiceInfo * service_info = new ConnextStaticServiceInfo();
  service_info->replier_ = replier;
  service_info->callbacks_ = callbacks;
  service_info->request_datareader_ = request_datareader;

  rmw_service_t * service = rmw_service_allocate();
  if (!service) {
    rmw_set_error_string("service handle is null");
    return NULL;
  }
  service->implementation_identifier = rti_connext_identifier;
  service->data = service_info;
  return service;
}

rmw_ret_t
rmw_destroy_service(rmw_service_t * service)
{
  if (!service) {
    rmw_set_error_string("service handle is null");
    return RMW_RET_ERROR;
  }

  // TODO(esteve): de-allocate Replier and request DataReader
  delete static_cast<ConnextStaticServiceInfo *>(service->data);
  delete service;
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
    rmw_set_error_string("service handle is null");
    return RMW_RET_ERROR;
  }
  if (service->implementation_identifier != rti_connext_identifier) {
    rmw_set_error_string("service handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!ros_request_header) {
    rmw_set_error_string("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_request) {
    rmw_set_error_string("ros request handle is null");
    return RMW_RET_ERROR;
  }
  if (!taken) {
    rmw_set_error_string("taken handle is null");
    return RMW_RET_ERROR;
  }

  ConnextStaticServiceInfo * service_info =
    static_cast<ConnextStaticServiceInfo *>(service->data);
  if (!service_info) {
    rmw_set_error_string("service info handle is null");
    return RMW_RET_ERROR;
  }

  void * replier = service_info->replier_;
  if (!replier) {
    rmw_set_error_string("replier handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks = service_info->callbacks_;
  if (!callbacks) {
    rmw_set_error_string("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  *taken = callbacks->take_request(replier, ros_request_header, ros_request);

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
    rmw_set_error_string("client handle is null");
    return RMW_RET_ERROR;
  }
  if (client->implementation_identifier != rti_connext_identifier) {
    rmw_set_error_string("client handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!ros_request_header) {
    rmw_set_error_string("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_response) {
    rmw_set_error_string("ros response handle is null");
    return RMW_RET_ERROR;
  }
  if (!taken) {
    rmw_set_error_string("taken handle is null");
    return RMW_RET_ERROR;
  }

  ConnextStaticClientInfo * client_info =
    static_cast<ConnextStaticClientInfo *>(client->data);
  if (!client_info) {
    rmw_set_error_string("client info handle is null");
    return RMW_RET_ERROR;
  }

  void * requester = client_info->requester_;
  if (!requester) {
    rmw_set_error_string("requester handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  if (!callbacks) {
    rmw_set_error_string("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  *taken = callbacks->take_response(requester, ros_request_header, ros_response);

  return RMW_RET_OK;
}

rmw_ret_t
rmw_send_response(
  const rmw_service_t * service,
  void * ros_request_header,
  void * ros_response)
{
  if (!service) {
    rmw_set_error_string("service handle is null");
    return RMW_RET_ERROR;
  }
  if (service->implementation_identifier != rti_connext_identifier) {
    rmw_set_error_string("service handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!ros_request_header) {
    rmw_set_error_string("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_response) {
    rmw_set_error_string("ros response handle is null");
    return RMW_RET_ERROR;
  }

  ConnextStaticServiceInfo * service_info =
    static_cast<ConnextStaticServiceInfo *>(service->data);
  if (!service_info) {
    rmw_set_error_string("service info handle is null");
    return RMW_RET_ERROR;
  }

  void * replier = service_info->replier_;
  if (!replier) {
    rmw_set_error_string("replier handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks = service_info->callbacks_;
  if (!callbacks) {
    rmw_set_error_string("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  callbacks->send_response(replier, ros_request_header, ros_response);

  return RMW_RET_OK;
}

}
