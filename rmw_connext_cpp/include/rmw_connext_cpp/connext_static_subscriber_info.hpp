// Copyright 2014-2017 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_CONNEXT_CPP__CONNEXT_STATIC_SUBSCRIBER_INFO_HPP_
#define RMW_CONNEXT_CPP__CONNEXT_STATIC_SUBSCRIBER_INFO_HPP_

#include <atomic>

#include "rmw_connext_shared_cpp/ndds_include.hpp"

#include "rmw_connext_shared_cpp/types.hpp"

#include "rosidl_typesupport_connext_cpp/message_type_support.h"

class ConnextSubscriberListener;

extern "C"
{
struct ConnextStaticSubscriberInfo : ConnextCustomEventInfo
{
  DDS::Subscriber * dds_subscriber_;
  ConnextSubscriberListener * listener_;
  DDS::DataReader * topic_reader_;
  DDS::ReadCondition * read_condition_;
  bool ignore_local_publications;
  const message_type_support_callbacks_t * callbacks_;
  rmw_ret_t get_status(const DDS_StatusMask mask, void * event) override;
  DDSEntity* get_entity() override;
};
}  // extern "C"

class ConnextSubscriberListener : public DDS::SubscriberListener
{
public:
  virtual void on_subscription_matched(
    DDSDataReader *,
    const DDS_SubscriptionMatchedStatus & status)
  {
    current_count_ = status.current_count;
  }

  std::size_t current_count() const
  {
    return current_count_;
  }

private:
  std::atomic<std::size_t> current_count_;
};

/**
 * Remap the specific RTI Connext DDS DataReader Status to a generic RMW status type.
 *
 * @param mask input status mask
 * @param event
 */
inline rmw_ret_t ConnextStaticSubscriberInfo::get_status(
  const DDS_StatusMask mask,
  void * event)
{

  switch(mask) {
    case DDS_SAMPLE_REJECTED_STATUS:

      SampleRejectedStatus * sample_rejected = topic_reader_->sample_rejected_status();

      rmw_sample_rejected_status_t * rmw_sample_rejected_status = static_cast<rmw_sample_rejected_status_t*>(event);
      rmw_sample_rejected_status->total_count = sample_rejected->total_count();
      rmw_sample_rejected_status->total_count_change = sample_rejected->total_count_change();

      break;

    case DDS_LIVELINESS_CHANGED_STATUS:

      LivelinessChangedStatus * liveliness_changed = topic_reader_->liveliness_changed_status();

      rmw_liveliness_changed_status_t * rmw_liveliness_changed_status = static_cast<rmw_liveliness_changed_status_t*>(event);
      rmw_liveliness_changed_status->alive_count = liveliness_changed_status->alive_count();
      rmw_liveliness_changed_status->not_alive_count = liveliness_changed_status->not_alive_count();
      rmw_liveliness_changed_status->alive_count_change = liveliness_changed_status->alive_count_change();
      rmw_liveliness_changed_status->not_alive_count_change = liveliness_changed_status->not_alive_count_change();

      break;

    case DDS_REQUESTED_DEADLINE_MISSED_STATUS:

      RequestedDeadlineMissedStatus * requested_deadline_missed = topic_reader_->requested_deadline_missed_status();

      rmw_requested_deadline_missed_status_t * rmw_requested_deadline_missed_status = static_cast<rmw_requested_deadline_missed_status_t*>(event);
      rmw_requested_deadline_missed_status->total_count = requested_deadline_missed->total_count();
      rmw_requested_deadline_missed_status->total_count_change = requested_deadline_missed->total_count_change()

      break;

    case DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS:

      RequestedIncompatibleQosStatus * requested_incompatible = topic_reader_->requested_incompatible_qos_status();

      rmw_requested_incompatible_qos_status_t * rmw_requested_incompatible_qos_status = static_cast<rmw_requested_incompatible_qos_status_t*>(event);
      rmw_requested_incompatible_qos_status->total_count = topic_reader_->total_count();
      rmw_requested_incompatible_qos_status->total_count_change = topic_reader_->total_count_change();

      break;

    case DDS_SAMPLE_LOST_STATUS:

      SampleLostStatus * sample_lost_status = topic_reader_->sample_lost_status();

      rmw_sample_lost_status_t * rmw_sample_lost_status = static_cast<sample_lost_status_t *>(event;
      rmw_sample_lost_status->total_count-> sample_lost_status->total_count();
      rmw_sample_lost_status->total_count_change->sample_lost_status->total_count_change();

      break;

    case DDS_SUBSCRIPTION_MATCHED_STATUS:

      SubscriptionMatchedStatus * subscription_matched = topic_reader_->subscription_matched_status();

      rmw_subscription_matched_status_t * rmw_subscription_matched_status = static_cast<rmw_subscription_matched_status_t*>(event);
      rmw_subscription_matched_status->total_count = subscription_matched->total_count();
      rmw_subscription_matched_status->total_count_change = subscription_matched->total_count_changed();
      rmw_subscription_matched_status->current_count = subscription_matched->total_count();
      rmw_subscription_matched_status->current_count_change = subscription_matched->total_count_changed();

      break;

    default:
      return RMW_RET_QOS_UNSUPPORTED;
  }
  return RMW_RET_OK;
}

inline DDSEntity* ConnextStaticSubscriberInfo::get_entity()
{
  return dds_subscriber_;
}


#endif  // RMW_CONNEXT_CPP__CONNEXT_STATIC_SUBSCRIBER_INFO_HPP_
