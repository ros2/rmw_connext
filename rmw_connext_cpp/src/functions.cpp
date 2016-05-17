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
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

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
#include <ndds/ndds_cpp.h>
#include <ndds/ndds_requestreply_cpp.h>
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

#include "rmw/rmw.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/types.h"

#include "rmw/impl/cpp/macros.hpp"

#include "rosidl_typesupport_connext_c/identifier.h"
#include "rosidl_typesupport_connext_cpp/identifier.hpp"
#include "rosidl_typesupport_connext_cpp/message_type_support.h"
#include "rosidl_typesupport_connext_cpp/service_type_support.h"

#include "rmw_connext_shared_cpp/shared_functions.hpp"
#include "rmw_connext_shared_cpp/types.hpp"

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
const char * rti_connext_identifier = "rmw_connext_cpp";


struct ConnextStaticPublisherInfo
{
  DDSPublisher * dds_publisher_;
  DDSDataWriter * topic_writer_;
  const message_type_support_callbacks_t * callbacks_;
  rmw_gid_t publisher_gid;
};

struct ConnextStaticSubscriberInfo
{
  DDSSubscriber * dds_subscriber_;
  DDSDataReader * topic_reader_;
  DDSReadCondition * read_condition_;
  bool ignore_local_publications;
  const message_type_support_callbacks_t * callbacks_;
};

struct ConnextStaticClientInfo
{
  void * requester_;
  DDSDataReader * response_datareader_;
  DDSReadCondition * read_condition_;
  const service_type_support_callbacks_t * callbacks_;
};

struct ConnextStaticServiceInfo
{
  void * replier_;
  DDSDataReader * request_datareader_;
  DDSReadCondition * read_condition_;
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
  return init();
}

rmw_node_t *
rmw_create_node(const char * name, size_t domain_id)
{
  return create_node(rti_connext_identifier, name, domain_id);
}

