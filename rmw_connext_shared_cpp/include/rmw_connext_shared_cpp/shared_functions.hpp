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

#ifndef RMW_CONNEXT_SHARED_CPP__SHARED_FUNCTIONS_HPP_
#define RMW_CONNEXT_SHARED_CPP__SHARED_FUNCTIONS_HPP_

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

#include "rmw/allocators.h"
#include "rmw/rmw.h"
#include "rmw/types.h"

#include "rmw/impl/cpp/macros.hpp"

#include "rmw_connext_shared_cpp/visibility_control.h"
#include "rmw_connext_shared_cpp/types.hpp"

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_ret_t init();

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_ret_t check_attach_condition_error(DDS::ReturnCode_t retcode);

RMW_CONNEXT_SHARED_CPP_PUBLIC
bool
get_datareader_qos(DDSDomainParticipant * participant, const rmw_qos_profile_t & qos_profile,
  DDS_DataReaderQos & datareader_qos);

RMW_CONNEXT_SHARED_CPP_PUBLIC
bool
get_datawriter_qos(DDSDomainParticipant * participant, const rmw_qos_profile_t & qos_profile,
  DDS_DataWriterQos & datawriter_qos);

template<typename DDSEntityQos>
bool set_entity_qos_from_profile(const rmw_qos_profile_t & qos_profile,
  DDSEntityQos & entity_qos)
{
  // Read properties from the rmw profile
  switch (qos_profile.history) {
    case RMW_QOS_POLICY_HISTORY_KEEP_LAST:
      entity_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_KEEP_ALL:
      entity_qos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS history policy");
      return false;
  }

  switch (qos_profile.reliability) {
    case RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT:
      entity_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_RELIABLE:
      entity_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS reliability policy");
      return false;
  }

  switch (qos_profile.durability) {
    case RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL:
      entity_qos.durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_VOLATILE:
      entity_qos.durability.kind = DDS_VOLATILE_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS durability policy");
      return false;
  }

  if (qos_profile.depth != RMW_QOS_POLICY_DEPTH_SYSTEM_DEFAULT) {
    entity_qos.history.depth = static_cast<DDS_Long>(qos_profile.depth);
  }

  // ensure the history depth is at least the requested queue size
  assert(entity_qos.history.depth >= 0);
  if (
    entity_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    static_cast<size_t>(entity_qos.history.depth) < qos_profile.depth
  )
  {
    if (qos_profile.depth > (std::numeric_limits<DDS_Long>::max)()) {
      RMW_SET_ERROR_MSG(
        "failed to set history depth since the requested queue size exceeds the DDS type");
      return false;
    }
    entity_qos.history.depth = static_cast<DDS_Long>(qos_profile.depth);
  }
  return true;
}

RMW_CONNEXT_SHARED_CPP_PUBLIC
void
destroy_topic_names_and_types(
  rmw_topic_names_and_types_t * topic_names_and_types);

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_node_t *
create_node(
  const char * implementation_identifier,
  const char * name,
  const char * namespace_,
  size_t domain_id);

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_ret_t
destroy_node(const char * implementation_identifier, rmw_node_t * node);

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_guard_condition_t *
create_guard_condition(const char * implementation_identifier);

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_ret_t
destroy_guard_condition(const char * implementation_identifier,
  rmw_guard_condition_t * guard_condition);

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_waitset_t *
create_waitset(const char * implementation_identifier, size_t max_conditions);

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_ret_t
destroy_waitset(const char * implementation_identifier,
  rmw_waitset_t * waitset);

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_ret_t
trigger_guard_condition(const char * implementation_identifier,
  const rmw_guard_condition_t * guard_condition_handle);

