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

#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/types.h"

#include "rosidl_typesupport_connext_cpp/message_type_support.h"

extern "C"
{

rmw_ret_t
rmw_get_serialized_message_size(
  const rosidl_message_bounds_t * message_bounds,
  const rosidl_message_type_support_t * type_support,
  size_t * size)
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

  *size = (size_t) expected_length;

  return RMW_RET_OK;
}

rmw_ret_t
rmw_init_publisher_allocation(
  const rosidl_message_type_support_t * type_support,
  const rosidl_message_bounds_t * message_bounds,
  rmw_publisher_allocation_t * allocation);
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

  rcutils_uint8_array_t cdr_stream = rcutils_get_zero_initialized_uint8_array();
  cdr_stream.allocator = rcutils_get_default_allocator();


  cdr_stream->buffer_length = expected_length;
  if (cdr_stream->buffer_length > (std::numeric_limits<unsigned int>::max)()) {
    fprintf(stderr, "cdr_stream->buffer_length, unexpectedly larger than max unsigned int\n");
    return false;
  }
  if (cdr_stream->buffer_capacity < cdr_stream->buffer_length) {
    cdr_stream->allocator.deallocate(cdr_stream->buffer, cdr_stream->allocator.state);
    cdr_stream->buffer = static_cast<uint8_t *>(cdr_stream->allocator.allocate(cdr_stream->buffer_length, cdr_stream->allocator.state));
  }

}

}  // extern "C"