rmw_ret_t
rmw_destroy_node(rmw_node_t * node)
{
  return destroy_node(rti_connext_identifier, node);
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
    node->implementation_identifier, rti_connext_identifier,
    return NULL)

  if (!type_support) {
    RMW_SET_ERROR_MSG("type support handle is null");
    return NULL;
  }
  if (type_support->typesupport_identifier !=
    rosidl_typesupport_connext_cpp::typesupport_connext_identifier &&
    type_support->typesupport_identifier != rosidl_typesupport_connext_c__identifier)
  {
    RMW_SET_ERROR_MSG("type support identifier did not match valid typesupports");
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

  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
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
    RMW_SET_ERROR_MSG("failed to allocate publisher");
    goto fail;
  }

  registered = callbacks->register_type(participant, type_name.c_str());
  if (!registered) {
    RMW_SET_ERROR_MSG("failed to register type");
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
    RMW_SET_ERROR_MSG("failed to create datawriter");
    goto fail;
  }

  // Allocate memory for the ConnextStaticPublisherInfo object.
  buf = rmw_allocate(sizeof(ConnextStaticPublisherInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the ConnextStaticPublisherInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(publisher_info, buf, goto fail, ConnextStaticPublisherInfo)
  buf = nullptr;  // Only free the publisher_info pointer; don't need the buf pointer anymore.
  publisher_info->dds_publisher_ = dds_publisher;
  publisher_info->topic_writer_ = topic_writer;
  publisher_info->callbacks_ = callbacks;
  publisher_info->publisher_gid.implementation_identifier = rti_connext_identifier;
  static_assert(
    sizeof(ConnextPublisherGID) <= RMW_GID_STORAGE_SIZE,
    "RMW_GID_STORAGE_SIZE insufficient to store the rmw_connext_cpp GID implemenation."
  );
  // Zero the data memory.
  memset(publisher_info->publisher_gid.data, 0, RMW_GID_STORAGE_SIZE);
  {
    auto publisher_gid =
      reinterpret_cast<ConnextPublisherGID *>(publisher_info->publisher_gid.data);
    publisher_gid->publication_handle = topic_writer->get_instance_handle();
  }
  publisher_info->publisher_gid.implementation_identifier = rti_connext_identifier;

  publisher->implementation_identifier = rti_connext_identifier;
  publisher->data = publisher_info;
  publisher->topic_name = reinterpret_cast<const char *>(rmw_allocate(strlen(topic_name) + 1));
  if (!publisher->topic_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  memcpy(const_cast<char *>(publisher->topic_name), topic_name, strlen(topic_name) + 1);

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
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier, rti_connext_identifier,
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
  // TODO(wjwwood): need to figure out when to unregister types with the participant.
  ConnextStaticPublisherInfo * publisher_info =
    static_cast<ConnextStaticPublisherInfo *>(publisher->data);
  if (publisher_info) {
    DDSPublisher * dds_publisher = publisher_info->dds_publisher_;
    if (dds_publisher) {
      if (publisher_info->topic_writer_) {
        if (dds_publisher->delete_datawriter(publisher_info->topic_writer_) != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete datawriter");
          return RMW_RET_ERROR;
        }
        publisher_info->topic_writer_ = nullptr;
      }
      if (participant->delete_publisher(dds_publisher) != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to delete publisher");
        return RMW_RET_ERROR;
      }
      publisher_info->dds_publisher_ = nullptr;
    } else if (publisher_info->topic_writer_) {
      RMW_SET_ERROR_MSG("cannot delete datawriter because the publisher is null");
      return RMW_RET_ERROR;
    }
    RMW_TRY_DESTRUCTOR(
      publisher_info->~ConnextStaticPublisherInfo(),
      ConnextStaticPublisherInfo, return RMW_RET_ERROR)
    rmw_free(publisher_info);
    publisher->data = nullptr;
  }
  if (publisher->topic_name) {
    rmw_free(const_cast<char *>(publisher->topic_name));
  }
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
  if (publisher->implementation_identifier != rti_connext_identifier) {
    RMW_SET_ERROR_MSG("publisher handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!ros_message) {
    RMW_SET_ERROR_MSG("ros message handle is null");
    return RMW_RET_ERROR;
  }

  ConnextStaticPublisherInfo * publisher_info =
    static_cast<ConnextStaticPublisherInfo *>(publisher->data);
  if (!publisher_info) {
    RMW_SET_ERROR_MSG("publisher info handle is null");
    return RMW_RET_ERROR;
  }
  const message_type_support_callbacks_t * callbacks = publisher_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  DDSDataWriter * topic_writer = publisher_info->topic_writer_;
  if (!topic_writer) {
    RMW_SET_ERROR_MSG("topic writer handle is null");
    return RMW_RET_ERROR;
  }

  bool published = callbacks->publish(topic_writer, ros_message);
  if (!published) {
    RMW_SET_ERROR_MSG("failed to publish message");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_subscription_t *
rmw_create_subscription(const rmw_node_t * node,
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
    node->implementation_identifier, rti_connext_identifier,
    return NULL)

  if (!type_support) {
    RMW_SET_ERROR_MSG("type support handle is null");
    return NULL;
  }

  if (type_support->typesupport_identifier !=
    rosidl_typesupport_connext_cpp::typesupport_connext_identifier &&
    type_support->typesupport_identifier != rosidl_typesupport_connext_c__identifier)
  {
    RMW_SET_ERROR_MSG("type support identifier did not match valid typesupports");
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

  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
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
  DDSReadCondition * read_condition = nullptr;
  void * buf = nullptr;
  ConnextStaticSubscriberInfo * subscriber_info = nullptr;
  // Begin initializing elements.
  subscription = rmw_subscription_allocate();
  if (!subscription) {
    RMW_SET_ERROR_MSG("failed to allocate subscription");
    goto fail;
  }

  registered = callbacks->register_type(participant, type_name.c_str());
  if (!registered) {
    RMW_SET_ERROR_MSG("failed to register type");
    goto fail;
  }

  status = participant->get_default_subscriber_qos(subscriber_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default subscriber qos");
    goto fail;
  }

  dds_subscriber = participant->create_subscriber(subscriber_qos, NULL, DDS_STATUS_MASK_NONE);
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
    topic, datareader_qos,
    NULL, DDS_STATUS_MASK_NONE);
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

  // Allocate memory for the ConnextStaticSubscriberInfo object.
  buf = rmw_allocate(sizeof(ConnextStaticSubscriberInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the ConnextStaticSubscriberInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(subscriber_info, buf, goto fail, ConnextStaticSubscriberInfo)
  buf = nullptr;  // Only free the subscriber_info pointer; don't need the buf pointer anymore.
  subscriber_info->dds_subscriber_ = dds_subscriber;
  subscriber_info->topic_reader_ = topic_reader;
  subscriber_info->read_condition_ = read_condition;
  subscriber_info->callbacks_ = callbacks;
  subscriber_info->ignore_local_publications = ignore_local_publications;

  subscription->implementation_identifier = rti_connext_identifier;
  subscription->data = subscriber_info;

  subscription->topic_name = reinterpret_cast<const char *>(
    rmw_allocate(strlen(topic_name) + 1));
  if (!subscription->topic_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  memcpy(const_cast<char *>(subscription->topic_name), topic_name, strlen(topic_name) + 1);
  return subscription;
fail:
  if (subscription) {
    rmw_subscription_free(subscription);
  }
  // Assumption: participant is valid.
  if (dds_subscriber) {
    if (topic_reader) {
      if (read_condition) {
        if (topic_reader->delete_readcondition(read_condition) != DDS_RETCODE_OK) {
          std::stringstream ss;
          ss << "leaking readcondition while handling failure at " <<
            __FILE__ << ":" << __LINE__ << '\n';
          (std::cerr << ss.str()).flush();
        }
      }
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
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  if (!subscription) {
    RMW_SET_ERROR_MSG("subscription handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription handle,
    subscription->implementation_identifier, rti_connext_identifier,
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
  // TODO(wjwwood): need to figure out when to unregister types with the participant.
  auto result = RMW_RET_OK;
  ConnextStaticSubscriberInfo * subscriber_info =
    static_cast<ConnextStaticSubscriberInfo *>(subscription->data);
  if (subscriber_info) {
    auto dds_subscriber = subscriber_info->dds_subscriber_;
    if (dds_subscriber) {
      auto topic_reader = subscriber_info->topic_reader_;
      if (topic_reader) {
        auto read_condition = subscriber_info->read_condition_;
        if (read_condition) {
          if (topic_reader->delete_readcondition(read_condition) != DDS_RETCODE_OK) {
            RMW_SET_ERROR_MSG("failed to delete readcondition");
            result = RMW_RET_ERROR;
          }
          subscriber_info->read_condition_ = nullptr;
        }
        if (dds_subscriber->delete_datareader(topic_reader) != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete datareader");
          result = RMW_RET_ERROR;
        }
        subscriber_info->topic_reader_ = nullptr;
      } else if (subscriber_info->read_condition_) {
        RMW_SET_ERROR_MSG("cannot delete readcondition because the datareader is null");
        result = RMW_RET_ERROR;
      }
      if (participant->delete_subscriber(dds_subscriber) != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to delete subscriber");
        result = RMW_RET_ERROR;
      }
      subscriber_info->dds_subscriber_ = nullptr;
    } else if (subscriber_info->topic_reader_) {
      RMW_SET_ERROR_MSG("cannot delete datareader because the subscriber is null");
      result = RMW_RET_ERROR;
    }
    RMW_TRY_DESTRUCTOR(
      subscriber_info->~ConnextStaticSubscriberInfo(),
      ConnextStaticSubscriberInfo, result = RMW_RET_ERROR)
    rmw_free(subscriber_info);
    subscription->data = nullptr;
  }
  if (subscription->topic_name) {
    rmw_free(const_cast<char *>(subscription->topic_name));
  }
  rmw_subscription_free(subscription);

  return result;
}

rmw_ret_t
_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken,
  DDS_InstanceHandle_t * sending_publication_handle)
{
  if (!subscription) {
    RMW_SET_ERROR_MSG("subscription handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription handle,
    subscription->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  if (!ros_message) {
    RMW_SET_ERROR_MSG("ros message handle is null");
    return RMW_RET_ERROR;
  }
  if (!taken) {
    RMW_SET_ERROR_MSG("taken handle is null");
    return RMW_RET_ERROR;
  }

  ConnextStaticSubscriberInfo * subscriber_info =
    static_cast<ConnextStaticSubscriberInfo *>(subscription->data);
  if (!subscriber_info) {
    RMW_SET_ERROR_MSG("subscriber info handle is null");
    return RMW_RET_ERROR;
  }
  DDSDataReader * topic_reader = subscriber_info->topic_reader_;
  if (!topic_reader) {
    RMW_SET_ERROR_MSG("topic reader handle is null");
    return RMW_RET_ERROR;
  }
  const message_type_support_callbacks_t * callbacks = subscriber_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  bool success = callbacks->take(
    topic_reader, subscriber_info->ignore_local_publications, ros_message, taken,
    sending_publication_handle);

  return success ? RMW_RET_OK : RMW_RET_ERROR;
}

rmw_ret_t
rmw_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken)
{
  return _take(subscription, ros_message, taken, nullptr);
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
  auto ret = _take(subscription, ros_message, taken, &sending_publication_handle);
  if (ret != RMW_RET_OK) {
    // Error string is already set.
    return RMW_RET_ERROR;
  }

  rmw_gid_t * sender_gid = &message_info->publisher_gid;
  sender_gid->implementation_identifier = rti_connext_identifier;
  memset(sender_gid->data, 0, RMW_GID_STORAGE_SIZE);
  auto detail = reinterpret_cast<ConnextPublisherGID *>(sender_gid->data);
  detail->publication_handle = sending_publication_handle;

  return RMW_RET_OK;
}

rmw_guard_condition_t *
rmw_create_guard_condition()
{
  return create_guard_condition(rti_connext_identifier);
}

rmw_ret_t
rmw_destroy_guard_condition(rmw_guard_condition_t * guard_condition)
{
  return destroy_guard_condition(rti_connext_identifier, guard_condition);
}

rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition_handle)
{
  return trigger_guard_condition(rti_connext_identifier, guard_condition_handle);
}

rmw_waitset_t *
rmw_create_waitset(size_t max_conditions)
{
  return create_waitset(rti_connext_identifier, max_conditions);
}

rmw_ret_t
rmw_destroy_waitset(rmw_waitset_t * waitset)
{
  return destroy_waitset(rti_connext_identifier, waitset);
}

rmw_ret_t
rmw_wait(rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_waitset_t * waitset,
  const rmw_time_t * wait_timeout)
{
  return wait<ConnextStaticSubscriberInfo, ConnextStaticServiceInfo, ConnextStaticClientInfo>
           (rti_connext_identifier, subscriptions, guard_conditions, services, clients, waitset,
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
    node->implementation_identifier, rti_connext_identifier,
    return NULL)

  if (!type_support) {
    RMW_SET_ERROR_MSG("type support handle is null");
    return NULL;
  }

  if (type_support->typesupport_identifier !=
    rosidl_typesupport_connext_cpp::typesupport_connext_identifier &&
    type_support->typesupport_identifier != rosidl_typesupport_connext_c__identifier)
  {
    RMW_SET_ERROR_MSG("type support identifier did not match valid typesupports");
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

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_client_t * client = nullptr;
  DDS_DataReaderQos datareader_qos;
  DDS_DataWriterQos datawriter_qos;
  DDSDataReader * response_datareader = nullptr;
  DDSReadCondition * read_condition = nullptr;
  void * requester = nullptr;
  void * buf = nullptr;
  ConnextStaticClientInfo * client_info = nullptr;

  // Begin inializing elements.
  client = rmw_client_allocate();
  if (!client) {
    RMW_SET_ERROR_MSG("failed to allocate client");
    goto fail;
  }

  if (!get_datareader_qos(participant, *qos_profile, datareader_qos)) {
    // error string was set within the function
    goto fail;
  }

  if (!get_datawriter_qos(participant, *qos_profile, datawriter_qos)) {
    // error string was set within the function
    goto fail;
  }

  requester = callbacks->create_requester(
    participant, service_name, &datareader_qos, &datawriter_qos,
    reinterpret_cast<void **>(&response_datareader), &rmw_allocate);
  if (!requester) {
    RMW_SET_ERROR_MSG("failed to create requester");
    goto fail;
  }
  if (!response_datareader) {
    RMW_SET_ERROR_MSG("data reader handle is null");
    goto fail;
  }

  read_condition = response_datareader->create_readcondition(
    DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  buf = rmw_allocate(sizeof(ConnextStaticClientInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the ConnextStaticClientInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(client_info, buf, goto fail, ConnextStaticClientInfo)
  buf = nullptr;  // Only free the client_info pointer; don't need the buf pointer anymore.
  client_info->requester_ = requester;
  client_info->callbacks_ = callbacks;
  client_info->response_datareader_ = response_datareader;
  client_info->read_condition_ = read_condition;

  client->implementation_identifier = rti_connext_identifier;
  client->data = client_info;
  client->service_name = reinterpret_cast<const char *>(rmw_allocate(strlen(service_name) + 1));
  if (!client->service_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  memcpy(const_cast<char *>(client->service_name), service_name, strlen(service_name) + 1);
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
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  ConnextStaticClientInfo * client_info = static_cast<ConnextStaticClientInfo *>(client->data);

  if (client_info) {
    auto response_datareader = client_info->response_datareader_;
    if (response_datareader) {
      auto read_condition = client_info->read_condition_;
      if (read_condition) {
        if (response_datareader->delete_readcondition(read_condition) != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete readcondition");
          result = RMW_RET_ERROR;
        }
        client_info->read_condition_ = nullptr;
      }
    } else if (client_info->read_condition_) {
      RMW_SET_ERROR_MSG("cannot delete readcondition because the datareader is null");
      result = RMW_RET_ERROR;
    }
    const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
    if (callbacks) {
      if (client_info->requester_) {
        callbacks->destroy_requester(client_info->requester_, &rmw_free);
      }
    }

    RMW_TRY_DESTRUCTOR(
      client_info->~ConnextStaticClientInfo(),
      ConnextStaticClientInfo, result = RMW_RET_ERROR)
    rmw_free(client_info);
    client->data = nullptr;
    if (client->service_name) {
      rmw_free(const_cast<char *>(client->service_name));
    }
  }
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
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  if (!ros_request) {
    RMW_SET_ERROR_MSG("ros request handle is null");
    return RMW_RET_ERROR;
  }

  ConnextStaticClientInfo * client_info = static_cast<ConnextStaticClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }
  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  void * requester = client_info->requester_;
  if (!requester) {
    RMW_SET_ERROR_MSG("requester handle is null");
    return RMW_RET_ERROR;
  }

  *sequence_id = callbacks->send_request(requester, ros_request);
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
    node->implementation_identifier, rti_connext_identifier,
    return NULL)

  if (!type_support) {
    RMW_SET_ERROR_MSG("type support handle is null");
    return NULL;
  }

  if (type_support->typesupport_identifier !=
    rosidl_typesupport_connext_cpp::typesupport_connext_identifier &&
    type_support->typesupport_identifier != rosidl_typesupport_connext_c__identifier)
  {
    RMW_SET_ERROR_MSG("type support identifier did not match valid typesupports");
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

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }

  // Past this point, a failure results in unrolling code in the goto fail block.
  DDSDataReader * request_datareader = nullptr;
  DDSReadCondition * read_condition = nullptr;
  DDS_DataReaderQos datareader_qos;
  DDS_DataWriterQos datawriter_qos;
  void * replier = nullptr;
  void * buf = nullptr;
  ConnextStaticServiceInfo * service_info = nullptr;
  rmw_service_t * service = nullptr;
  // Begin initializing elements.
  service = rmw_service_allocate();
  if (!service) {
    RMW_SET_ERROR_MSG("service handle is null");
    goto fail;
  }

  if (!get_datareader_qos(participant, *qos_profile, datareader_qos)) {
    // error string was set within the function
    goto fail;
  }

  if (!get_datawriter_qos(participant, *qos_profile, datawriter_qos)) {
    // error string was set within the function
    goto fail;
  }

  replier = callbacks->create_replier(
    participant, service_name, &datareader_qos, &datawriter_qos,
    reinterpret_cast<void **>(&request_datareader), &rmw_allocate);
  if (!replier) {
    RMW_SET_ERROR_MSG("failed to create replier");
    goto fail;
  }
  if (!request_datareader) {
    RMW_SET_ERROR_MSG("data reader handle is null");
    goto fail;
  }

  read_condition = request_datareader->create_readcondition(
    DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  buf = rmw_allocate(sizeof(ConnextStaticServiceInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the ConnextStaticServiceInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(service_info, buf, goto fail, ConnextStaticServiceInfo)
  buf = nullptr;  // Only free the service_info pointer; don't need the buf pointer anymore.
  service_info->replier_ = replier;
  service_info->callbacks_ = callbacks;
  service_info->request_datareader_ = request_datareader;
  service_info->read_condition_ = read_condition;

  service->implementation_identifier = rti_connext_identifier;
  service->data = service_info;
  service->service_name = reinterpret_cast<const char *>(rmw_allocate(strlen(service_name) + 1));
  if (!service->service_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  memcpy(const_cast<char *>(service->service_name), service_name, strlen(service_name) + 1);
  return service;
fail:
  if (service) {
    rmw_service_free(service);
  }
  if (request_datareader) {
    if (read_condition) {
      if (request_datareader->delete_readcondition(read_condition) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "leaking readcondition while handling failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    }
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
    RMW_SET_ERROR_MSG("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  ConnextStaticServiceInfo * service_info = static_cast<ConnextStaticServiceInfo *>(service->data);

  if (service_info) {
    auto request_datareader = service_info->request_datareader_;
    if (request_datareader) {
      auto read_condition = service_info->read_condition_;
      if (read_condition) {
        if (request_datareader->delete_readcondition(read_condition) != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete readcondition");
          result = RMW_RET_ERROR;
        }
        service_info->read_condition_ = nullptr;
      }
    } else if (service_info->read_condition_) {
      RMW_SET_ERROR_MSG("cannot delete readcondition because the datareader is null");
      result = RMW_RET_ERROR;
    }
    const service_type_support_callbacks_t * callbacks = service_info->callbacks_;
    if (callbacks) {
      if (service_info->replier_) {
        callbacks->destroy_replier(service_info->replier_, &rmw_free);
      }
    }

    RMW_TRY_DESTRUCTOR(
      service_info->~ConnextStaticServiceInfo(),
      ConnextStaticServiceInfo, result = RMW_RET_ERROR)
    rmw_free(service_info);
    service->data = nullptr;
    if (service->service_name) {
      rmw_free(const_cast<char *>(service->service_name));
    }
  }
  rmw_service_free(service);

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
    service->implementation_identifier, rti_connext_identifier,
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

  ConnextStaticServiceInfo * service_info =
    static_cast<ConnextStaticServiceInfo *>(service->data);
  if (!service_info) {
    RMW_SET_ERROR_MSG("service info handle is null");
    return RMW_RET_ERROR;
  }

  void * replier = service_info->replier_;
  if (!replier) {
    RMW_SET_ERROR_MSG("replier handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks = service_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  *taken = callbacks->take_request(replier, request_header, ros_request);

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
    client->implementation_identifier, rti_connext_identifier,
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

  ConnextStaticClientInfo * client_info =
    static_cast<ConnextStaticClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }

  void * requester = client_info->requester_;
  if (!requester) {
    RMW_SET_ERROR_MSG("requester handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  *taken = callbacks->take_response(requester, request_header, ros_response);

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
    service->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  if (!request_header) {
    RMW_SET_ERROR_MSG("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_response) {
    RMW_SET_ERROR_MSG("ros response handle is null");
    return RMW_RET_ERROR;
  }

  ConnextStaticServiceInfo * service_info =
    static_cast<ConnextStaticServiceInfo *>(service->data);
  if (!service_info) {
    RMW_SET_ERROR_MSG("service info handle is null");
    return RMW_RET_ERROR;
  }

  void * replier = service_info->replier_;
  if (!replier) {
    RMW_SET_ERROR_MSG("replier handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks = service_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  callbacks->send_response(replier, request_header, ros_response);

  return RMW_RET_OK;
}


rmw_ret_t
rmw_get_topic_names_and_types(
  const rmw_node_t * node,
  rmw_topic_names_and_types_t * topic_names_and_types)
{
  return get_topic_names_and_types(rti_connext_identifier, node,
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
  return count_publishers(rti_connext_identifier, node, topic_name, count);
}

rmw_ret_t
rmw_count_subscribers(
  const rmw_node_t * node,
  const char * topic_name,
  size_t * count)
{
  return count_subscribers(rti_connext_identifier, node, topic_name, count);
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
    rti_connext_identifier,
    return RMW_RET_ERROR)
  if (!gid) {
    RMW_SET_ERROR_MSG("gid is null");
    return RMW_RET_ERROR;
  }

  const ConnextStaticPublisherInfo * publisher_info =
    static_cast<const ConnextStaticPublisherInfo *>(publisher->data);
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
    rti_connext_identifier,
    return RMW_RET_ERROR)
  if (!gid2) {
    RMW_SET_ERROR_MSG("gid2 is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    gid2,
    gid2->implementation_identifier,
    rti_connext_identifier,
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
    node->implementation_identifier, rti_connext_identifier,
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
    node->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, rti_connext_identifier,
    return RMW_RET_ERROR)

  if (!is_available) {
    RMW_SET_ERROR_MSG("is_available is null");
    return RMW_RET_ERROR;
  }

  ConnextStaticClientInfo * client_info =
    static_cast<ConnextStaticClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  void * requester = client_info->requester_;
  if (!requester) {
    RMW_SET_ERROR_MSG("requester handle is null");
    return RMW_RET_ERROR;
  }
  const char * request_topic_name = callbacks->get_request_topic_name(requester);
  if (!request_topic_name) {
    RMW_SET_ERROR_MSG("could not get request topic name");
    return RMW_RET_ERROR;
  }

  *is_available = false;
  // In the Connext RPC implementation, a server is ready when:
  //   - At least one server is subscribed to the request topic.
  //   - At least one server is publishing to the reponse topic.
  size_t number_of_request_subscribers = 0;
  rmw_ret_t ret = rmw_count_subscribers(
    node,
    request_topic_name,
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
