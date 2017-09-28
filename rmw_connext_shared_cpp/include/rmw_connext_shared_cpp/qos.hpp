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

#ifndef RMW_CONNEXT_SHARED_CPP__QOS_HPP_
#define RMW_CONNEXT_SHARED_CPP__QOS_HPP_

#include <cassert>
#include <limits>

#include "ndds_include.hpp"

#include "rmw/error_handling.h"
#include "rmw/types.h"

#include "rmw_connext_shared_cpp/visibility_control.h"

RMW_CONNEXT_SHARED_CPP_PUBLIC
bool
get_datareader_qos(
  DDSDomainParticipant * participant,
  const rmw_qos_profile_t & qos_profile,
  DDS_DataReaderQos & datareader_qos);

RMW_CONNEXT_SHARED_CPP_PUBLIC
bool
get_datawriter_qos(
  DDSDomainParticipant * participant,
  const rmw_qos_profile_t & qos_profile,
  DDS_DataWriterQos & datawriter_qos);

template<typename DDSEntityQos>
bool
set_entity_qos_from_profile(
  const rmw_qos_profile_t & qos_profile,
  DDSEntityQos & entity_qos)
{
  // Read properties from the rmw profile
  switch (qos_profile.history) {
    case RMW_QOS_POLICY_HISTORY_KEEP_LAST:
      entity_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_KEEP_ALL:
      entity_qos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS history policy");
      return false;
  }

  switch (qos_profile.reliability) {
    case RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT:
      entity_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_RELIABLE:
      entity_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS reliability policy");
      return false;
  }

  switch (qos_profile.durability) {
    case RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL:
      entity_qos.durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_VOLATILE:
      entity_qos.durability.kind = DDS_VOLATILE_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS durability policy");
      return false;
  }

  if (qos_profile.depth != RMW_QOS_POLICY_DEPTH_SYSTEM_DEFAULT) {
    entity_qos.history.depth = static_cast<DDS_Long>(qos_profile.depth);
  }

  // ensure the history depth is at least the requested queue size
  assert(entity_qos.history.depth >= 0);
  if (
    entity_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    static_cast<size_t>(entity_qos.history.depth) < qos_profile.depth)
  {
    if (qos_profile.depth > (std::numeric_limits<DDS_Long>::max)()) {
      RMW_SET_ERROR_MSG(
        "failed to set history depth since the requested queue size exceeds the DDS type");
      return false;
    }
    entity_qos.history.depth = static_cast<DDS_Long>(qos_profile.depth);
  }
  return true;
}

#endif  // RMW_CONNEXT_SHARED_CPP__QOS_HPP_
