// Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0
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


#include <map>
#include <set>
#include <string>
#include <functional>
#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/logging_macros.h"
#include "rcutils/strdup.h"
#include "rcutils/types.h"

#include "rmw/allocators.h"
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/error_handling.h"
#include "rmw/get_topic_names_and_types.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/impl/cpp/key_value.hpp"
#include "rmw/names_and_types.h"
#include "rmw/rmw.h"

#include "rmw_connext_shared_cpp/types.hpp"
#include "rmw_connext_shared_cpp/demangle.hpp"
#include "rmw_connext_shared_cpp/namespace_prefix.hpp"
#include "rmw_connext_shared_cpp/service_names_and_types.hpp"
#include "rmw_connext_shared_cpp/subscriber_names_and_types.hpp"
#include "rmw_connext_shared_cpp/publisher_names_and_types.hpp"
#include "rmw_connext_shared_cpp/guid_helper.hpp"

constexpr char SAMPLE_PREFIX[] = "/Sample_";


bool
is_node_match(DDS_UserDataQosPolicy & user_data_qos,
              const char * node_name,
              const char * node_namespace)
{
  uint8_t * buf = user_data_qos.value.get_contiguous_buffer();
  if (buf) {
    std::vector<uint8_t> kv(buf, buf + user_data_qos.value.length());
    auto map = rmw::impl::cpp::parse_key_value(kv);
    auto name_found = map.find("name");
    auto ns_found = map.find("namespace");

    if (name_found != map.end() && ns_found != map.end()) {
      std::string name(name_found->second.begin(), name_found->second.end());
      std::string ns(ns_found->second.begin(), ns_found->second.end());
      return (strcmp(node_name, name.c_str()) == 0 && strcmp(node_namespace, ns.c_str()) == 0);
    }
  }
  return false;
}

rmw_ret_t
get_key(ConnextNodeInfo * node_info,
        const char * node_name,
        const char * node_namespace,
        DDS_GUID_t * key)
{
  auto participant = node_info->participant;
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return RMW_RET_ERROR;
  }

  DDS_DomainParticipantQos dpqos;
  auto dds_ret = participant->get_qos(dpqos);
  if (dds_ret == DDS_RETCODE_OK && is_node_match(dpqos.user_data, node_name, node_namespace)) {
    DDS_InstanceHandle_to_GUID(key, node_info->participant->get_instance_handle());
    return RMW_RET_OK;
  }

  DDS_InstanceHandleSeq handles;
  if (participant->get_discovered_participants(handles) != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("unable to fetch discovered participants.");
    return RMW_RET_ERROR;
  }

  for (DDS_Long i = 0; i < handles.length(); ++i) {
    DDS_ParticipantBuiltinTopicData pbtd;
    auto dds_ret = participant->get_discovered_participant_data(pbtd, handles[i]);
    if (dds_ret == DDS_RETCODE_OK) {
      uint8_t * buf = pbtd.user_data.value.get_contiguous_buffer();
      if (buf) {
        std::vector<uint8_t> kv(buf, buf + pbtd.user_data.value.length());
        auto map = rmw::impl::cpp::parse_key_value(kv);
        auto name_found = map.find("name");
        auto ns_found = map.find("namespace");

        if (name_found != map.end() && ns_found != map.end()) {
          std::string name(name_found->second.begin(), name_found->second.end());
          std::string ns(ns_found->second.begin(), ns_found->second.end());
          if (strcmp(node_name, name.c_str()) == 0 &&
              strcmp(node_namespace, ns.c_str()) == 0) {
            DDS_BuiltinTopicKey_to_GUID(key, pbtd.key);
            return RMW_RET_OK;
          }
        }
      }
    } else {
      RMW_SET_ERROR_MSG("unable to fetch discovered participants data.");
      return RMW_RET_ERROR;
    }
  }
  RMW_SET_ERROR_MSG("unable to match node_name/namespace with discovered nodes.");
  return RMW_RET_ERROR;
}

