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

#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

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

#include <rmw/impl/cpp/macros.hpp>

#include "rosidl_typesupport_connext_cpp/identifier.hpp"
#include <rosidl_typesupport_connext_cpp/message_type_support.h>
#include <rosidl_typesupport_connext_cpp/service_type_support.h>

inline std::string
_create_type_name(
  const message_type_support_callbacks_t * callbacks,
  const std::string & sep)
{
  return
    std::string(callbacks->package_name) +
    "::" + sep + "::dds_::" + callbacks->message_name + "_";
}

extern "C"
{

const char * rti_connext_identifier = "connext_static";

struct ConnextStaticPublisherInfo
{
  DDSPublisher * dds_publisher_;
  DDSDataWriter * topic_writer_;
  const message_type_support_callbacks_t * callbacks_;
};

struct ConnextStaticSubscriberInfo
{
  DDSSubscriber * dds_subscriber_;
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
    domain, participant_qos, NULL,
    DDS_STATUS_MASK_NONE);
  if (!participant) {
    rmw_set_error_string("failed to create participant");
    return NULL;
  }

  rmw_node_t * node_handle = rmw_node_allocate();
  if (!node_handle) {
    rmw_set_error_string("failed to allocate memory for node handle");
    if (dpf_->delete_participant(participant) != DDS_RETCODE_OK) {
      std::stringstream ss;
      ss << "leaking participant while handling failue at " <<
        __FILE__ << ":" << __LINE__;
      (std::cerr << ss.str()).flush();
    }
    return NULL;
  }
  node_handle->implementation_identifier = rti_connext_identifier;
  node_handle->data = participant;

  return node_handle;
}

