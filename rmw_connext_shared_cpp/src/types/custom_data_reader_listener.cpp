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
#include "rmw_connext_shared_cpp/namespace_prefix.hpp"
#include "rmw_connext_shared_cpp/demangle.hpp"
#include "rmw_connext_shared_cpp/guid_helper.hpp"
#include "rmw_connext_shared_cpp/types.hpp"

// Uncomment this to get extra console output about discovery.
// #define DISCOVERY_DEBUG_LOGGING 1

void CustomDataReaderListener::add_information(
  const DDS_GUID_t & participant_guid,
  const DDS_GUID_t & guid,
  const std::string & topic_name,
  const std::string & type_name,
  EntityType entity_type)
{
  (void)entity_type;
  std::lock_guard<std::mutex> lock(mutex_);

  // store topic name and type name
  topic_cache.AddTopic(participant_guid, guid, topic_name, type_name);

#ifdef DISCOVERY_DEBUG_LOGGING
  printf("+%s %s <%s>\n",
    entity_type == EntityType::Publisher ? "P" : "S",
    topic_name.c_str(),
    type_name.c_str());
#endif
}

void CustomDataReaderListener::remove_information(
  const DDS_GUID_t & guid,
  EntityType entity_type)
{
  (void)entity_type;
  std::lock_guard<std::mutex> lock(mutex_);

  // remove entries
  topic_cache.RemoveTopic(guid);
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

size_t CustomDataReaderListener::count_topic(const char * topic_name)
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = std::find_if(
    topic_cache.getTopicToTypes().begin(),
    topic_cache.getTopicToTypes().end(),
    [&](auto tnt) -> bool {
      auto fqdn = _demangle_if_ros_topic(tnt.first);
      if (fqdn == topic_name) {
        return true;
      }
      return false;
    });
  size_t count;
  if (it == topic_cache.getTopicToTypes().end()) {
    count = 0;
  } else {
    count = it->second.size();
  }
  return count;
}

void CustomDataReaderListener::fill_topic_names_and_types(
  bool no_demangle,
  std::map<std::string, std::set<std::string>> & topic_names_to_types)
{
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto topic_name_to_types : topic_cache.getTopicGuidToInfo()) {
    if (!no_demangle && (_get_ros_prefix_if_exists(topic_name_to_types.first) != ros_topic_prefix)) {
      continue;
    }
    topic_names_to_types[topic_name_to_types.second.name].insert(topic_name_to_types.second.type);
  }
}

void
CustomDataReaderListener::fill_service_names_and_types(
  std::map<std::string, std::set<std::string>> & services)
{
  for (auto topic_guid_to_info : topic_cache.getTopicGuidToInfo()) {
    std::string service_name = _demangle_service_from_topic(topic_guid_to_info.second.name);
    if (!service_name.length()) {
      // not a service
      continue;
    }
    std::string service_type = _demangle_service_type_only(topic_guid_to_info.second.type);
    if (service_type.size() != 0) {
      services[service_name].insert(service_type);
    }

  }
}

void CustomDataReaderListener::fill_topic_names_and_types_by_guid(
  bool no_demangle,
  std::map<std::string, std::set<std::string>> & topic_names_to_types_by_guid,
  DDS_GUID_t & participant_guid)
{
  std::lock_guard<std::mutex> lock(mutex_);

  auto tmp_map = topic_cache.filter(participant_guid);
  for (auto & topics : tmp_map) {
    if (!no_demangle && (_get_ros_prefix_if_exists(topics.first()) != ros_topic_prefix)) {
      continue;
    }
    if(topic_names_to_types_by_guid.find(topics.first()) == tmp_map.end()) {
      topic_names_to_types_by_guid[topics.first()] = std::set<std::string>();
    }
    for (auto &  type : topics->second()) {
      topic_names_to_types_by_guid[topics.first()].insert(type);
    }
  }
}

void CustomDataReaderListener::fill_service_names_and_types_by_guid(
    std::map<std::string, std::set<std::string>> & services,
    DDS_GUID_t & participant_guid)
{
  std::lock_guard<std::mutex> lock(mutex_);

  auto tmp_map = topic_cache.filter(participant_guid);
  for (auto & service : tmp_map) {
    std::string service_name = _demangle_service_from_topic(service.first());
    if (!service_name.length()) {
      // not a service
      continue;
    }
    if(services.find(service_name) == tmp_map.end()) {
      services[service_name] = std::set<std::string>();
    }
    for (auto & type : service->second()) {
      services[service_name].insert(type);
    }
  }
}
