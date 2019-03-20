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


#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"

#include "rmw_connext_shared_cpp/connext_static_event_info.hpp"
#include "rmw_connext_shared_cpp/event.hpp"
#include "rmw_connext_shared_cpp/event_converter.hpp"
#include "rmw_connext_shared_cpp/types.hpp"

rmw_ret_t
__rmw_take_event(
	const char * implementation_identifier,
  const rmw_event_t * event_handle,
  void * event,
  bool * taken)
{

	  // pointer error checking here
	  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    event handle,
    event_handle->implementation_identifier, 
    implementation_identifier,
    return RMW_RET_ERROR);

    rmw_ret_t ret_code = RMW_RET_EVENT_UNSUPPORTED;

	  //check if we support the input event type
    if(is_event_supported(event_handle->event_type)) {

      // lookup status mask from rmw_event_type
      DDS_StatusMask status_mask = get_mask_from_rmw(event_handle->event_type);

      // cast the event_handle to the appropriate type to get the appropriate
      // status from the handle
      // CustomConnextPublisher and CustomConnextSubscriber should implement this interface
      ConnextCustomEventInfo *custom_event_info =
              static_cast<ConnextCustomEventInfo *>(event_handle->data);

      // call get status with the appropriate mask
      // get_status should fill the event with the appropriate status information
      ret_code = custom_event_info->get_status(status_mask, event);
    }

    //if ret_code is not okay, return error and set taken to false.
    *taken = (ret_code == RMW_RET_OK);
    return ret_code;
}
