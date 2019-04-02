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

/**
 *
 */
typedef struct ConnextCustomEventInfo
{
  /**
   * Return the corresponding RMW status given the input DDS_StatusMask and its corresponding event.
   * @param mask input DDS_StatusMask
   * @param event
   * @return
   */
  virtual rmw_ret_t get_status(const DDS_StatusMask mask, void * event) = 0;
  virtual DDSEntity* get_entity() = 0;
  /**
    * Assign the input DDS return code to its corresponding RMW return code.
    * @param dds_return_code input DDS return code
    * @return to_return the corresponding rmw_ret_t that maps to the input DDS_ReturnCode_t. By
    * default RMW_RET_ERROR is returned if no corresponding rmw_ret_t is not defined.
    */
  rmw_ret_t check_dds_ret_code(DDS_ReturnCode_t & dds_return_code)
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
} ConnextCustomEventInfo;

#endif  // RMW_CONNEXT_SHARED_CPP__CONNEXT_STATIC_EVENT_INFO_HPP_
