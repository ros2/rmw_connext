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

#ifndef RMW_CONNEXT_CPP__CONNEXT_STATIC_PUBLISHER_INFO_HPP_
#define RMW_CONNEXT_CPP__CONNEXT_STATIC_PUBLISHER_INFO_HPP_

#include <atomic>

#include "rmw_connext_shared_cpp/types.hpp"

#include "rosidl_typesupport_connext_cpp/message_type_support.h"

class ConnextPublisherListener;

extern "C"
{
struct ConnextStaticPublisherInfo : ConnextCustomEventInfo
{
  DDS::Publisher * dds_publisher_;
  ConnextPublisherListener * listener_;
  DDS::DataWriter * topic_writer_;
  const message_type_support_callbacks_t * callbacks_;
  rmw_gid_t publisher_gid;
};
}  // extern "C"

class ConnextPublisherListener : public DDS::PublisherListener
{
public:
  virtual void on_publication_matched(
    DDSDataWriter *,
    const DDS_PublicationMatchedStatus & status)
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
 * Remap the specific RTI Connext DDS DataWriter Status to a generic RMW status type.
 *
 * @param mask input status mask
 * @param event
 */
inline rmw_ret_code ConnextStaticSubscriberInfo::get_status(
        const DDS_StatusMask mask,
        void * event) {

  switch(mask) {

    case DDS_LIVELINESS_LOST:

      LivelinessLostStatus * liveliness_lost = topic_writer_->liveliness_lost_status();

      rmw_liveliness_lost_t rmw_liveliness_lost = new liveliness_lost_t();
      rmw_liveliness_lost.total_count = liveliness_lost.total_count();
      rmw_liveliness_lost.total_count_change = liveliness_lost.total_count_change();

      //todo return value?
      //todo assign to void *?
      break;

    case DDS_OFFERED_DEADLINE_MISSED:

      OfferedDeadlineMissedStatus * offered_deadline_missed = topic_writer_->offered_deadline_missed_status();

      rmw_offered_deadline_missed_t rmw_offered_deadline_missed = new rmw_offered_deadline_missed_t();
      rmw_offered_deadline_missed.total_count = offered_deadline_missed->total_count();
      rmw_offered_deadline_missed.total_count_change = offered_deadline_missed->total_count_change();
      //TODO
      //rmw_offered_deadline_missed.last_instance_handle = offered_deadline_missed->last_instance_handle();

      //todo return value?
      //todo assign to void *?
      break;

    case DDS_OFFERED_INCOMPATIBLE_QOS:

      OfferedIncompatibleQosStatus * offered_incompatible_status = topic_writer_->offered_incompatible_status();

      rmw_offered_incompatible_qos_t rmw_offered_incompatible_qos = new rmw_offered_incompatible_qos_t();

      rmw_offered_incompatible_qos.total_count = topic_writer_->total_count();
      rmw_offered_incompatible_qos.total_count_change = topic_writer_->total_count_change();
      //TODO
      //rmw_offered_incompatible_qos.last_policy_id = topic_writer_->last_policy_id();
      //rmw_offered_incompatible_qos.policies = topic_writer_->policies();

      //todo return value?
      //todo assign to void *?
      return RMW_RET_ERROR;

    case DDS_PUBLICATION_MATCHED:

      PublicationMatchedStatus * publication_matched_status = topic_writer_->publication_matched_status();

      rmw_publication_matched_t rmw_publication_matched = new rmw_publication_matched_t();
      rmw_publication_matched_t.total_count = topic_writer_->total_count();
      rmw_publication_matched_t.total_count_change = topic_writer_->total_count_change();
      rmw_publication_matched_t.current_count = topic_writer_->current_count();
      rmw_publication_matched_t.current_count_change = topic_writer_->current_count_change();
      //TODO
      //rmw_publication_matched_t.last_subscription_handle = topic_writer_->last_subscription_handle();

      //todo return value?
      //todo assign to void *?
      break;

    default:
      //todo is this really ok?
      return RMW_RET_OK;
  }
}
inline DDSEntity* ConnextStaticSubscriberInfo::get_entity()
{
  return dds_publisher_;
}

#endif  // RMW_CONNEXT_CPP__CONNEXT_STATIC_PUBLISHER_INFO_HPP_
