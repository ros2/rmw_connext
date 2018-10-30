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

#include <mutex>
#include <string>

#include "rmw/error_handling.h"

#include "rmw_connext_shared_cpp/trigger_guard_condition.hpp"
#include "rmw_connext_shared_cpp/types.hpp"

// Uncomment this to get extra console output about discovery.
// #define DISCOVERY_DEBUG_LOGGING 1

void CustomDataReaderListener::add_information(
  const DDS_InstanceHandle_t & instance_handle,
  const std::string & topic_name,
  const std::string & type_name,
  EntityType entity_type)
{
  (void)entity_type;
  std::lock_guard<std::mutex> topic_descriptor_lock(topic_descriptor_mutex_);
  // store topic name and type name
  auto & topic_types = topic_names_and_types[topic_name];
  topic_types.insert(type_name);
  // store mapping to instance handle
  TopicDescriptor topic_descriptor;
  topic_descriptor.instance_handle = instance_handle;
  topic_descriptor.name = topic_name;
  topic_descriptor.type = type_name;
  topic_descriptors.push_back(topic_descriptor);
#ifdef DISCOVERY_DEBUG_LOGGING
  printf("+%s %s <%s>\n",
    entity_type == EntityType::Publisher ? "P" : "S",
    topic_name.c_str(),
    type_name.c_str());
#endif
}

void CustomDataReaderListener::remove_information(
  const DDS_InstanceHandle_t & instance_handle,
  EntityType entity_type)
{
  (void)entity_type;
  std::lock_guard<std::mutex> topic_descriptor_lock(topic_descriptor_mutex_);
  // find entry by instance handle
  for (auto it = topic_descriptors.begin(); it != topic_descriptors.end(); ++it) {
    if (DDS_InstanceHandle_equals(&it->instance_handle, &instance_handle)) {
      // remove entries
#ifdef DISCOVERY_DEBUG_LOGGING
      printf("-%s %s <%s>\n",
        entity_type == EntityType::Publisher ? "P" : "S",
        it->name.c_str(),
        it->type.c_str());
#endif
      auto & topic_types = topic_names_and_types[it->name];
      topic_types.erase(topic_types.find(it->type));
      if (topic_types.empty()) {
        topic_names_and_types.erase(it->name);
      }
      topic_descriptors.erase(it);
      break;
    }
  }
}

void CustomDataReaderListener::trigger_graph_guard_condition()
{
#ifdef DISCOVERY_DEBUG_LOGGING
  printf("graph guard condition triggered...\n");
#endif
  rmw_ret_t ret = trigger_guard_condition(implementation_identifier_, graph_guard_condition_);
  if (ret != RMW_RET_OK) {
    fprintf(stderr, "failed to trigger graph guard condition: %s\n", rmw_get_error_string().str);
  }
}
