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

#include <string>
#include <vector>

#include "rcutils/logging_macros.h"
#include "rcutils/strdup.h"

#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/key_value.hpp"
#include "rmw/sanity_checks.h"

#include "rmw_connext_shared_cpp/ndds_include.hpp"
#include "rmw_connext_shared_cpp/node_names.hpp"
#include "rmw_connext_shared_cpp/types.hpp"

rmw_ret_t
get_node_names(
  const char * implementation_identifier,
  const rmw_node_t * node,
  rcutils_string_array_t * node_names)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != implementation_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (rmw_check_zero_rmw_string_array(node_names) != RMW_RET_OK) {
    return RMW_RET_ERROR;
  }

  DDSDomainParticipant * participant = static_cast<ConnextNodeInfo *>(node->data)->participant;
  DDS_InstanceHandleSeq handles;
  if (participant->get_discovered_participants(handles) != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("unable to fetch discovered participants.");
    return RMW_RET_ERROR;
  }
  auto length = handles.length() + 1;  // add yourself
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_ret_t rcutils_ret = rcutils_string_array_init(node_names, length, &allocator);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
    return rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
  }

  DDS_DomainParticipantQos participant_qos;
  DDS_ReturnCode_t status = participant->get_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default participant qos");
    return RMW_RET_ERROR;
  }
  node_names->data[0] = rcutils_strdup(participant_qos.participant_name.name, allocator);
  if (!node_names->data[0]) {
    RMW_SET_ERROR_MSG("could not allocate memory for node name")
    return RMW_RET_BAD_ALLOC;
  }

  for (auto i = 1; i < length; ++i) {
    DDS::ParticipantBuiltinTopicData pbtd;
    auto dds_ret = participant->get_discovered_participant_data(pbtd, handles[i - 1]);
    std::string name;
    if (DDS_RETCODE_OK == dds_ret) {
      auto data = static_cast<unsigned char *>(pbtd.user_data.value.get_contiguous_buffer());
      std::vector<uint8_t> kv(data, data + pbtd.user_data.value.length());
      auto map = rmw::impl::cpp::parse_key_value(kv);
      auto found = map.find("name");
      if (found != map.end()) {
        name = std::string(found->second.begin(), found->second.end());
      }
      if (name.empty()) {
        // use participant name if no name was found in the user data
        if (pbtd.participant_name.name) {
          name = pbtd.participant_name.name;
        }
      }
    }
    if (name.empty()) {
      // ignore discovered participants without a name
      node_names->data[i] = nullptr;
      continue;
    }
    node_names->data[i] = rcutils_strdup(name.c_str(), allocator);
    if (!node_names->data[i]) {
      RMW_SET_ERROR_MSG("could not allocate memory for node name")
      rcutils_ret = rcutils_string_array_fini(node_names);
      if (rcutils_ret != RCUTILS_RET_OK) {
        RCUTILS_LOG_ERROR_NAMED(
          "rmw_connext_cpp",
          "failed to cleanup during error handling: %s", rcutils_get_error_string_safe())
      }
      return RMW_RET_BAD_ALLOC;
    }
  }

  return RMW_RET_OK;
}
