// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_CONNEXT_CPP__SUBSCRIPTION_HPP_
#define RMW_CONNEXT_CPP__SUBSCRIPTION_HPP_

#include "rmw/types.h"

#include "rmw_connext_shared_cpp/types.hpp"

namespace rmw_connext_cpp
{

rmw_subscription_t *
create_subscription(
  const ConnextParticipantInfo * participant_info,
  const rosidl_message_type_support_t * type_supports,
  const char * topic_name,
  const rmw_qos_profile_t * qos_profile,
  const rmw_subscription_options_t * subscription_options);

rmw_ret_t
destroy_subscription(
  const ConnextParticipantInfo * participant_info,
  rmw_subscription_t * subscription);

}  // namespace rmw_connext_cpp

#endif  // RMW_CONNEXT_CPP__SUBSCRIPTION_HPP_
