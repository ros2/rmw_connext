// Copyright 2017 Open Source Robotics Foundation, Inc.
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

#ifndef ROSIDL_TYPESUPPORT_CONNEXT_CPP__CONNEXT_STATIC_MESSAGE_HANDLE_HPP_
#define ROSIDL_TYPESUPPORT_CONNEXT_CPP__CONNEXT_STATIC_MESSAGE_HANDLE_HPP_

extern "C"
{
struct ConnextStaticMessageHandle
{
  const void * untyped_ros_message = nullptr;
  void * untyped_dds_message = nullptr;
  const char * raw_message = nullptr;  // making this void for not including rmw dep
  const size_t * raw_message_length;  // making this void for not including rmw dep
};
}  // extern "C"

#endif  // ROSIDL_TYPESUPPORT_CONNEXT_CPP__CONNEXT_STATIC_MESSAGE_HANDLE_HPP_
