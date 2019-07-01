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

#ifndef TYPES__CONNEXT_STATIC_PUBLISHER_INFO_HPP_
#define TYPES__CONNEXT_STATIC_PUBLISHER_INFO_HPP_

#include "rmw_connext_shared_cpp/shared_functions.hpp"

#include "rosidl_typesupport_connext_cpp/message_type_support.h"

extern "C"
{
struct ConnextStaticPublisherInfo
{
  DDSPublisher * dds_publisher_;
  DDSDataWriter * topic_writer_;
  const message_type_support_callbacks_t * callbacks_;
  rmw_gid_t publisher_gid;
};
}  // extern "C"

#endif  // TYPES__CONNEXT_STATIC_PUBLISHER_INFO_HPP_
