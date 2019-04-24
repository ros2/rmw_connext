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

#include <limits>
#include "rmw_connext_shared_cpp/qos.hpp"

namespace
{

template<typename DDSEntityQos>
bool
set_entity_qos_from_profile_generic(
  const rmw_qos_profile_t & qos_profile,
  DDSEntityQos & entity_qos)
{
  // Read properties from the rmw profile
  switch (qos_profile.history) {
    case RMW_QOS_POLICY_HISTORY_KEEP_LAST:
      entity_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_KEEP_ALL:
      entity_qos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS history policy");
      return false;
  }

  switch (qos_profile.reliability) {
    case RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT:
      entity_qos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_RELIABLE:
      entity_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS reliability policy");
      return false;
  }

  switch (qos_profile.durability) {
    case RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL:
      entity_qos.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_VOLATILE:
      entity_qos.durability.kind = DDS::VOLATILE_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS durability policy");
      return false;
  }

  if (qos_profile.depth != RMW_QOS_POLICY_DEPTH_SYSTEM_DEFAULT) {
    entity_qos.history.depth = static_cast<DDS::Long>(qos_profile.depth);
  }

  // DDS_DeadlineQosPolicy has default value of DDS_DURATION_INFINITE, don't touch it for 0
  if (qos_profile.deadline.sec != 0 || qos_profile.deadline.nsec != 0) {
    entity_qos.deadline.period.sec = qos_profile.deadline.sec;
    entity_qos.deadline.period.nanosec = qos_profile.deadline.nsec;
  }

  switch (qos_profile.liveliness) {
    case RMW_QOS_POLICY_LIVELINESS_AUTOMATIC:
      entity_qos.liveliness.kind = DDS::AUTOMATIC_LIVELINESS_QOS;
      break;
    case RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_NODE:
      entity_qos.liveliness.kind = DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
      break;
    case RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC:
      entity_qos.liveliness.kind = DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS;
      break;
    case RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS liveliness policy");
      return false;
  }
  if (qos_profile.liveliness_lease_duration.sec != 0 ||
    qos_profile.liveliness_lease_duration.nsec != 0)
  {
    entity_qos.liveliness.lease_duration.sec = qos_profile.liveliness_lease_duration.sec;
    entity_qos.liveliness.lease_duration.nanosec = qos_profile.liveliness_lease_duration.nsec;
  }

  // ensure the history depth is at least the requested queue size
  assert(entity_qos.history.depth >= 0);
  if (
    entity_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    static_cast<size_t>(entity_qos.history.depth) < qos_profile.depth)
  {
    if (qos_profile.depth > (std::numeric_limits<DDS::Long>::max)()) {
      RMW_SET_ERROR_MSG(
        "failed to set history depth since the requested queue size exceeds the DDS type");
      return false;
    }
    entity_qos.history.depth = static_cast<DDS::Long>(qos_profile.depth);
  }
  return true;
}

bool
set_entity_qos_from_profile(
  const rmw_qos_profile_t & qos_profile,
  DDS::DataReaderQos & entity_qos)
{
  // Set any QoS settings that are specific to DataReader, then call the shared version
  return set_entity_qos_from_profile_generic(qos_profile, entity_qos);
}

bool
set_entity_qos_from_profile(
  const rmw_qos_profile_t & qos_profile,
  DDS::DataWriterQos & entity_qos)
{
  // Set any QoS settings that are specific to DataWriter, then call the shared version
  if (qos_profile.lifespan.sec != 0 || qos_profile.lifespan.nsec != 0) {
    entity_qos.lifespan.duration.sec = qos_profile.lifespan.sec;
    entity_qos.lifespan.duration.nanosec = qos_profile.lifespan.nsec;
  }
  return set_entity_qos_from_profile_generic(qos_profile, entity_qos);
}

}  // anonymous namespace

bool
get_datareader_qos(
  DDS::DomainParticipant * participant,
  const rmw_qos_profile_t & qos_profile,
  DDS::DataReaderQos & datareader_qos)
{
  DDS::ReturnCode_t status = participant->get_default_datareader_qos(datareader_qos);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default datareader qos");
    return false;
  }

  status = DDS::PropertyQosPolicyHelper::add_property(
    datareader_qos.property,
    "dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size",
    "4096",
    DDS::BOOLEAN_FALSE);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to add qos property");
    return false;
  }

  status = DDS::PropertyQosPolicyHelper::add_property(
    datareader_qos.property,
    "reader_resource_limits.dynamically_allocate_fragmented_samples",
    "1",
    DDS::BOOLEAN_FALSE);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to add qos property");
    return false;
  }

  if (!set_entity_qos_from_profile(qos_profile, datareader_qos)) {
    return false;
  }

  return true;
}

bool
get_datawriter_qos(
  DDS::DomainParticipant * participant,
  const rmw_qos_profile_t & qos_profile,
  DDS::DataWriterQos & datawriter_qos)
{
  DDS::ReturnCode_t status = participant->get_default_datawriter_qos(datawriter_qos);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default datawriter qos");
    return false;
  }

  status = DDS::PropertyQosPolicyHelper::add_property(
    datawriter_qos.property,
    "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
    "4096",
    DDS::BOOLEAN_FALSE);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to add qos property");
    return false;
  }

  if (!set_entity_qos_from_profile(qos_profile, datawriter_qos)) {
    return false;
  }

  // TODO(wjwwood): conditionally use the async publish mode using a heuristic:
  //  https://github.com/ros2/rmw_connext/issues/190
  datawriter_qos.publish_mode.kind = DDS::ASYNCHRONOUS_PUBLISH_MODE_QOS;

  return true;
}
