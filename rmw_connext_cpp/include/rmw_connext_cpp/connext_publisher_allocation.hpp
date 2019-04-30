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

#ifndef RMW_CONNEXT_CPP__CONNEXT_PUBLISHER_ALLOCATION_HPP_
#define RMW_CONNEXT_CPP__CONNEXT_PUBLISHER_ALLOCATION_HPP_

#include "connext_static_serialized_dataSupport.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct connext_publisher_allocation_t
{
  rcutils_uint8_array_t cdr_stream;
  ConnextStaticSerializedData * instance;
} connext_publisher_allocation_t;

#ifdef __cplusplus
}
#endif

#endif  // RMW_CONNEXT_CPP__CONNEXT_PUBLISHER_ALLOCATION_HPP_
