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

#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/types.h"

#include "rmw_connext_shared_cpp/count.hpp"
#include "rmw_connext_shared_cpp/ndds_include.hpp"

#include "rmw_connext_cpp/identifier.hpp"
#include "rmw_connext_cpp/connext_static_client_info.hpp"

// Uncomment this to get extra console output about discovery.
// This affects code in this file, but there is a similar variable in:
//   rmw_connext_shared_cpp/shared_functions.cpp
// #define DISCOVERY_DEBUG_LOGGING 1

rmw_ret_t
_publisher_count_matched_subscriptions(DDS::DataWriter * datawriter, size_t * count)
{
  DDS_ReturnCode_t ret;
  DDS_PublicationMatchedStatus s;
  ret = datawriter->get_publication_matched_status(s);
  if (ret != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get publication matched status");
    return RMW_RET_ERROR;
  }

// TODO(karsten1987): replace this block with logging macros
#ifdef DISCOVERY_DEBUG_LOGGING
  using std::to_string;
  using std::stringstream;
  std::stringstream ss;
  // *INDENT-OFF* (prevent uncrustify from making unnecessary indents here)
  ss << "DDS_PublicationMatchedStatus:\n"
     << "  topic name:               " << datawriter->get_topic()->get_name() << "\n"
     << "  current_count:            " << to_string(s.current_count) << "\n"
     << "  current_count_change:     " << to_string(s.current_count_change) << "\n"
     << "  current_count_peak:       " << to_string(s.current_count_peak) << "\n"
     << "  total_count:              " << to_string(s.total_count) << "\n"
     << "  total_count_change:       " << to_string(s.total_count_change) << "\n"
     << "  last_subscription_handle: " << to_string(s.last_subscription_handle) << "\n";
  // *INDENT-ON*
  printf("%s", ss.str().c_str());
#endif

  *count = s.current_count;
  return RMW_RET_OK;
}

rmw_ret_t
_subscription_count_matched_publishers(DDS::DataReader * datareader, size_t * count)
{
  DDS_ReturnCode_t ret;
  DDS_SubscriptionMatchedStatus s;
  ret = datareader->get_subscription_matched_status(s);
  if (ret != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get subscription matched status");
    return RMW_RET_ERROR;
  }

#ifdef DISCOVERY_DEBUG_LOGGING
  using std::to_string;
  using std::stringstream;
  std::stringstream ss;
  // *INDENT-OFF* (prevent uncrustify from making unnecessary indents here)
  ss << "DDS_SubscriptionMatchedStatus:\n"
     << "  topic name:               " << datareader->get_topicdescription()->get_name() << "\n"
     << "  current_count:            " << to_string(s.current_count) << "\n"
     << "  current_count_change:     " << to_string(s.current_count_change) << "\n"
     << "  current_count_peak:       " << to_string(s.current_count_peak) << "\n"
     << "  total_count:              " << to_string(s.total_count) << "\n"
     << "  total_count_change:       " << to_string(s.total_count_change) << "\n"
     << "  last_publication_handle:  " << to_string(s.last_publication_handle) << "\n";
  // *INDENT-ON*
  printf("%s", ss.str().c_str());
#endif

  *count = s.current_count;
  return RMW_RET_OK;
}

extern "C"
{
rmw_ret_t
rmw_service_server_is_available(
  const rmw_node_t * node,
  const rmw_client_t * client,
  bool * is_available)
{
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
  DDS::DataWriter * request_datawriter =
    static_cast<DDS::DataWriter *>(callbacks->get_request_datawriter(requester));
  const char * request_topic_name = request_datawriter->get_topic()->get_name();
  if (!request_topic_name) {
    RMW_SET_ERROR_MSG("could not get request topic name");
    return RMW_RET_ERROR;
  }

  *is_available = false;
  // In the Connext RPC implementation, a server is ready when:
  //   - At least one subscriber is matched to the request publisher.
  //   - At least one publisher is matched to the reponse subscription.
  size_t number_of_request_subscribers = 0;
  rmw_ret_t ret = _publisher_count_matched_subscriptions(
    request_datawriter, &number_of_request_subscribers);
  if (ret != RMW_RET_OK) {
    // error string already set
    return ret;
  }
// TODO(karsten1987): replace this block with logging macros
#ifdef DISCOVERY_DEBUG_LOGGING
  DDSPublisher * request_publisher = request_datawriter->get_publisher();
  DDS_PublisherQos pub_qos;
  request_publisher->get_qos(pub_qos);
  fprintf(stderr, "******** rmw_server_is_available *****\n");
  for (DDS_Long i = 0; i < pub_qos.partition.name.length(); ++i) {
    fprintf(stderr, "publisher address %p\n", static_cast<void *>(request_publisher));
    fprintf(stderr, "request topic name: %s\n", request_topic_name);
    fprintf(stderr, "request partition (should be rq) %s\n", pub_qos.partition.name[i]);
  }

  DDS::DataReader * response_datareader =
    static_cast<DDS::DataReader *>(callbacks->get_reply_datareader(requester));
  const char * response_topic_name = response_datareader->get_topicdescription()->get_name();
  DDSSubscriber * response_sub = response_datareader->get_subscriber();
  DDS_SubscriberQos sub_qos;
  response_sub->get_qos(sub_qos);
  for (DDS_Long i = 0; i < sub_qos.partition.name.length(); ++i) {
    fprintf(stderr, "subscriber address %p\n", static_cast<void *>(response_sub));
    fprintf(stderr, "response topic name: %s\n", response_topic_name);
    fprintf(stderr, "response partition (should be rr) %s\n", sub_qos.partition.name[i]);
  }
  fprintf(stderr, "********\n");
  printf("Checking for service server:\n");
  printf(" - %s: %zu\n",
    request_topic_name,
    number_of_request_subscribers);
#endif
  if (number_of_request_subscribers == 0) {
    // not ready
    return RMW_RET_OK;
  }

  size_t number_of_response_publishers = 0;
  ret = _subscription_count_matched_publishers(
    client_info->response_datareader_, &number_of_response_publishers);
  if (ret != RMW_RET_OK) {
    // error string already set
    return ret;
  }
#ifdef DISCOVERY_DEBUG_LOGGING
  printf(" - %s: %zu\n",
    client_info->response_datareader_->get_topicdescription()->get_name(),
    number_of_response_publishers);
#endif
  if (number_of_response_publishers == 0) {
    fprintf(stderr, "Number of response publishers is 0\n");
    // not ready
    return RMW_RET_OK;
  }

  // all conditions met, there is a service server available
  *is_available = true;
  return RMW_RET_OK;
}
}  // extern "C"
