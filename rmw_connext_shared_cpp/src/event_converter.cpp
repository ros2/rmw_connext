// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#include <unordered_map>

#include "rmw_connext_shared_cpp/event.hpp"
#include "rmw_connext_shared_cpp/event_converter.hpp"

DDS::StatusKind get_status_kind_from_rmw(rmw_event_type_t event_t)
{
  return __rmw_event_type_to_dds_status_mask_map.at(event_t);
}

rmw_ret_t check_dds_ret_code(DDS::ReturnCode_t dds_return_code)
{
  switch (dds_return_code) {
    case DDS_RETCODE_OK:
      return RMW_RET_OK;
    case DDS_RETCODE_ERROR:
      return RMW_RET_ERROR;
    case DDS_RETCODE_TIMEOUT:
      return RMW_RET_TIMEOUT;
    default:
      return RMW_RET_ERROR;
  }
}
