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

#ifndef ROSIDL_TYPESUPPORT_CONNEXT_CPP__CONNEXT_STATIC_CDR_STREAM_HPP_
#define ROSIDL_TYPESUPPORT_CONNEXT_CPP__CONNEXT_STATIC_CDR_STREAM_HPP_

#include "rcutils/allocator.h"

extern "C"
{
// TODO(karsten1987): Join rmw_raw_message_t and this ConnextStaticCDRStream struct
// inside rcutils to have only one implementation of it.
typedef struct ConnextStaticCDRStream
{
  char * buffer = NULL;
  unsigned int buffer_length = 0;
  unsigned int buffer_capacity = 0;
  rcutils_allocator_t allocator;
} ConnextStaticCDRStream;
}  // extern "C"

#endif  // ROSIDL_TYPESUPPORT_CONNEXT_CPP__CONNEXT_STATIC_CDR_STREAM_HPP_
