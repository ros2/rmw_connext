// Copyright 2014-2017 Open Source Robotics Foundation, Inc.
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

#include <string>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"

#include "rmw_connext_shared_cpp/types.hpp"
#include "rmw_connext_shared_cpp/qos.hpp"

#include "rmw_connext_cpp/connext_static_client_info.hpp"
#include "rmw_connext_cpp/identifier.hpp"
#include "process_topic_and_service_names.hpp"
#include "type_support_common.hpp"

// Uncomment this to get extra console output about discovery.
// This affects code in this file, but there is a similar variable in:
//   rmw_connext_shared_cpp/shared_functions.cpp
// #define DISCOVERY_DEBUG_LOGGING 1

extern "C"
{
rmw_client_t *
rmw_create_client(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_supports,
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

  RMW_CONNEXT_EXTRACT_SERVICE_TYPESUPPORT(type_supports, type_support)

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
  DDS_SubscriberQos subscriber_qos;
  DDS_ReturnCode_t status;
  DDS_PublisherQos publisher_qos;
  DDS_DataReaderQos datareader_qos;
  DDS_DataWriterQos datawriter_qos;
  DDS::Publisher * dds_publisher = nullptr;
  DDS::Subscriber * dds_subscriber = nullptr;
  DDS::DataReader * response_datareader = nullptr;
  DDS::DataWriter * request_datawriter = nullptr;
  DDS::ReadCondition * read_condition = nullptr;
  void * requester = nullptr;
  void * buf = nullptr;
  ConnextStaticClientInfo * client_info = nullptr;
  rmw_client_t * client = nullptr;
  std::string mangled_name = "";

  // memory allocations for namespacing
  char * request_topic_str = nullptr;
  char * response_topic_str = nullptr;

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

  // allocating memory for request topic and response topic strings
  if (!_process_service_name(
      service_name,
      qos_profile->avoid_ros_namespace_conventions,
      &request_topic_str,
      &response_topic_str))
  {
    goto fail;
  }

  // TODO(karsten1987): For now, I'll expose the datawriter
  // to access the respective DDSPublisher object.
  // This has to be evaluated whether or not to provide a
  // Subscriber/Publisher object directly with preset partitions.
  requester = callbacks->create_requester(
    participant, request_topic_str, response_topic_str,
    &datareader_qos, &datawriter_qos,
    reinterpret_cast<void **>(&response_datareader),
    reinterpret_cast<void **>(&request_datawriter),
    &rmw_allocate);
  DDS_String_free(request_topic_str);
  request_topic_str = nullptr;
  DDS_String_free(response_topic_str);
  response_topic_str = nullptr;

  if (!requester) {
    RMW_SET_ERROR_MSG("failed to create requester");
    goto fail;
  }
  if (!response_datareader) {
    RMW_SET_ERROR_MSG("data reader handle is null");
    goto fail;
  }
  if (!request_datawriter) {
    RMW_SET_ERROR_MSG("data request handle is null");
    goto fail;
  }

  dds_subscriber = response_datareader->get_subscriber();
  status = participant->get_default_subscriber_qos(subscriber_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default subscriber qos");
    goto fail;
  }

  dds_publisher = request_datawriter->get_publisher();
  status = participant->get_default_publisher_qos(publisher_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default subscriber qos");
    goto fail;
  }

  // update attached subscriber
  dds_subscriber->set_qos(subscriber_qos);

  // update attached publisher
  dds_publisher->set_qos(publisher_qos);

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
  RMW_TRY_PLACEMENT_NEW(client_info, buf, goto fail, ConnextStaticClientInfo, )
  buf = nullptr;  // Only free the client_info pointer; don't need the buf pointer anymore.
  client_info->requester_ = requester;
  client_info->callbacks_ = callbacks;
  client_info->response_datareader_ = response_datareader;
  client_info->read_condition_ = read_condition;

  client->implementation_identifier = rti_connext_identifier;
  client->data = client_info;
  client->service_name = reinterpret_cast<const char *>(rmw_allocate(strlen(service_name) + 1));
  if (!client->service_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for service name");
    goto fail;
  }
  memcpy(const_cast<char *>(client->service_name), service_name, strlen(service_name) + 1);

  mangled_name =
    response_datareader->get_topicdescription()->get_name();
  node_info->subscriber_listener->add_information(
    response_datareader->get_instance_handle(),
    mangled_name.c_str(),
    response_datareader->get_topicdescription()->get_type_name(),
    EntityType::Subscriber);
  node_info->subscriber_listener->trigger_graph_guard_condition();

  mangled_name =
    request_datawriter->get_topic()->get_name();
  node_info->publisher_listener->add_information(
    request_datawriter->get_instance_handle(),
    mangled_name.c_str(),
    request_datawriter->get_topic()->get_type_name(),
    EntityType::Publisher);
  node_info->publisher_listener->trigger_graph_guard_condition();

// TODO(karsten1987): replace this block with logging macros
#ifdef DISCOVERY_DEBUG_LOGGING
  fprintf(stderr, "****** Creating Client Details: *********\n");
  fprintf(stderr, "Subscriber topic %s\n", response_datareader->get_topicdescription()->get_name());
  fprintf(stderr, "Subscriber address %p\n", static_cast<void *>(dds_subscriber));
  fprintf(stderr, "Publisher topic %s\n", request_datawriter->get_topic()->get_name());
  fprintf(stderr, "Publisher address %p\n", static_cast<void *>(dds_publisher));
  fprintf(stderr, "******\n");
#endif

  return client;
fail:
  if (request_topic_str) {
    DDS_String_free(request_topic_str);
    request_topic_str = nullptr;
  }
  if (response_topic_str) {
    DDS_String_free(response_topic_str);
    response_topic_str = nullptr;
  }
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
rmw_destroy_client(rmw_node_t * node, rmw_client_t * client)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
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

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);

  if (client_info) {
    auto response_datareader = client_info->response_datareader_;

    node_info->subscriber_listener->remove_information(
      client_info->response_datareader_->get_instance_handle(), EntityType::Subscriber);
    DDS::DataWriter * request_datawriter = static_cast<DDS::DataWriter *>(
      client_info->callbacks_->get_request_datawriter(client_info->requester_));
    node_info->subscriber_listener->trigger_graph_guard_condition();

    node_info->publisher_listener->remove_information(
      request_datawriter->get_instance_handle(),
      EntityType::Publisher);
    node_info->publisher_listener->trigger_graph_guard_condition();

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
}  // extern "C"