rmw_ret_t
validate_names_and_namespace(const char * node_name,
                             const char * node_namespace)
{
  if (!node_name) {
    RMW_SET_ERROR_MSG("null node name");
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (!node_namespace) {
    RMW_SET_ERROR_MSG("null node namespace");
    return RMW_RET_INVALID_ARGUMENT;
  }
  return RMW_RET_ERROR;
}

rmw_ret_t
copy_topics_names_and_types(const std::map<std::string, std::set<std::string>> &topics,
                            rcutils_allocator_t *allocator,
                            bool no_demangle,
                            rmw_names_and_types_t *topic_names_and_types) {
  // Copy data to results handle
  if (topics.size() > 0) {
    // Setup string array to store names
    rmw_ret_t rmw_ret = rmw_names_and_types_init(topic_names_and_types, topics.size(), allocator);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }
    // Setup cleanup function, in case of failure below
    auto fail_cleanup = [&topic_names_and_types]() {
      rmw_ret_t rmw_ret = rmw_names_and_types_fini(topic_names_and_types);
      if (rmw_ret != RMW_RET_OK) {
        RCUTILS_LOG_ERROR("error during report of error: %s", rmw_get_error_string().str);
      }
    };
    // Setup demangling functions based on no_demangle option
    auto demangle_topic = _demangle_if_ros_topic;
    auto demangle_type = _demangle_if_ros_type;
    if (no_demangle) {
      auto noop = [](const std::string &in) {
        return in;
      };
      demangle_topic = noop;
      demangle_type = noop;
    }
    // For each topic, store the name, initialize the string array for types, and store all types
    size_t index = 0;
    for (const auto &topic_n_types : topics) {
      // Duplicate and store the topic_name
      char *topic_name = rcutils_strdup(demangle_topic(topic_n_types.first).c_str(), *allocator);
      if (!topic_name) {
        RMW_SET_ERROR_MSG("failed to allocate memory for topic name");
        fail_cleanup();
        return RMW_RET_BAD_ALLOC;
      }
      topic_names_and_types->names.data[index] = topic_name;
      // Setup storage for types
      {
        rcutils_ret_t rcutils_ret = rcutils_string_array_init(
          &topic_names_and_types->types[index],
          topic_n_types.second.size(),
          allocator);
        if (rcutils_ret != RCUTILS_RET_OK) {
          RMW_SET_ERROR_MSG(rcutils_get_error_string().str);
          fail_cleanup();
          return rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
        }
      }
      // Duplicate and store each type for the topic
      size_t type_index = 0;
      for (const auto &type : topic_n_types.second) {
        char *type_name = rcutils_strdup(demangle_type(type).c_str(), *allocator);
        if (!type_name) {
          RMW_SET_ERROR_MSG("failed to allocate memory for type name");
          fail_cleanup();
          return RMW_RET_BAD_ALLOC;
        }
        topic_names_and_types->types[index].data[type_index] = type_name;
        ++type_index;
      }  // for each type
      ++index;
    }  // for each topic
  }
  return RMW_RET_OK;
}

rmw_ret_t
copy_services_to_names_and_types(const std::map<std::string, std::set<std::string>> &services,
                                 rcutils_allocator_t *allocator,
                                 rmw_names_and_types_t *service_names_and_types) {
  // Fill out service_names_and_types
  if (services.size()) {
    // Setup string array to store names
    rmw_ret_t rmw_ret =
      rmw_names_and_types_init(service_names_and_types, services.size(), allocator);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }
    // Setup cleanup function, in case of failure below
    auto fail_cleanup = [&service_names_and_types]() {
      rmw_ret_t rmw_ret = rmw_names_and_types_fini(service_names_and_types);
      if (rmw_ret != RMW_RET_OK) {
        RCUTILS_LOG_ERROR("error during report of error: %s", rmw_get_error_string().str);
      }
    };
    // For each service, store the name, initialize the string array for types, and store all types
    size_t index = 0;
    for (const auto &service_n_types : services) {
      // Duplicate and store the service_name
      char *service_name = rcutils_strdup(service_n_types.first.c_str(), *allocator);
      if (!service_name) {
        RMW_SET_ERROR_MSG("failed to allocate memory for service name");
        fail_cleanup();
        return RMW_RET_BAD_ALLOC;
      }
      service_names_and_types->names.data[index] = service_name;
      // Setup storage for types
      {
        rcutils_ret_t rcutils_ret = rcutils_string_array_init(
          &service_names_and_types->types[index],
          service_n_types.second.size(),
          allocator);
        if (rcutils_ret != RCUTILS_RET_OK) {
          RMW_SET_ERROR_MSG(rcutils_get_error_string().str);
          fail_cleanup();
          return rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
        }
      }
      // Duplicate and store each type for the service
      size_t type_index = 0;
      for (const auto &type : service_n_types.second) {
        // Strip the SAMPLE_PREFIX if it is found (e.g. from services using OpenSplice typesupport).
        // It may not be found if services are detected using other typesupports.
        size_t n = type.find(SAMPLE_PREFIX);
        std::string stripped_type = type;
        if (std::string::npos != n) {
          stripped_type = type.substr(0, n + 1) + type.substr(n + strlen(SAMPLE_PREFIX));
        }
        char *type_name = rcutils_strdup(stripped_type.c_str(), *allocator);
        if (!type_name) {
          RMW_SET_ERROR_MSG("failed to allocate memory for type name");
          fail_cleanup();
          return RMW_RET_BAD_ALLOC;
        }
        service_names_and_types->types[index].data[type_index] = type_name;
        ++type_index;
      }  // for each type
      ++index;
    }  // for each service
  }
  return RMW_RET_OK;
}


