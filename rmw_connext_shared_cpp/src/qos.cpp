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

#include <cassert>
#include <limits>

#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#include "./qos_impl.hpp"

namespace
{

bool
is_duration_default(rmw_duration_t duration)
{
  // Though the default QoS profiles use RMW_DURATION_INFINITE, historical usage allowed
  // 0 to mean unspecified, and should not be broken.
  return duration == 0 || duration == RMW_DURATION_INFINITE;
}

DDS_Duration_t
rmw_duration_to_dds(const rmw_duration_t & nanoseconds)
{
  DDS_Duration_t duration;
  if (nanoseconds == RMW_DURATION_INFINITE) {
    duration.sec = DDS::DURATION_INFINITE_SEC;
    duration.nanosec = DDS::DURATION_INFINITE_NSEC;
  } else {
    duration.sec = static_cast<DDS_Long>(nanoseconds / 1000000000LL);
    duration.nanosec = static_cast<DDS_UnsignedLong>(nanoseconds % 1000000000LL);
  }
  return duration;
}

rmw_duration_t
dds_duration_to_rmw(const DDS_Duration_t & duration)
{
  if (
    duration.sec == DDS::DURATION_INFINITE_SEC && duration.nanosec == DDS::DURATION_INFINITE_NSEC)
  {
    return RMW_DURATION_INFINITE;
  } else {
    return RCUTILS_S_TO_NS(duration.sec) + duration.nanosec;
  }
}

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
    case RMW_QOS_POLICY_HISTORY_UNKNOWN:
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
    case RMW_QOS_POLICY_RELIABILITY_UNKNOWN:
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
    case RMW_QOS_POLICY_DURABILITY_UNKNOWN:
    default:
      RMW_SET_ERROR_MSG("Unknown QoS durability policy");
      return false;
  }

  if (qos_profile.depth != RMW_QOS_POLICY_DEPTH_SYSTEM_DEFAULT) {
    entity_qos.history.depth = static_cast<DDS::Long>(qos_profile.depth);
  }

  // DDS_DeadlineQosPolicy has default value of  DDS_DURATION_INFINITE
  // don't overwrite if default passed
  if (!is_duration_default(qos_profile.deadline)) {
    entity_qos.deadline.period = rmw_duration_to_dds(qos_profile.deadline);
  }

  switch (qos_profile.liveliness) {
    case RMW_QOS_POLICY_LIVELINESS_AUTOMATIC:
      entity_qos.liveliness.kind = DDS::AUTOMATIC_LIVELINESS_QOS;
      break;
    case RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC:
      entity_qos.liveliness.kind = DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS;
      break;
    case RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT:
      break;
    case RMW_QOS_POLICY_LIVELINESS_UNKNOWN:
    default:
      RMW_SET_ERROR_MSG("Unknown QoS liveliness policy");
      return false;
  }
  if (!is_duration_default(qos_profile.liveliness_lease_duration)) {
    entity_qos.liveliness.lease_duration =
      rmw_duration_to_dds(qos_profile.liveliness_lease_duration);
  }

  // ensure the history depth is at least the requested queue size
  assert(entity_qos.history.depth >= 0);
  if (
    entity_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    static_cast<size_t>(entity_qos.history.depth) < qos_profile.depth)
  {
    if (qos_profile.depth > static_cast<size_t>((std::numeric_limits<DDS::Long>::max)())) {
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
  if (!is_duration_default(qos_profile.lifespan)) {
    entity_qos.lifespan.duration = rmw_duration_to_dds(qos_profile.lifespan);
  }
  return set_entity_qos_from_profile_generic(qos_profile, entity_qos);
}

}  // anonymous namespace

