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
rmw_event_t *
rmw_create_publisher_event(
  const rmw_publisher_t * publisher,
  const rmw_event_type_t event_type)
{
  return __rmw_create_publisher_event(rti_connext_identifier, publisher, event_type);
}


rmw_event_t *
rmw_create_subscription_event(
  const rmw_subscription_t * subscription,
  const rmw_event_type_t event_type)
{
  return __rmw_create_subscription_event(rti_connext_identifier, subscription, event_type);
}


/*
 * Take an event from the event handle.
 *
 * \param event subscription object to take from
 * \param taken boolean flag indicating if an event was taken or not
 * \return `RMW_RET_OK` if successful, or
 * \return `RMW_RET_BAD_ALLOC` if memory allocation failed, or
 * \return `RMW_RET_ERROR` if an unexpected error occurs.
 */
rmw_ret_t
rmw_take_event(
  const rmw_event_t * event_handle,
  void * event,
  bool * taken)
{
  return __rmw_take_event(
    rti_connext_identifier,
    event_handle,
    event,
    taken);
}


rmw_ret_t
rmw_destroy_event(rmw_event_t * event)
{
  return __rmw_destroy_event(rti_connext_identifier, event);
}
}  // extern "C"