template<typename SubscriberInfo, typename ServiceInfo, typename ClientInfo>
rmw_ret_t
wait(const char * implementation_identifier,
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_waitset_t * waitset,
  const rmw_time_t * wait_timeout)
{
  // To ensure that we properly clean up the wait set, we declare an
  // object whose destructor will detach what we attached (this was previously
  // being done inside the destructor of the wait set.
  struct atexit_t
  {
    ~atexit_t()
    {
      // Manually detach conditions and clear sequences, to ensure a clean wait set for next time.
      if (!waitset) {
        RMW_SET_ERROR_MSG("waitset handle is null");
        return;
      }
      RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
        waitset handle,
        waitset->implementation_identifier, implementation_identifier,
        return )
      ConnextWaitSetInfo * waitset_info = static_cast<ConnextWaitSetInfo *>(waitset->data);
      if (!waitset_info) {
        RMW_SET_ERROR_MSG("WaitSet implementation struct is null");
        return;
      }

      DDSWaitSet * dds_waitset = static_cast<DDSWaitSet *>(waitset_info->waitset);
      if (!dds_waitset) {
        RMW_SET_ERROR_MSG("DDS waitset handle is null");
        return;
      }

      DDSConditionSeq * attached_conditions =
        static_cast<DDSConditionSeq *>(waitset_info->attached_conditions);
      if (!attached_conditions) {
        RMW_SET_ERROR_MSG("DDS condition sequence handle is null");
        return;
      }

      DDS_ReturnCode_t retcode;
      retcode = dds_waitset->get_conditions(*attached_conditions);
      if (retcode != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("Failed to get attached conditions for waitset");
        return;
      }

      for (DDS_Long i = 0; i < attached_conditions->length(); ++i) {
        retcode = dds_waitset->detach_condition((*attached_conditions)[i]);
        if (retcode != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("Failed to get detach condition from waitset");
        }
      }
    }
    rmw_waitset_t * waitset = NULL;
    const char * implementation_identifier = NULL;
  } atexit;

  atexit.waitset = waitset;
  atexit.implementation_identifier = implementation_identifier;

  if (!waitset) {
    RMW_SET_ERROR_MSG("waitset handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    waitset,
    waitset->implementation_identifier, implementation_identifier,
    return RMW_RET_ERROR);

  ConnextWaitSetInfo * waitset_info = static_cast<ConnextWaitSetInfo *>(waitset->data);
  if (!waitset_info) {
    RMW_SET_ERROR_MSG("WaitSet implementation struct is null");
    return RMW_RET_ERROR;
  }

  DDSWaitSet * dds_waitset = static_cast<DDSWaitSet *>(waitset_info->waitset);
  if (!dds_waitset) {
    RMW_SET_ERROR_MSG("DDS waitset handle is null");
    return RMW_RET_ERROR;
  }

  DDSConditionSeq * active_conditions =
    static_cast<DDSConditionSeq *>(waitset_info->active_conditions);
  if (!active_conditions) {
    RMW_SET_ERROR_MSG("DDS condition sequence handle is null");
    return RMW_RET_ERROR;
  }

  // add a condition for each subscriber
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
    SubscriberInfo * subscriber_info =
      static_cast<SubscriberInfo *>(subscriptions->subscribers[i]);
    if (!subscriber_info) {
      RMW_SET_ERROR_MSG("subscriber info handle is null");
      return RMW_RET_ERROR;
    }
    DDSReadCondition * read_condition = subscriber_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }
    rmw_ret_t rmw_status = check_attach_condition_error(
      dds_waitset->attach_condition(read_condition));
    if (rmw_status != RMW_RET_OK) {
      return rmw_status;
    }
  }

  // add a condition for each guard condition
  if (guard_conditions) {
    for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
      DDSGuardCondition * guard_condition =
        static_cast<DDSGuardCondition *>(guard_conditions->guard_conditions[i]);
      if (!guard_condition) {
        RMW_SET_ERROR_MSG("guard condition handle is null");
        return RMW_RET_ERROR;
      }
      rmw_ret_t rmw_status = check_attach_condition_error(
        dds_waitset->attach_condition(guard_condition));
      if (rmw_status != RMW_RET_OK) {
        return rmw_status;
      }
    }
  }

  // add a condition for each service
  for (size_t i = 0; i < services->service_count; ++i) {
    ServiceInfo * service_info =
      static_cast<ServiceInfo *>(services->services[i]);

    if (!service_info) {
      RMW_SET_ERROR_MSG("service info handle is null");
      return RMW_RET_ERROR;
    }

    DDSReadCondition * read_condition = service_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }
    rmw_ret_t rmw_status = check_attach_condition_error(
      dds_waitset->attach_condition(read_condition));
    if (rmw_status != RMW_RET_OK) {
      return rmw_status;
    }
  }

  // add a condition for each client
  for (size_t i = 0; i < clients->client_count; ++i) {
    ClientInfo * client_info =
      static_cast<ClientInfo *>(clients->clients[i]);
    if (!client_info) {
      RMW_SET_ERROR_MSG("client info handle is null");
      return RMW_RET_ERROR;
    }

    DDSDataReader * response_datareader = client_info->response_datareader_;
    if (!response_datareader) {
      RMW_SET_ERROR_MSG("response datareader handle is null");
      return RMW_RET_ERROR;
    }

    // MIGHT BE IMPORTANT !!!
    DDSReadCondition * read_condition = client_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }
    rmw_ret_t rmw_status = check_attach_condition_error(
      dds_waitset->attach_condition(read_condition));
    if (rmw_status != RMW_RET_OK) {
      return rmw_status;
    }
  }

  // invoke wait until one of the conditions triggers
  DDS_Duration_t timeout;
  if (!wait_timeout) {
    timeout = DDS_DURATION_INFINITE;
  } else {
    timeout.sec = static_cast<DDS_Long>(wait_timeout->sec);
    timeout.nanosec = static_cast<DDS_Long>(wait_timeout->nsec);
  }

  DDS_ReturnCode_t status = dds_waitset->wait(*active_conditions, timeout);

  if (status != DDS_RETCODE_OK && status != DDS_RETCODE_TIMEOUT) {
    RMW_SET_ERROR_MSG("failed to wait on waitset");
    return RMW_RET_ERROR;
  }

  // set subscriber handles to zero for all not triggered conditions
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
    SubscriberInfo * subscriber_info =
      static_cast<SubscriberInfo *>(subscriptions->subscribers[i]);
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
    for (; j < active_conditions->length(); ++j) {
      if ((*active_conditions)[j] == read_condition) {
        break;
      }
    }
    // if subscriber condition is not found in the active set
    // reset the subscriber handle
    if (!(j < active_conditions->length())) {
      subscriptions->subscribers[i] = 0;
    }
    DDS_ReturnCode_t retcode = dds_waitset->detach_condition(read_condition);
    if (retcode != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("Failed to get detach condition from waitset");
    }
  }

  // set guard condition handles to zero for all not triggered conditions
  if (guard_conditions) {
    for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
      DDSCondition * condition =
        static_cast<DDSCondition *>(guard_conditions->guard_conditions[i]);
      if (!condition) {
        RMW_SET_ERROR_MSG("condition handle is null");
        return RMW_RET_ERROR;
      }

      // search for guard condition in active set
      DDS_Long j = 0;
      for (; j < active_conditions->length(); ++j) {
        if ((*active_conditions)[j] == condition) {
          DDSGuardCondition * guard = static_cast<DDSGuardCondition *>(condition);
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
      if (!(j < active_conditions->length())) {
        guard_conditions->guard_conditions[i] = 0;
      }
      DDS_ReturnCode_t retcode = dds_waitset->detach_condition(condition);
      if (retcode != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("Failed to get detach condition from waitset");
      }
    }
  }

  // set service handles to zero for all not triggered conditions
  for (size_t i = 0; i < services->service_count; ++i) {
    ServiceInfo * service_info =
      static_cast<ServiceInfo *>(services->services[i]);
    if (!service_info) {
      RMW_SET_ERROR_MSG("service info handle is null");
      return RMW_RET_ERROR;
    }
    DDSReadCondition * read_condition = service_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }

    // search for service condition in active set
    DDS_Long j = 0;
    for (; j < active_conditions->length(); ++j) {
      if ((*active_conditions)[j] == read_condition) {
        break;
      }
    }
    // if service condition is not found in the active set
    // reset the subscriber handle
    if (!(j < active_conditions->length())) {
      services->services[i] = 0;
    }
    DDS_ReturnCode_t retcode = dds_waitset->detach_condition(read_condition);
    if (retcode != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("Failed to get detach condition from waitset");
    }
  }

  // set client handles to zero for all not triggered conditions
  for (size_t i = 0; i < clients->client_count; ++i) {
    ClientInfo * client_info =
      static_cast<ClientInfo *>(clients->clients[i]);
    if (!client_info) {
      RMW_SET_ERROR_MSG("client info handle is null");
      return RMW_RET_ERROR;
    }
    DDSReadCondition * read_condition = client_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }

    // search for service condition in active set
    DDS_Long j = 0;
    for (; j < active_conditions->length(); ++j) {
      if ((*active_conditions)[j] == read_condition) {
        break;
      }
    }
    // if client condition is not found in the active set
    // reset the subscriber handle
    if (!(j < active_conditions->length())) {
      clients->clients[i] = 0;
    }
    DDS_ReturnCode_t retcode = dds_waitset->detach_condition(read_condition);
    if (retcode != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("Failed to get detach condition from waitset");
    }
  }

  if (status == DDS_RETCODE_TIMEOUT) {
    return RMW_RET_TIMEOUT;
  }
  return RMW_RET_OK;
}

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_ret_t
get_topic_names_and_types(const char * implementation_identifier,
  const rmw_node_t * node,
  rmw_topic_names_and_types_t * topic_names_and_types,
  const char * const ros_topic_prefix,
  const char * const ros_service_requester_prefix,
  const char * const ros_service_response_prefix);

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_ret_t
get_node_names(const char * implementation_identifier,
  const rmw_node_t * node,
  rcutils_string_array_t * node_names);

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_ret_t
count_publishers(const char * implementation_identifier,
  const rmw_node_t * node,
  const char * topic_name,
  const char * const ros_topics_prefix,
  const char * const ros_service_requester_prefix,
  const char * const ros_service_response_prefix,
  size_t * count);

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_ret_t
count_subscribers(const char * implementation_identifier,
  const rmw_node_t * node,
  const char * topic_name,
  const char * const ros_topics_prefix,
  const char * const ros_service_requester_prefix,
  const char * const ros_service_response_prefix,
  size_t * count);

RMW_CONNEXT_SHARED_CPP_PUBLIC
const rmw_guard_condition_t *
node_get_graph_guard_condition(const rmw_node_t * node);

#endif  // RMW_CONNEXT_SHARED_CPP__SHARED_FUNCTIONS_HPP_
