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

#include "rosidl_typesupport_connext_cpp/message_type_support.h"

class ConnextSubscriberListener;

extern "C"
{
struct ConnextStaticSubscriberInfo
{
  DDSSubscriber * dds_subscriber_;
  ConnextSubscriberListener * listener_;
  DDSDataReader * topic_reader_;
  DDSReadCondition * read_condition_;
  bool ignore_local_publications;
  const message_type_support_callbacks_t * callbacks_;
};
}  // extern "C"

class ConnextSubscriberListener : public DDSSubscriberListener
{
public:
  virtual void on_subscription_matched(
    DDSDataReader * reader,
    const DDS_SubscriptionMatchedStatus & status)
  {
    (void) reader;
    current_count_ = status.current_count;
  }

  std::size_t current_count() const
  {
    return current_count_;
  }

private:
  std::atomic<std::size_t> current_count_;
};

#endif  // RMW_CONNEXT_CPP__CONNEXT_STATIC_SUBSCRIBER_INFO_HPP_