bool
get_datareader_qos(
  DDS::DomainParticipant * participant,
  const rmw_qos_profile_t & qos_profile,
  const char * dds_topic_name,
  DDS::DataReaderQos & datareader_qos)
{
  bool topic_profile_found = false;

  if (rmw_connext_shared_cpp::are_topic_profiles_allowed()) {
    DDS::DomainParticipantFactory * dpf = DDS::DomainParticipantFactory::get_instance();
    if (!dpf) {
      RMW_SET_ERROR_MSG("failed to get participant factory");
      return false;
    }
    if (DDS::RETCODE_OK == dpf->get_datareader_qos_from_profile(
        datareader_qos,
        dpf->get_default_library(),
        dds_topic_name))
    {
      topic_profile_found = true;
    }
    // no profile matching the topic name found -> look for the default profile
  }

  if (!topic_profile_found) {
    // This is an UNDOCUMMENTED rti Connext function.
    // What does it do?
    // It allows getting the profile marked as `is_default_profile="true"` in the externally
    // provided qos profile file while using topic filters.
    //
    // There are a few DomainParticipant and DomainParticipantFactory documented methods that sound
    // that can solve this: get_default_library, get_default_profile, get_default_profile_library.
    // They cannot. Those are only usefully if you programatically set the default profile/library,
    // but they don't allow you to detect the profile marked as default in the XML file.
    DDS::ReturnCode_t status = participant->get_default_datareader_qos_w_topic_name(
      datareader_qos, dds_topic_name);
    if (DDS::RETCODE_OK != status) {
      RMW_SET_ERROR_MSG("failed to get default datareader qos");
      return false;
    }
  }

  // This property will be added only if it wasn't specified in the external QoS profile file.
  DDS::ReturnCode_t status = DDS::PropertyQosPolicyHelper::add_property(
    datareader_qos.property,
    "dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size",
    "4096",
    DDS::BOOLEAN_FALSE);
  if (DDS::RETCODE_OK != status && DDS::RETCODE_PRECONDITION_NOT_MET != status) {
    RMW_SET_ERROR_MSG("failed to add qos property");
    return false;
  }

  // This property will be added only if it wasn't specified in the external QoS profile file.
  status = DDS::PropertyQosPolicyHelper::add_property(
    datareader_qos.property,
    "reader_resource_limits.dynamically_allocate_fragmented_samples",
    "1",
    DDS::BOOLEAN_FALSE);
  if (DDS::RETCODE_OK != status && DDS::RETCODE_PRECONDITION_NOT_MET != status) {
    RMW_SET_ERROR_MSG("failed to add qos property");
    return false;
  }

  if (topic_profile_found) {
    // ignore ROS QoS when a topic profile was found.
    return true;
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
  const char * dds_topic_name,
  DDS::DataWriterQos & datawriter_qos)
{
  bool topic_profile_found = false;
  if (rmw_connext_shared_cpp::are_topic_profiles_allowed()) {
    DDS::DomainParticipantFactory * dpf = DDS::DomainParticipantFactory::get_instance();
    if (!dpf) {
      RMW_SET_ERROR_MSG("failed to get participant factory");
      return false;
    }
    if (DDS::RETCODE_OK == dpf->get_datawriter_qos_from_profile(
        datawriter_qos,
        dpf->get_default_library(),
        dds_topic_name))
    {
      topic_profile_found = true;
    }
    // no profile matching the topic name found -> look for the default profile
  }

  if (!topic_profile_found) {
    // This is an UNDOCUMMENTED rti Connext function.
    // See comment in `get_datareader_qos()` for more details.
    DDS::ReturnCode_t status = participant->get_default_datawriter_qos_w_topic_name(
      datawriter_qos, dds_topic_name);
    if (DDS::RETCODE_OK != status) {
      RMW_SET_ERROR_MSG("failed to get default datareader qos");
      return false;
    }
  }

  // This property will be added only if it wasn't specified in the external QoS profile file.
  DDS::ReturnCode_t status = DDS::PropertyQosPolicyHelper::add_property(
    datawriter_qos.property,
    "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
    "4096",
    DDS::BOOLEAN_FALSE);
  if (DDS::RETCODE_OK != status && DDS::RETCODE_PRECONDITION_NOT_MET != status) {
    RMW_SET_ERROR_MSG("failed to add qos property");
    return false;
  }

  if (rmw_connext_shared_cpp::is_publish_mode_overriden()) {
    // TODO(wjwwood): conditionally use the async publish mode using a heuristic:
    //  https://github.com/ros2/rmw_connext/issues/190
    datawriter_qos.publish_mode.kind = DDS::ASYNCHRONOUS_PUBLISH_MODE_QOS;
  }

  if (topic_profile_found) {
    // ignore ROS QoS when a topic profile was found.
    return true;
  }

  if (!set_entity_qos_from_profile(qos_profile, datawriter_qos)) {
    return false;
  }

  return true;
}

rmw_qos_policy_kind_t
dds_qos_policy_to_rmw_qos_policy(DDS::QosPolicyId_t policy_id)
{
  switch (policy_id) {
    case DDS_DURABILITY_QOS_POLICY_ID:
      return RMW_QOS_POLICY_DURABILITY;
    case DDS_DEADLINE_QOS_POLICY_ID:
      return RMW_QOS_POLICY_DEADLINE;
    case DDS_LIVELINESS_QOS_POLICY_ID:
      return RMW_QOS_POLICY_LIVELINESS;
    case DDS_RELIABILITY_QOS_POLICY_ID:
      return RMW_QOS_POLICY_RELIABILITY;
    case DDS_HISTORY_QOS_POLICY_ID:
      return RMW_QOS_POLICY_HISTORY;
    case DDS_LIFESPAN_QOS_POLICY_ID:
      return RMW_QOS_POLICY_LIFESPAN;
    default:
      return RMW_QOS_POLICY_INVALID;
  }
}

template<typename AttributeT>
void
dds_qos_lifespan_to_rmw_qos_lifespan(
  const AttributeT & dds_qos,
  rmw_qos_profile_t * qos)
{
  qos->lifespan = dds_duration_to_rmw(dds_qos.lifespan.duration);
}

template<>
void
dds_qos_lifespan_to_rmw_qos_lifespan<DDS::DataReaderQos>(
  const DDS::DataReaderQos & /*dds_qos*/,
  rmw_qos_profile_t * /*qos*/)
{
  // lifespan does does not exist in DataReader, so no-op here
}

template<>
void
dds_qos_lifespan_to_rmw_qos_lifespan<DDS::SubscriptionBuiltinTopicData>(
  const DDS::SubscriptionBuiltinTopicData & /*dds_qos*/,
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
  dds_remote_qos_to_rmw_qos(dds_qos, qos);

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
}

template
void dds_qos_to_rmw_qos<DDS::DataWriterQos>(
  const DDS::DataWriterQos & dds_qos,
  rmw_qos_profile_t * qos);

template
void dds_qos_to_rmw_qos<DDS::DataReaderQos>(
  const DDS::DataReaderQos & dds_qos,
  rmw_qos_profile_t * qos);

template<typename AttributeT>
void
dds_remote_qos_to_rmw_qos(
  const AttributeT & dds_qos,
  rmw_qos_profile_t * qos)
{
  qos->history = RMW_QOS_POLICY_HISTORY_UNKNOWN;
  qos->depth = RMW_QOS_POLICY_DEPTH_SYSTEM_DEFAULT;

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

  qos->deadline = dds_duration_to_rmw(dds_qos.deadline.period);

  dds_qos_lifespan_to_rmw_qos_lifespan(dds_qos, qos);

  switch (dds_qos.liveliness.kind) {
    case DDS_AUTOMATIC_LIVELINESS_QOS:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_AUTOMATIC;
      break;
    case DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC;
      break;
    default:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_UNKNOWN;
      break;
  }
  qos->liveliness_lease_duration = dds_duration_to_rmw(dds_qos.liveliness.lease_duration);
}

template
void dds_remote_qos_to_rmw_qos<DDS::PublicationBuiltinTopicData>(
  const DDS::PublicationBuiltinTopicData & dds_qos,
  rmw_qos_profile_t * qos);

template
void dds_remote_qos_to_rmw_qos<DDS::SubscriptionBuiltinTopicData>(
  const DDS::SubscriptionBuiltinTopicData & dds_qos,
  rmw_qos_profile_t * qos);
