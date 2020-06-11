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

#include "rmw_connext_cpp/connext_static_subscriber_info.hpp"
#include "rmw_connext_shared_cpp/event_converter.hpp"
#include "rmw_connext_shared_cpp/qos.hpp"

rmw_ret_t ConnextStaticSubscriberInfo::get_status(
  rmw_event_type_t event_type,
  void * event)
{
  switch (event_type) {
    case RMW_EVENT_LIVELINESS_CHANGED:
      {
        DDS::LivelinessChangedStatus liveliness_changed;
        DDS::ReturnCode_t dds_return_code =
          topic_reader_->get_liveliness_changed_status(liveliness_changed);

        rmw_ret_t from_dds = check_dds_ret_code(dds_return_code);
        if (from_dds != RMW_RET_OK) {
          return from_dds;
        }

        rmw_liveliness_changed_status_t * rmw_liveliness_changed_status =
          static_cast<rmw_liveliness_changed_status_t *>(event);
        rmw_liveliness_changed_status->alive_count = liveliness_changed.alive_count;
        rmw_liveliness_changed_status->not_alive_count = liveliness_changed.not_alive_count;
        rmw_liveliness_changed_status->alive_count_change = liveliness_changed.alive_count_change;
        rmw_liveliness_changed_status->not_alive_count_change =
          liveliness_changed.not_alive_count_change;

        break;
      }
    case RMW_EVENT_REQUESTED_DEADLINE_MISSED:
      {
        DDS::RequestedDeadlineMissedStatus requested_deadline_missed;
        DDS::ReturnCode_t dds_return_code =
          topic_reader_->get_requested_deadline_missed_status(requested_deadline_missed);

        rmw_ret_t from_dds = check_dds_ret_code(dds_return_code);
        if (from_dds != RMW_RET_OK) {
          return from_dds;
        }

        rmw_requested_deadline_missed_status_t * rmw_requested_deadline_missed_status =
          static_cast<rmw_requested_deadline_missed_status_t *>(event);
        rmw_requested_deadline_missed_status->total_count = requested_deadline_missed.total_count;
        rmw_requested_deadline_missed_status->total_count_change =
          requested_deadline_missed.total_count_change;

        break;
      }
    case RMW_EVENT_REQUESTED_QOS_INCOMPATIBLE:
      {
        DDS::RequestedIncompatibleQosStatus requested_incompatible_qos;
        DDS::ReturnCode_t dds_return_code =
          topic_reader_->get_requested_incompatible_qos_status(requested_incompatible_qos);

        rmw_ret_t from_dds = check_dds_ret_code(dds_return_code);
        if (RMW_RET_OK != from_dds) {
          return from_dds;
        }

        rmw_requested_qos_incompatible_event_status_t * rmw_requested_qos_incompatible =
          static_cast<rmw_requested_qos_incompatible_event_status_t *>(event);
        rmw_requested_qos_incompatible->total_count = requested_incompatible_qos.total_count;
        rmw_requested_qos_incompatible->total_count_change =
          requested_incompatible_qos.total_count_change;
        rmw_requested_qos_incompatible->last_policy_kind = dds_qos_policy_to_rmw_qos_policy(
          requested_incompatible_qos.last_policy_id);

        break;
      }
    case RMW_EVENT_MESSAGE_LOST:
      {
        DDS::SampleLostStatus sample_lost_status;

        DDS::ReturnCode_t dds_return_code =
          topic_reader_->get_sample_lost_status(sample_lost_status);
        rmw_ret_t from_dds = check_dds_ret_code(dds_return_code);
        if (RMW_RET_OK != from_dds) {
          return from_dds;
        }

        rmw_message_lost_status_t * rmw_message_lost =
          static_cast<rmw_message_lost_status_t *>(event);
        rmw_message_lost->total_count = sample_lost_status.total_count;
        rmw_message_lost->total_count_change = sample_lost_status.total_count_change;
        break;
      }
    default:
      return RMW_RET_UNSUPPORTED;
  }
  return RMW_RET_OK;
}

DDS::Entity * ConnextStaticSubscriberInfo::get_entity()
{
  return topic_reader_;
}
