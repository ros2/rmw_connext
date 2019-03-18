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
#include "rmw_connext_shared_cpp/connext_static_event_info.hpp"

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

inline rmw_ret_t ConnextStaticSubscriberInfo::get_status(
  const DDS_StatusMask mask,
  void * event)
{
  switch(mask) {
    case DDS_SAMPLE_REJECTED_STATUS:
      break;
    case DDS_LIVELINESS_CHANGED_STATUS: {
      DDS_LivelinessChangedStatus status;
      topic_reader_->get_liveliness_changed_status(status);
      // event << status; TODO(eknapp) we clearly do not stream to a void*
      break;
    }
    case DDS_REQUESTED_DEADLINE_MISSED_STATUS:
      break;
    case DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS:
      break;
    case DDS_SAMPLE_LOST_STATUS:
      break;
    case DDS_SUBSCRIPTION_MATCHED_STATUS:
      break;
    default:
      return RMW_RET_ERROR;
  }
}

inline DDSEntity* ConnextStaticSubscriberInfo::get_entity()
{
  return dds_subscriber_;
}


#endif  // RMW_CONNEXT_CPP__CONNEXT_STATIC_SUBSCRIBER_INFO_HPP_
