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

#ifndef RMW_CONNEXT_SHARED_CPP__TOPIC_ENDPOINT_INFO_HPP_
#define RMW_CONNEXT_SHARED_CPP__TOPIC_ENDPOINT_INFO_HPP_

#include "rmw/topic_endpoint_info_array.h"
#include "rmw_connext_shared_cpp/visibility_control.h"

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_ret_t
get_publishers_info_by_topic(
  const char * identifier,
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * topic_name,
  bool no_mangle,
  rmw_topic_endpoint_info_array_t * publishers_info);

RMW_CONNEXT_SHARED_CPP_PUBLIC
rmw_ret_t
get_subscriptions_info_by_topic(
  const char * identifier,
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * topic_name,
  bool no_mangle,
  rmw_topic_endpoint_info_array_t * subscriptions_info);

#endif  // RMW_CONNEXT_SHARED_CPP__TOPIC_ENDPOINT_INFO_HPP_
