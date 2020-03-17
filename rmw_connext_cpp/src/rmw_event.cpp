// Copyright 2014-2019 Open Source Robotics Foundation, Inc.
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

#include "rmw/rmw.h"

#include "rmw_connext_shared_cpp/event.hpp"

#include "rmw_connext_cpp/identifier.hpp"

extern "C"
{

/// Initialize a rmw publisher event.
/**
 * \param[in|out] rmw_event to initialize
 * \param publisher to initialize with
 * \param event_type for the event to handle
 * \return `RMW_RET_OK` if successful, or
 * \return `RMW_RET_INVALID_ARGUMENT` if invalid argument, or
 * \return `RMW_RET_UNSUPPORTED` if event_type is not supported, or
 * \return `RMW_RET_ERROR` if an unexpected error occurs.
 */
rmw_ret_t
rmw_publisher_event_init(
  rmw_event_t * rmw_event,
  const rmw_publisher_t * publisher,
  rmw_event_type_t event_type)
{
  return __rmw_init_event(
    rti_connext_identifier,
    rmw_event,
    publisher->implementation_identifier,
    publisher->data,
    event_type);
}

/// Initialize a rmw subscription event.
/**
 * \param[in|out] rmw_event to initialize
 * \param subscription to initialize with
 * \param event_type for the event to handle
 * \return `RMW_RET_OK` if successful, or
 * \return `RMW_RET_INVALID_ARGUMENT` if invalid argument, or
 * \return `RMW_RET_UNSUPPORTED` if event_type is not supported, or
 * \return `RMW_RET_ERROR` if an unexpected error occurs.
 */
rmw_ret_t
rmw_subscription_event_init(
  rmw_event_t * rmw_event,
  const rmw_subscription_t * subscription,
  rmw_event_type_t event_type)
{
  return __rmw_init_event(
    rti_connext_identifier,
    rmw_event,
    subscription->implementation_identifier,
    subscription->data,
    event_type);
}

/// Take an event from the event handle.
/**
 * \param event_handle event object to take from
 * \param event_info event info object to write taken data into
 * \param taken boolean flag indicating if an event was taken or not
 * \return `RMW_RET_OK` if successful, or
 * \return `RMW_RET_BAD_ALLOC` if memory allocation failed, or
 * \return `RMW_RET_ERROR` if an unexpected error occurs.
 */
rmw_ret_t
rmw_take_event(
  const rmw_event_t * event_handle,
  void * event_info,
  bool * taken)
{
  return __rmw_take_event(
    rti_connext_identifier,
    event_handle,
    event_info,
    taken);
}

}  // extern "C"
