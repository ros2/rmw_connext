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

#ifndef RMW_CONNEXT_SHARED_CPP__CONNEXT_STATIC_EVENT_INFO_HPP_
#define RMW_CONNEXT_SHARED_CPP__CONNEXT_STATIC_EVENT_INFO_HPP_

#include "rmw_connext_shared_cpp/ndds_include.hpp"
#include "ndds/ndds_cpp.h"
#include "ndds/ndds_namespace_cpp.h"
#include "rmw/ret_types.h"

using namespace DDS;

typedef struct ConnextCustomEventInfo
{
  virtual rmw_ret_t get_status(const DDS_StatusMask mask, void * event) = 0;
  virtual DDSEntity* get_entity() = 0;
  /**
    * Assign the input DDS return code to its corresponding RMW return code.
    * @param dds_return_code input DDS return code
    * @return to_return the corresponding rmw_ret_t that maps to the input DDS_ReturnCode_t. By
    * default RMW_RET_ERROR is returned if no corresponding rmw_ret_t is not defined.
    */
  rmw_ret_t check_dds_ret_code(DDS_ReturnCode_t & dds_return_code) {

    switch(dds_return_code) {

      case DDS_RETCODE_OK:
        return RMW_RET_OK;
      case DDS_RETCODE_ERROR:
        return RMW_RET_ERROR;
      case DDS_RETCODE_UNSUPPORTED:
        return RMW_RET_EVENT_UNSUPPORTED;
      case DDS_RETCODE_BAD_PARAMETER:
        return RMW_RET_INVALID_ARGUMENT;
      case DDS_RETCODE_PRECONDITION_NOT_MET:
        return RMW_RETCODE_PRECONDITION_NOT_MET;
      case DDS_RETCODE_OUT_OF_RESOURCES:
        return RMW_RET_OUT_OF_RESOURCES;
      case DDS_RETCODE_NOT_ENABLED:
        return RMW_RET_NOT_ENABLED;
      case DDS_RETCODE_IMMUTABLE_POLICY:
        return RMW_RET_IMMUTABLE_POLICY;
      case DDS_RETCODE_INCONSISTENT_POLICY:
        return RMW_RET_INCONSISTENT_POLICY;
      case DDS_RETCODE_TIMEOUT:
        return RMW_RET_TIMEOUT;
      case DDS_RETCODE_NO_DATA:
        return RMW_RET_NO_DATA;
      // The following codes are currently not handled:
      // case DDS_RETCODE_ALREADY_DELETED:
      // case DDS_RETCODE_ILLEGAL_OPERATION:
      default:
        return RMW_RET_ERROR;
    }
  }
} ConnextCustomEventInfo;

#endif  // RMW_CONNEXT_SHARED_CPP__CONNEXT_STATIC_EVENT_INFO_HPP_