rmw_ret_t
rmw_destroy_node(rmw_node_t * node)
{
  if (!node) {
    rmw_set_error_string("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  DDSDomainParticipantFactory * dpf_ = DDSDomainParticipantFactory::get_instance();
  if (!dpf_) {
    rmw_set_error_string("failed to get participant factory");
    return RMW_RET_ERROR;
  }

  DDSDomainParticipant * participant = static_cast<DDSDomainParticipant *>(node->data);
  if (!participant) {
    rmw_set_error_string("participant handle is null");
  }
  // This unregisters types and destroys topics which were shared between
  // publishers and subscribers and could not be cleaned up in the delete functions.
  participant->delete_contained_entities();

  DDS_ReturnCode_t ret = dpf_->delete_participant(participant);
  if (ret != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to delete participant");
    return RMW_RET_ERROR;
  }

  node->data = nullptr;
  rmw_node_free(node);

  return RMW_RET_OK;
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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_identifier,
    return NULL)

  if (!type_support) {
    rmw_set_error_string("type support handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support handle,
    type_support->typesupport_identifier,
    rosidl_typesupport_connext_cpp::typesupport_connext_identifier,
    return NULL)

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
  std::string type_name = _create_type_name(callbacks, "msg");
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_publisher_t * publisher = nullptr;
  bool registered;
  DDS_PublisherQos publisher_qos;
  DDS_ReturnCode_t status;
  DDSPublisher * dds_publisher = nullptr;
  DDSTopic * topic;
  DDSTopicDescription * topic_description = nullptr;
  DDS_DataWriterQos datawriter_qos;
  DDSDataWriter * topic_writer = nullptr;
  void * buf = nullptr;
  ConnextStaticPublisherInfo * publisher_info = nullptr;
  // Begin initializing elements
  publisher = rmw_publisher_allocate();
  if (!publisher) {
    rmw_set_error_string("failed to allocate publisher");
    goto fail;
  }

  registered = callbacks->register_type(participant, type_name.c_str());
  if (!registered) {
    rmw_set_error_string("failed to register type");
    goto fail;
  }

  status = participant->get_default_publisher_qos(publisher_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default publisher qos");
    goto fail;
  }

  dds_publisher = participant->create_publisher(
    publisher_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!dds_publisher) {
    rmw_set_error_string("failed to create publisher");
    goto fail;
  }

  topic_description = participant->lookup_topicdescription(topic_name);
  if (!topic_description) {
    DDS_TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS_RETCODE_OK) {
      rmw_set_error_string("failed to get default topic qos");
      goto fail;
    }

    topic = participant->create_topic(
      topic_name, type_name.c_str(), default_topic_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!topic) {
      rmw_set_error_string("failed to create topic");
      goto fail;
    }
  } else {
    DDS_Duration_t timeout = DDS_Duration_t::from_seconds(0);
    topic = participant->find_topic(topic_name, timeout);
    if (!topic) {
      rmw_set_error_string("failed to find topic");
      goto fail;
    }
  }

  status = participant->get_default_datawriter_qos(datawriter_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default datawriter qos");
    goto fail;
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
    goto fail;
  }

  topic_writer = dds_publisher->create_datawriter(
    topic, datawriter_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!topic_writer) {
    rmw_set_error_string("failed to create datawriter");
    goto fail;
  }

  // Allocate memory for the ConnextStaticPublisherInfo object.
  buf = rmw_allocate(sizeof(ConnextStaticPublisherInfo));
  if (!buf) {
    rmw_set_error_string("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the ConnextStaticPublisherInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(publisher_info, buf, goto fail, ConnextStaticPublisherInfo)
  buf = nullptr;  // Only free the publisher_info pointer; don't need the buf pointer anymore.
  publisher_info->dds_publisher_ = dds_publisher;
  publisher_info->topic_writer_ = topic_writer;
  publisher_info->callbacks_ = callbacks;

  publisher->implementation_identifier = rti_connext_identifier;
  publisher->data = publisher_info;
  return publisher;
fail:
  if (publisher) {
    rmw_publisher_free(publisher);
  }
  // Assumption: participant is valid.
  if (dds_publisher) {
    if (topic_writer) {
      if (dds_publisher->delete_datawriter(topic_writer) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "leaking datawriter while handling failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    }
    if (participant->delete_publisher(dds_publisher) != DDS_RETCODE_OK) {
      std::stringstream ss;
      ss << "leaking publisher while handling failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (publisher_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      publisher_info->~ConnextStaticPublisherInfo(), ConnextStaticPublisherInfo)
    rmw_free(publisher_info);
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
    rmw_set_error_string("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  if (!publisher) {
    rmw_set_error_string("publisher handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  DDSDomainParticipant * participant = (DDSDomainParticipant *)node->data;
  if (!participant) {
    rmw_set_error_string("participant handle is null");
    return RMW_RET_ERROR;
  }
  // TODO(wjwwood): need to figure out when to unregister types with the participant.
  ConnextStaticPublisherInfo * publisher_info = (ConnextStaticPublisherInfo *)publisher->data;
  if (publisher_info) {
    DDSPublisher * dds_publisher = publisher_info->dds_publisher_;
    if (dds_publisher) {
      if (publisher_info->topic_writer_) {
        if (dds_publisher->delete_datawriter(publisher_info->topic_writer_) != DDS_RETCODE_OK) {
          rmw_set_error_string("failed to delete datawriter");
          return RMW_RET_ERROR;
        }
        publisher_info->topic_writer_ = nullptr;
      }
      if (participant->delete_publisher(dds_publisher) != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to delete publisher");
        return RMW_RET_ERROR;
      }
      publisher_info->dds_publisher_ = nullptr;
    } else if (publisher_info->topic_writer_) {
      rmw_set_error_string("cannot delete datawriter because the publisher is null");
      return RMW_RET_ERROR;
    }
    RMW_TRY_DESTRUCTOR(
      publisher_info->~ConnextStaticPublisherInfo(),
      ConnextStaticPublisherInfo, return RMW_RET_ERROR)
    rmw_free(publisher_info);
    publisher->data = nullptr;
  }
  rmw_publisher_free(publisher);

  return RMW_RET_OK;
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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_identifier,
    return NULL)

  if (!type_support) {
    rmw_set_error_string("type support handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support handle,
    type_support->typesupport_identifier,
    rosidl_typesupport_connext_cpp::typesupport_connext_identifier,
    return NULL)

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
  std::string type_name = _create_type_name(callbacks, "msg");
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_subscription_t * subscription = nullptr;
  bool registered;
  DDS_SubscriberQos subscriber_qos;
  DDS_ReturnCode_t status;
  DDSSubscriber * dds_subscriber = nullptr;
  DDSTopic * topic;
  DDSTopicDescription * topic_description = nullptr;
  DDS_DataReaderQos datareader_qos;
  DDSDataReader * topic_reader = nullptr;
  void * buf = nullptr;
  ConnextStaticSubscriberInfo * subscriber_info = nullptr;
  // Begin initializing elements.
  subscription = rmw_subscription_allocate();
  if (!subscription) {
    rmw_set_error_string("failed to allocate subscription");
    goto fail;
  }

  registered = callbacks->register_type(participant, type_name.c_str());
  if (!registered) {
    rmw_set_error_string("failed to register type");
    goto fail;
  }

  status = participant->get_default_subscriber_qos(subscriber_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default subscriber qos");
    goto fail;
  }

  dds_subscriber = participant->create_subscriber(subscriber_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!dds_subscriber) {
    rmw_set_error_string("failed to create subscriber");
    goto fail;
  }

  topic_description = participant->lookup_topicdescription(topic_name);
  if (!topic_description) {
    DDS_TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS_RETCODE_OK) {
      rmw_set_error_string("failed to get default topic qos");
      goto fail;
    }

    topic = participant->create_topic(
      topic_name, type_name.c_str(), default_topic_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!topic) {
      rmw_set_error_string("failed to create topic");
      goto fail;
    }
  } else {
    DDS_Duration_t timeout = DDS_Duration_t::from_seconds(0);
    topic = participant->find_topic(topic_name, timeout);
    if (!topic) {
      rmw_set_error_string("failed to find topic");
      goto fail;
    }
  }

  status = participant->get_default_datareader_qos(datareader_qos);
  if (status != DDS_RETCODE_OK) {
    rmw_set_error_string("failed to get default datareader qos");
    goto fail;
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

  topic_reader = dds_subscriber->create_datareader(
    topic, datareader_qos,
    NULL, DDS_STATUS_MASK_NONE);
  if (!topic_reader) {
    rmw_set_error_string("failed to create datareader");
    goto fail;
  }

  // Allocate memory for the ConnextStaticSubscriberInfo object.
  buf = rmw_allocate(sizeof(ConnextStaticSubscriberInfo));
  if (!buf) {
    rmw_set_error_string("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the ConnextStaticSubscriberInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(subscriber_info, buf, goto fail, ConnextStaticSubscriberInfo)
  buf = nullptr;  // Only free the subscriber_info pointer; don't need the buf pointer anymore.
  subscriber_info->dds_subscriber_ = dds_subscriber;
  subscriber_info->topic_reader_ = topic_reader;
  subscriber_info->callbacks_ = callbacks;
  subscriber_info->ignore_local_publications = ignore_local_publications;

  subscription->implementation_identifier = rti_connext_identifier;
  subscription->data = subscriber_info;
  return subscription;
fail:
  if (subscription) {
    rmw_subscription_free(subscription);
  }
  // Assumption: participant is valid.
  if (dds_subscriber) {
    if (topic_reader) {
      if (dds_subscriber->delete_datareader(topic_reader) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "leaking datareader while handling failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    }
    if (participant->delete_subscriber(dds_subscriber) != DDS_RETCODE_OK) {
      std::stringstream ss;
      std::cerr << "leaking subscriber while handling failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (subscriber_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      subscriber_info->~ConnextStaticSubscriberInfo(), ConnextStaticSubscriberInfo)
    rmw_free(subscriber_info);
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
    rmw_set_error_string("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  if (!subscription) {
    rmw_set_error_string("subscription handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription handle,
    subscription->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  DDSDomainParticipant * participant = (DDSDomainParticipant *)node->data;
  if (!participant) {
    rmw_set_error_string("participant handle is null");
    return RMW_RET_ERROR;
  }
  // TODO(wjwwood): need to figure out when to unregister types with the participant.
  ConnextStaticSubscriberInfo * subscriber_info =
    (ConnextStaticSubscriberInfo *)subscription->data;
  if (subscriber_info) {
    auto dds_subscriber = subscriber_info->dds_subscriber_;
    if (dds_subscriber) {
      auto topic_reader = subscriber_info->topic_reader_;
      if (topic_reader) {
        if (dds_subscriber->delete_datareader(topic_reader) != DDS_RETCODE_OK) {
          rmw_set_error_string("failed to delete datareader");
          return RMW_RET_ERROR;
        }
        subscriber_info->topic_reader_ = nullptr;
      }
      if (participant->delete_subscriber(dds_subscriber) != DDS_RETCODE_OK) {
        rmw_set_error_string("failed to delete subscriber");
        return RMW_RET_ERROR;
      }
      subscriber_info->dds_subscriber_ = nullptr;
    } else if (subscriber_info->topic_reader_) {
      rmw_set_error_string("cannot delete datareader because the subscriber is null");
      return RMW_RET_ERROR;
    }
    RMW_TRY_DESTRUCTOR(
      subscriber_info->~ConnextStaticSubscriberInfo(),
      ConnextStaticSubscriberInfo, return RMW_RET_ERROR)
    rmw_free(subscriber_info);
    subscription->data = nullptr;
  }
  rmw_subscription_free(subscription);

  return RMW_RET_OK;
}

rmw_ret_t
rmw_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken)
{
  if (!subscription) {
    rmw_set_error_string("subscription handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription handle,
    subscription->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

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
  rmw_guard_condition_t * guard_condition = rmw_guard_condition_allocate();
  if (!guard_condition) {
    rmw_set_error_string("failed to allocate guard condition");
    return NULL;
  }
  // Allocate memory for the DDSGuardCondition object.
  DDSGuardCondition * dds_guard_condition = nullptr;
  void * buf = rmw_allocate(sizeof(DDSGuardCondition));
  if (!buf) {
    rmw_set_error_string("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the DDSGuardCondition in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(dds_guard_condition, buf, goto fail, DDSGuardCondition)
  buf = nullptr;  // Only free the dds_guard_condition pointer; don't need the buf pointer anymore.
  guard_condition->implementation_identifier = rti_connext_identifier;
  guard_condition->data = new DDSGuardCondition();
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
rmw_destroy_guard_condition(rmw_guard_condition_t * guard_condition)
{
  if (!guard_condition) {
    rmw_set_error_string("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    guard condition handle,
    guard_condition->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  RMW_TRY_DESTRUCTOR(
    ((DDSGuardCondition *)guard_condition->data)->~DDSGuardCondition(),
    DDSGuardCondition, result = RMW_RET_ERROR)
  rmw_guard_condition_free(guard_condition);
  return result;
}

rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition_handle)
{
  if (!guard_condition_handle) {
    rmw_set_error_string("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    guard condition handle,
    guard_condition_handle->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_identifier,
    return NULL)

  if (!type_support) {
    rmw_set_error_string("type support handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support handle,
    type_support->typesupport_identifier,
    rosidl_typesupport_connext_cpp::typesupport_connext_identifier,
    return NULL)

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
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_client_t * client = nullptr;
  DDSDataReader * response_datareader = nullptr;
  void * requester = nullptr;
  void * buf = nullptr;
  ConnextStaticClientInfo * client_info = nullptr;
  // Begin inializing elements.
  client = rmw_client_allocate();
  if (!client) {
    rmw_set_error_string("failed to allocate client");
    goto fail;
  }
  requester = callbacks->create_requester(
    participant, service_name, reinterpret_cast<void **>(&response_datareader));
  if (!requester) {
    rmw_set_error_string("failed to create requester");
    goto fail;
  }
  if (!response_datareader) {
    rmw_set_error_string("data reader handle is null");
    goto fail;
  }

  buf = rmw_allocate(sizeof(ConnextStaticClientInfo));
  if (!buf) {
    rmw_set_error_string("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the ConnextStaticClientInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(client_info, buf, goto fail, ConnextStaticClientInfo)
  buf = nullptr;  // Only free the client_info pointer; don't need the buf pointer anymore.
  client_info->requester_ = requester;
  client_info->callbacks_ = callbacks;
  client_info->response_datareader_ = response_datareader;

  client->implementation_identifier = rti_connext_identifier;
  client->data = client_info;
  return client;
fail:
  if (client) {
    rmw_client_free(client);
  }
  if (response_datareader) {
    if (participant->delete_datareader(response_datareader) != DDS_RETCODE_OK) {
      std::stringstream ss;
      ss << "leaking datareader while handling failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  // TODO(wjwwood): deallocate requester (currently allocated with new elsewhere)
  if (client_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      client_info->~ConnextStaticClientInfo(), ConnextStaticClientInfo)
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
    rmw_set_error_string("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  // TODO(esteve): de-allocate Requester and response DataReader
  RMW_TRY_DESTRUCTOR(
    static_cast<ConnextStaticClientInfo *>(client->data)->~ConnextStaticClientInfo(),
    ConnextStaticClientInfo,
    result = RMW_RET_ERROR)
  rmw_client_free(client);
  return result;
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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_identifier,
    return NULL)

  if (!type_support) {
    rmw_set_error_string("type support handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    type support handle,
    type_support->typesupport_identifier,
    rosidl_typesupport_connext_cpp::typesupport_connext_identifier,
    return NULL)

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
  // Past this point, a failure results in unrolling code in the goto fail block.
  DDSDataReader * request_datareader = nullptr;
  void * replier = nullptr;
  void * buf = nullptr;
  ConnextStaticServiceInfo * service_info = nullptr;
  rmw_service_t * service = nullptr;
  // Begin initializing elements.
  service = rmw_service_allocate();
  if (!service) {
    rmw_set_error_string("service handle is null");
    goto fail;
  }
  replier = callbacks->create_replier(
    participant, service_name, reinterpret_cast<void **>(&request_datareader));
  if (!replier) {
    rmw_set_error_string("failed to create replier");
    goto fail;
  }
  if (!request_datareader) {
    rmw_set_error_string("data reader handle is null");
    goto fail;
  }

  buf = rmw_allocate(sizeof(ConnextStaticServiceInfo));
  if (!buf) {
    rmw_set_error_string("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the ConnextStaticServiceInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(service_info, buf, goto fail, ConnextStaticServiceInfo)
  buf = nullptr;  // Only free the service_info pointer; don't need the buf pointer anymore.
  service_info->replier_ = replier;
  service_info->callbacks_ = callbacks;
  service_info->request_datareader_ = request_datareader;

  service->implementation_identifier = rti_connext_identifier;
  service->data = service_info;
  return service;
fail:
  if (service) {
    rmw_service_free(service);
  }
  if (request_datareader) {
    if (participant->delete_datareader(request_datareader) != RMW_RET_OK) {
      std::stringstream ss;
      ss << "leaking datareader while handling failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (service_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      service_info->~ConnextStaticServiceInfo(), ConnextStaticServiceInfo)
    rmw_free(service_info);
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
    rmw_set_error_string("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  // TODO(esteve): de-allocate Replier and request DataReader
  RMW_TRY_DESTRUCTOR(
    static_cast<ConnextStaticServiceInfo *>(service->data)->~ConnextStaticServiceInfo(),
    ConnextStaticServiceInfo,
    result = RMW_RET_ERROR)
  rmw_service_free(service);
  return result;
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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

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
