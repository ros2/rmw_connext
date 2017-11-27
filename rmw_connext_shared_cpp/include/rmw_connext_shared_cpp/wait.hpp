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

#ifndef RMW_CONNEXT_SHARED_CPP__WAIT_HPP_
#define RMW_CONNEXT_SHARED_CPP__WAIT_HPP_

#include "ndds_include.hpp"

#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/types.h"

#include "rmw_connext_shared_cpp/condition_error.hpp"
#include "rmw_connext_shared_cpp/types.hpp"
#include "rmw_connext_shared_cpp/visibility_control.h"

template<typename SubscriberInfo, typename ServiceInfo, typename ClientInfo>
rmw_ret_t
wait(
  const char * implementation_identifier,
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_wait_set_t * wait_set,
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
      if (!wait_set) {
        RMW_SET_ERROR_MSG("wait set handle is null");
        return;
      }
      RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
        wait set handle,
        wait_set->implementation_identifier, implementation_identifier,
        return )
      ConnextWaitSetInfo * wait_set_info = static_cast<ConnextWaitSetInfo *>(wait_set->data);
      if (!wait_set_info) {
        RMW_SET_ERROR_MSG("WaitSet implementation struct is null");
        return;
      }

      DDSWaitSet * dds_wait_set = static_cast<DDSWaitSet *>(wait_set_info->wait_set);
      if (!dds_wait_set) {
        RMW_SET_ERROR_MSG("DDS wait set handle is null");
        return;
      }

      DDSConditionSeq * attached_conditions =
        static_cast<DDSConditionSeq *>(wait_set_info->attached_conditions);
      if (!attached_conditions) {
        RMW_SET_ERROR_MSG("DDS condition sequence handle is null");
        return;
      }

      DDS_ReturnCode_t retcode;
      retcode = dds_wait_set->get_conditions(*attached_conditions);
      if (retcode != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("Failed to get attached conditions for wait set");
        return;
      }

      for (DDS_Long i = 0; i < attached_conditions->length(); ++i) {
        retcode = dds_wait_set->detach_condition((*attached_conditions)[i]);
        if (retcode != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("Failed to get detach condition from wait set");
        }
      }
    }
    rmw_wait_set_t * wait_set = NULL;
    const char * implementation_identifier = NULL;
  } atexit;

  atexit.wait_set = wait_set;
  atexit.implementation_identifier = implementation_identifier;

  if (!wait_set) {
    RMW_SET_ERROR_MSG("wait set handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    wait set handle,
    wait_set->implementation_identifier, implementation_identifier,
    return RMW_RET_ERROR);

  ConnextWaitSetInfo * wait_set_info = static_cast<ConnextWaitSetInfo *>(wait_set->data);
  if (!wait_set_info) {
    RMW_SET_ERROR_MSG("WaitSet implementation struct is null");
    return RMW_RET_ERROR;
  }

  DDSWaitSet * dds_wait_set = static_cast<DDSWaitSet *>(wait_set_info->wait_set);
  if (!dds_wait_set) {
    RMW_SET_ERROR_MSG("DDS wait set handle is null");
    return RMW_RET_ERROR;
  }

  DDSConditionSeq * active_conditions =
    static_cast<DDSConditionSeq *>(wait_set_info->active_conditions);
  if (!active_conditions) {
    RMW_SET_ERROR_MSG("DDS condition sequence handle is null");
    return RMW_RET_ERROR;
  }

  // add a condition for each subscriber
  if (subscriptions) {
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
        dds_wait_set->attach_condition(read_condition));
      if (rmw_status != RMW_RET_OK) {
        return rmw_status;
      }
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
        dds_wait_set->attach_condition(guard_condition));
      if (rmw_status != RMW_RET_OK) {
        return rmw_status;
      }
    }
  }

  // add a condition for each service
  if (services) {
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
        dds_wait_set->attach_condition(read_condition));
      if (rmw_status != RMW_RET_OK) {
        return rmw_status;
      }
    }
  }

  // add a condition for each client
  if (clients) {
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
        dds_wait_set->attach_condition(read_condition));
      if (rmw_status != RMW_RET_OK) {
        return rmw_status;
      }
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

  DDS_ReturnCode_t status = dds_wait_set->wait(*active_conditions, timeout);

  if (status != DDS_RETCODE_OK && status != DDS_RETCODE_TIMEOUT) {
    RMW_SET_ERROR_MSG("failed to wait on wait set");
    return RMW_RET_ERROR;
  }

  // set subscriber handles to zero for all not triggered conditions
  if (subscriptions) {
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
      DDS_ReturnCode_t retcode = dds_wait_set->detach_condition(read_condition);
      if (retcode != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("Failed to get detach condition from wait set");
        return RMW_RET_ERROR;
      }
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
      DDS_ReturnCode_t retcode = dds_wait_set->detach_condition(condition);
      if (retcode != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("Failed to get detach condition from wait set");
        return RMW_RET_ERROR;
      }
    }
  }

  // set service handles to zero for all not triggered conditions
  if (services) {
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
      DDS_ReturnCode_t retcode = dds_wait_set->detach_condition(read_condition);
      if (retcode != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("Failed to get detach condition from wait set");
        return RMW_RET_ERROR;
      }
    }
  }

  // set client handles to zero for all not triggered conditions
  if (clients) {
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
      DDS_ReturnCode_t retcode = dds_wait_set->detach_condition(read_condition);
      if (retcode != DDS_RETCODE_OK) {
        RMW_SET_ERROR_MSG("Failed to get detach condition from wait set");
        return RMW_RET_ERROR;
      }
    }
  }

  if (status == DDS_RETCODE_TIMEOUT) {
    return RMW_RET_TIMEOUT;
  }
  return RMW_RET_OK;
}

#endif  // RMW_CONNEXT_SHARED_CPP__WAIT_HPP_
