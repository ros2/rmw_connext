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

#include "rmw_connext_shared_cpp/qos.hpp"

bool
get_datareader_qos(
  DDSDomainParticipant * participant,
  const rmw_qos_profile_t & qos_profile,
  DDS_DataReaderQos & datareader_qos)
{
  DDS_ReturnCode_t status = participant->get_default_datareader_qos(datareader_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default datareader qos");
    return false;
  }

  status = DDSPropertyQosPolicyHelper::add_property(
    datareader_qos.property,
    "dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size",
    "4096",
    DDS_BOOLEAN_FALSE);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to add qos property");
    return false;
  }

  status = DDSPropertyQosPolicyHelper::add_property(
    datareader_qos.property,
    "reader_resource_limits.dynamically_allocate_fragmented_samples",
    "1",
    DDS_BOOLEAN_FALSE);
  if (status != DDS_RETCODE_OK) {
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
  DDSDomainParticipant * participant,
  const rmw_qos_profile_t & qos_profile,
  DDS_DataWriterQos & datawriter_qos)
{
  DDS_ReturnCode_t status = participant->get_default_datawriter_qos(datawriter_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default datawriter qos");
    return false;
  }

  status = DDSPropertyQosPolicyHelper::add_property(
    datawriter_qos.property,
    "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
    "4096",
    DDS_BOOLEAN_FALSE);
  if (status != DDS_RETCODE_OK) {
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

void
dds_qos_lifespan_to_rmw_qos_lifespan(
  const DDS::DataWriterQos & dds_qos,
  rmw_qos_profile_t * qos)
{
  qos->lifespan.sec = dds_qos.lifespan.duration.sec;
  qos->lifespan.nsec = dds_qos.lifespan.duration.nanosec;
}

void
dds_qos_lifespan_to_rmw_qos_lifespan(
  const DDS::DataReaderQos & /*dds_qos*/,
  rmw_qos_profile_t * /*qos*/)
{
  // lifespan does does not exist in DataReader, so no-op here
}

template<typename AttributeT>
void
dds_qos_to_rmw_qos(
  const AttributeT & dds_qos,
  rmw_qos_profile_t * qos)
{
  switch (dds_qos.history.kind) {
    case DDS_KEEP_LAST_HISTORY_QOS:
      qos->history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
      break;
    case DDS_KEEP_ALL_HISTORY_QOS:
      qos->history = RMW_QOS_POLICY_HISTORY_KEEP_ALL;
      break;
    default:
      qos->history = RMW_QOS_POLICY_HISTORY_UNKNOWN;
      break;
  }
  qos->depth = static_cast<size_t>(dds_qos.history.depth);

  switch (dds_qos.reliability.kind) {
    case DDS_BEST_EFFORT_RELIABILITY_QOS:
      qos->reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
      break;
    case DDS_RELIABLE_RELIABILITY_QOS:
      qos->reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;
      break;
    default:
      qos->reliability = RMW_QOS_POLICY_RELIABILITY_UNKNOWN;
      break;
  }

  switch (dds_qos.durability.kind) {
    case DDS_TRANSIENT_LOCAL_DURABILITY_QOS:
      qos->durability = RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL;
      break;
    case DDS_VOLATILE_DURABILITY_QOS:
      qos->durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
      break;
    default:
      qos->durability = RMW_QOS_POLICY_DURABILITY_UNKNOWN;
      break;
  }

  qos->deadline.sec = dds_qos.deadline.period.sec;
  qos->deadline.nsec = dds_qos.deadline.period.nanosec;

  dds_qos_lifespan_to_rmw_qos_lifespan(dds_qos, qos);

  switch (dds_qos.liveliness.kind) {
    case DDS_AUTOMATIC_LIVELINESS_QOS:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_AUTOMATIC;
      break;
    case DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_NODE;
      break;
    case DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC;
      break;
    default:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_UNKNOWN;
      break;
  }
  qos->liveliness_lease_duration.sec = dds_qos.liveliness.lease_duration.sec;
  qos->liveliness_lease_duration.nsec = dds_qos.liveliness.lease_duration.nanosec;
}

template
void dds_qos_to_rmw_qos<DDS::DataWriterQos>(
  const DDS::DataWriterQos & dds_qos,
  rmw_qos_profile_t * qos);

template
void dds_qos_to_rmw_qos<DDS::DataReaderQos>(
  const DDS::DataReaderQos & dds_qos,
  rmw_qos_profile_t * qos);
