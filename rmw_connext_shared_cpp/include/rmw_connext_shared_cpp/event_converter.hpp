// Copyright 2015-2017 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_CONNEXT_SHARED_CPP__EVENT_CONVERTER_HPP_
#define RMW_CONNEXT_SHARED_CPP__EVENT_CONVERTER_HPP_

#include "ndds/ndds_cpp.h"
#include "rmw/event.h"

DDS_StatusMask get_mask_from_rmw(const rmw_event_type_t & event_t);

bool is_event_supported(const rmw_event_type_t & event_t);

#endif  // RMW_CONNEXT_SHARED_CPP__EVENT_CONVERTER_HPP_
