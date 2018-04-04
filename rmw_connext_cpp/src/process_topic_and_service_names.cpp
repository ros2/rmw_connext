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

#include "rcutils/format_string.h"
#include "rcutils/types.h"
#include "rcutils/split.h"

#include "rmw/rmw.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"

#include "rmw_connext_shared_cpp/namespace_prefix.hpp"
#include "rmw_connext_shared_cpp/ndds_include.hpp"

bool
_process_topic_name(
  const char * topic_name,
  bool avoid_ros_namespace_conventions,
  char ** topic_str)
{
  bool success = true;
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (!avoid_ros_namespace_conventions) {
    char * concat_str =
      rcutils_format_string(allocator, "%s%s", ros_topic_prefix, topic_name);
    if (!concat_str) {
      RMW_SET_ERROR_MSG("could not allocate memory for topic string")
      success = false;
      goto end;
    }
    *topic_str = DDS_String_dup(concat_str);
    allocator.deallocate(concat_str, allocator.state);
  } else {
    *topic_str = DDS_String_dup(topic_name);
  }

end:
  return success;
}

bool
_process_service_name(
  const char * service_name,
  bool avoid_ros_namespace_conventions,
  char ** service_str,
  char ** request_partition_str,
  char ** response_partition_str)
{
  bool success = true;
  rcutils_string_array_t name_tokens = rcutils_get_zero_initialized_string_array();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (rcutils_split_last(service_name, '/', allocator, &name_tokens) != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
    success = false;
    goto end;
  }
  if (name_tokens.size == 1) {
    if (!avoid_ros_namespace_conventions) {
      *request_partition_str = DDS_String_dup(ros_service_requester_prefix);
      *response_partition_str = DDS_String_dup(ros_service_response_prefix);
    }
    *service_str = DDS_String_dup(name_tokens.data[0]);
  } else if (name_tokens.size == 2) {
    if (avoid_ros_namespace_conventions) {
      // no ros_service_*_prefix, so store the user's namespace directly
      *request_partition_str = DDS_String_dup(name_tokens.data[0]);
      *response_partition_str = DDS_String_dup(name_tokens.data[0]);
    } else {
      // concat the ros_service_*_prefix with the user's namespace
      char * request_concat_str = rcutils_format_string(
        allocator,
        "%s/%s", ros_service_requester_prefix, name_tokens.data[0]);
      if (!request_concat_str) {
        RMW_SET_ERROR_MSG("could not allocate memory for partition string")
        success = false;
        goto end;
      }
      char * response_concat_str = rcutils_format_string(
        allocator,
        "%s/%s", ros_service_response_prefix, name_tokens.data[0]);
      if (!response_concat_str) {
        allocator.deallocate(request_concat_str, allocator.state);
        RMW_SET_ERROR_MSG("could not allocate memory for partition string")
        success = false;
        goto end;
      }
      *request_partition_str = DDS_String_dup(request_concat_str);
      *response_partition_str = DDS_String_dup(response_concat_str);
      allocator.deallocate(request_concat_str, allocator.state);
      allocator.deallocate(response_concat_str, allocator.state);
    }
    *service_str = DDS_String_dup(name_tokens.data[1]);
  } else {
    RMW_SET_ERROR_MSG("Illformated service name")
  }

end:
  // free that memory
  if (rcutils_string_array_fini(&name_tokens) != RCUTILS_RET_OK) {
    fprintf(stderr, "Failed to destroy the token string array\n");
  }

  return success;
}
