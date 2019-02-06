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

#include <string>
#include <limits>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rmw_connext_cpp/connext_publisher_allocation.hpp"

#include "type_support_common.hpp"

extern "C"
{

rmw_ret_t
rmw_get_serialized_message_size(
  const rosidl_message_bounds_t * message_bounds,
  const rosidl_message_type_support_t * type_supports,
  size_t * size)
{

  //TODO uncomment when message bound gets implemented
  // if (!message_bounds) {
  //   RMW_SET_ERROR_MSG("message_bounds is null");
  //   return RMW_RET_ERROR;
  // }

  if (!type_supports) {
    RMW_SET_ERROR_MSG("type_support is null");
    return RMW_RET_ERROR;
  }

  RMW_CONNEXT_EXTRACT_MESSAGE_TYPESUPPORT(type_supports, type_support, NULL)

  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  unsigned int expected_length = 0;
  if (!callbacks->get_serialized_length(&expected_length)) {
    RMW_SET_ERROR_MSG("failed to get serialized length");
    return RMW_RET_ERROR;
  }

  *size = static_cast<size_t>(expected_length);

  return RMW_RET_OK;
}

rmw_ret_t
rmw_init_publisher_allocation(
  const rosidl_message_type_support_t * type_support,
  const rosidl_message_bounds_t * message_bounds,
  rmw_publisher_allocation_t * allocation)
{

  //TODO uncomment when message bound gets implemented
  // if (!message_bounds) {
  //   RMW_SET_ERROR_MSG("message_bounds is null");
  //   return RMW_RET_ERROR;
  // }

  if (!type_support) {
    RMW_SET_ERROR_MSG("type_support is null");
    return RMW_RET_ERROR;
  }

  size_t expected_length = 0;
  if(rmw_get_serialized_message_size(message_bounds, type_support, &expected_length) != RMW_RET_OK)
  {
    RMW_SET_ERROR_MSG("failed to call rmw_get_serialized_message_size()");
    return RMW_RET_ERROR;
  }

  if (expected_length > std::numeric_limits<unsigned int>::max() ) {
    RMW_SET_ERROR_MSG("cdr_stream->buffer_length, unexpectedly larger than max unsigned int");
    return RMW_RET_ERROR;
  }

  rcutils_uint8_array_t cdr_stream = rcutils_get_zero_initialized_uint8_array();
  cdr_stream.allocator = rcutils_get_default_allocator();
  cdr_stream.buffer_capacity = expected_length;
  cdr_stream.allocator.deallocate(cdr_stream.buffer, cdr_stream.allocator.state);
  cdr_stream.buffer = static_cast<uint8_t *>(cdr_stream.allocator.allocate(
  cdr_stream.buffer_capacity, cdr_stream.allocator.state));

  connext_publisher_allocation_t * __connext_alloc = nullptr;
   __connext_alloc = static_cast<connext_publisher_allocation_t *>(
     rmw_allocate(sizeof(connext_publisher_allocation_t) ));
  if (!__connext_alloc) {
    RMW_SET_ERROR_MSG("failed to allocate memory for connext_alloc");
    return RMW_RET_ERROR;
  }

  __connext_alloc->cdr_stream =  cdr_stream;
  allocation->data = __connext_alloc;

  return RMW_RET_OK;
}

rmw_ret_t
rmw_fini_publisher_allocation(
  rmw_publisher_allocation_t * allocation)
{

  connext_publisher_allocation_t * connext_alloc = static_cast<connext_publisher_allocation_t * >(allocation->data);
  rcutils_uint8_array_t cdr_stream = connext_alloc->cdr_stream;
  cdr_stream.allocator.deallocate(cdr_stream.buffer, cdr_stream.allocator.state);
  rmw_free(connext_alloc);

  return RMW_RET_OK;
}

}  // extern "C"
