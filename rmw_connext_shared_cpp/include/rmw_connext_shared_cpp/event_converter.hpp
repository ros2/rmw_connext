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

#include <unordered_map>

#include "ndds/ndds_cpp.h"

#include "rmw/types.h"


inline DDS_StatusMask get_mask_from_rmw(const rmw_event_type_t & event_t)
{
  static const std::unordered_map<rmw_event_type_t, DDS_StatusMask> mask_map{
    {RMW_EVENT_SAMPLE_REJECTED, DDS_SAMPLE_REJECTED_STATUS},
    {RMW_EVENT_LIVELINESS_CHANGED, DDS_LIVELINESS_CHANGED_STATUS},
    {RMW_EVENT_REQUESTED_DEADLINE_MISSED, DDS_REQUESTED_DEADLINE_MISSED_STATUS},
    {RMW_EVENT_REQUESTED_INCOMPATIBLE_QOS, DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS},
    {RMW_EVENT_DATA_AVAILABLE, DDS_DATA_AVAILABLE_STATUS},
    {RMW_EVENT_SAMPLE_LOST, DDS_SAMPLE_LOST_STATUS},
    {RMW_EVENT_SUBSCRIPTION_MATCHED, DDS_SUBSCRIPTION_MATCHED_STATUS},
    {RMW_EVENT_LIVELINESS_LOST, DDS_LIVELINESS_LOST_STATUS},
    {RMW_EVENT_OFFERED_DEADLINE_MISSED, DDS_OFFERED_DEADLINE_MISSED_STATUS},
    {RMW_EVENT_OFFERED_INCOMPATIBLE_QOS, DDS_OFFERED_INCOMPATIBLE_QOS_STATUS},
    {RMW_EVENT_PUBLICATION_MATCHED, DDS_PUBLICATION_MATCHED_STATUS}
  };
  return mask_map.at(event_t);
}

#endif  // RMW_CONNEXT_SHARED_CPP__EVENT_CONVERTER_HPP_
