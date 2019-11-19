// Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <map>

#include "rmw_connext_shared_cpp/topic_info.hpp"

#include "rmw/error_handling.h"

rmw_ret_t
get_publishers_info_by_topic(
  const char * /* unused_param */,
  const rmw_node_t * /* unused_param */,
  rcutils_allocator_t * /* unused_param */,
  const char * /* unused_param */,
  bool /* unused_param */,
  rmw_topic_info_array_t * /* unused_param */)
{
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
get_subscriptions_info_by_topic(
  const char * /* unused_param */,
  const rmw_node_t * /* unused_param */,
  rcutils_allocator_t * /* unused_param */,
  const char * /* unused_param */,
  bool /* unused_param */,
  rmw_topic_info_array_t * /* unused_param */)
{
  return RMW_RET_UNSUPPORTED;
}
