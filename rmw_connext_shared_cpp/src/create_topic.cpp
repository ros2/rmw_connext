// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include "rmw_connext_shared_cpp/create_topic.hpp"

#include <cassert>
#include <mutex>

#include "rmw/error_handling.h"
#include "rmw/types.h"

#include "rmw_connext_shared_cpp/types.hpp"
#include "rmw_connext_shared_cpp/ndds_include.hpp"

DDS::Topic *
rmw_connext_shared_cpp::create_topic(
  const rmw_node_t * node,
  const char * topic_name,
  const char * dds_topic_name,
  const char * dds_topic_type)
{
  // This function is internal, and should be called from functions that already verified
  // the node validity.
  assert(node);
  assert(node->data);

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);

  std::lock_guard<std::mutex> guard(node_info->topic_creation_mutex);

  // This function returns a copy that has to be destroyed independently.
  DDS::Topic * topic = participant->find_topic(dds_topic_name, DDS::Duration_t::from_seconds(0));
  if (!topic) {
    DDS::TopicQos default_topic_qos;
    DDS::ReturnCode_t status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS::RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to get default topic qos");
      return nullptr;
    }

    topic = participant->create_topic(
      dds_topic_name, dds_topic_type,
      default_topic_qos, nullptr, DDS::STATUS_MASK_NONE);
    if (!topic) {
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "failed to create topic '%s' for node namespace='%s' name='%s'",
        topic_name, node->namespace_, node->name);
      return nullptr;
    }
  }
  return topic;
}
