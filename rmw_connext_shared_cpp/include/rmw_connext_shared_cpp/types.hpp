// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_CONNEXT_SHARED_CPP__TYPES_HPP_
#define RMW_CONNEXT_SHARED_CPP__TYPES_HPP_

#include <cassert>
#include <exception>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#include "rmw/rmw.h"

#include "ndds_include.hpp"

enum EntityType {Publisher, Subscriber};

class CustomDataReaderListener
  : public DDSDataReaderListener
{
public:
  explicit
  CustomDataReaderListener(
    const char * implementation_identifier, rmw_guard_condition_t * graph_guard_condition)
  : graph_guard_condition_(graph_guard_condition),
    implementation_identifier_(implementation_identifier)
  {}

  std::map<std::string, std::multiset<std::string>> topic_names_and_types;

  virtual void add_information(
    const DDS_InstanceHandle_t & instance_handle,
    const std::string & topic_name,
    const std::string & type_name,
    EntityType entity_type);

  virtual void remove_information(
    const DDS_InstanceHandle_t & instance_handle,
    EntityType entity_type);

  virtual void trigger_graph_guard_condition();

private:
  mutable std::mutex topic_descriptor_mutex_;
  struct TopicDescriptor
  {
    DDS_InstanceHandle_t instance_handle;
    std::string name;
    std::string type;
  };
  std::list<TopicDescriptor> topic_descriptors;
  rmw_guard_condition_t * graph_guard_condition_;
  const char * implementation_identifier_;
};

class CustomPublisherListener
  : public CustomDataReaderListener
{
public:
  CustomPublisherListener(
    const char * implementation_identifier, rmw_guard_condition_t * graph_guard_condition)
  : CustomDataReaderListener(implementation_identifier, graph_guard_condition)
  {}

  virtual void on_data_available(DDSDataReader * reader);
};

class CustomSubscriberListener
  : public CustomDataReaderListener
{
public:
  CustomSubscriberListener(
    const char * implementation_identifier, rmw_guard_condition_t * graph_guard_condition)
  : CustomDataReaderListener(implementation_identifier, graph_guard_condition)
  {}

  virtual void on_data_available(DDSDataReader * reader);
};

struct ConnextNodeInfo
{
  DDSDomainParticipant * participant;
  CustomPublisherListener * publisher_listener;
  CustomSubscriberListener * subscriber_listener;
  rmw_guard_condition_t * graph_guard_condition;
};

struct ConnextPublisherGID
{
  DDS_InstanceHandle_t publication_handle;
};

struct ConnextWaitSetInfo
{
  DDSWaitSet * wait_set;
  DDSConditionSeq * active_conditions;
  DDSConditionSeq * attached_conditions;
};

#endif  // RMW_CONNEXT_SHARED_CPP__TYPES_HPP_