rmw_ret_t
get_subscriber_names_and_types_by_node(
  const char * implementation_identifier,
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  bool no_demangle,
  rmw_names_and_types_t * topic_names_and_types)
{
  if (!node) {
    RMW_SET_ERROR_MSG("null node handle");
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (node->implementation_identifier != implementation_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }

  rmw_ret_t ret = rmw_names_and_types_check_zero(topic_names_and_types);
  if (ret != RMW_RET_OK) {
    return ret;
  }
  ret = validate_names_and_namespace(node_name, node_namespace);
  if (ret != RMW_RET_OK) {
    return ret;
  }

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }

  DDS_GUID_t key;
  auto get_guid_err = get_key(node_info, node_name, node_namespace, &key);
  if (get_guid_err != RMW_RET_OK) {
    return get_guid_err;
  }

  // combine publisher and subscriber information
  std::map<std::string, std::set<std::string>> topics;
  node_info->subscriber_listener->fill_topic_names_and_types_by_guid(no_demangle, topics, key);

  rmw_ret_t rmw_ret;
  rmw_ret = copy_topics_names_and_types(topics, allocator, no_demangle, topic_names_and_types);
  if (rmw_ret != RMW_RET_OK) {
    return rmw_ret;
  }

  return RMW_RET_OK;
}

rmw_ret_t
get_publisher_names_and_types_by_node(
  const char * implementation_identifier,
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  bool no_demangle,
  rmw_names_and_types_t * topic_names_and_types)
{
  if (!node) {
    RMW_SET_ERROR_MSG("null node handle");
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (node->implementation_identifier != implementation_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }

  rmw_ret_t ret = rmw_names_and_types_check_zero(topic_names_and_types);
  if (ret != RMW_RET_OK) {
    return ret;
  }
  ret = validate_names_and_namespace(node_name, node_namespace);
  if (ret != RMW_RET_OK) {
    return ret;
  }

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }

  DDS_GUID_t key;
  auto get_guid_err = get_key(node_info, node_name, node_namespace, &key);
  if (get_guid_err != RMW_RET_OK) {
    return get_guid_err;
  }

  // combine publisher and subscriber information
  std::map<std::string, std::set<std::string>> topics;
  node_info->publisher_listener->fill_topic_names_and_types_by_guid(no_demangle, topics, key);

  rmw_ret_t rmw_ret;
  rmw_ret = copy_topics_names_and_types(topics, allocator, no_demangle, topic_names_and_types);
  if (rmw_ret != RMW_RET_OK) {
    return rmw_ret;
  }

  return RMW_RET_OK;
}

rmw_ret_t
get_service_names_and_types_by_node(
  const char * implementation_identifier,
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  rmw_names_and_types_t * service_names_and_types)
{
  if (!node) {
    RMW_SET_ERROR_MSG("null node handle");
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (node->implementation_identifier != implementation_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }

  rmw_ret_t ret = rmw_names_and_types_check_zero(service_names_and_types);
  if (ret != RMW_RET_OK) {
    return ret;
  }

  auto node_info = static_cast<ConnextNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }

  DDS_GUID_t key;
  auto get_guid_err = get_key(node_info, node_name, node_namespace, &key);
  if (get_guid_err != RMW_RET_OK) {
    return get_guid_err;
  }

  // combine publisher and subscriber information
  std::map<std::string, std::set<std::string>> services;
  node_info->subscriber_listener->fill_service_names_and_types_by_guid(services, key);

  rmw_ret_t rmw_ret;
  rmw_ret = copy_services_to_names_and_types(services, allocator, service_names_and_types);
  if (rmw_ret != RMW_RET_OK) {
    return rmw_ret;
  }

  return RMW_RET_OK;
} // extern "C"
