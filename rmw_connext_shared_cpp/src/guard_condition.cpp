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

#include "rmw_connext_shared_cpp/guard_condition.hpp"
#include "rmw_connext_shared_cpp/ndds_include.hpp"

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"

rmw_guard_condition_t *
create_guard_condition(const char * implementation_identifier)
{
  rmw_guard_condition_t * guard_condition = rmw_guard_condition_allocate();
  if (!guard_condition) {
    RMW_SET_ERROR_MSG("failed to allocate guard condition");
    return NULL;
  }
  // Allocate memory for the DDSGuardCondition object.
  DDSGuardCondition * dds_guard_condition = nullptr;
  void * buf = rmw_allocate(sizeof(DDSGuardCondition));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the DDSGuardCondition in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(dds_guard_condition, buf, goto fail, DDSGuardCondition, )
  buf = nullptr;  // Only free the dds_guard_condition pointer; don't need the buf pointer anymore.
  guard_condition->implementation_identifier = implementation_identifier;
  guard_condition->data = dds_guard_condition;
  return guard_condition;
fail:
  if (guard_condition) {
    rmw_guard_condition_free(guard_condition);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

rmw_ret_t
destroy_guard_condition(
  const char * implementation_identifier,
  rmw_guard_condition_t * guard_condition)
{
  if (!guard_condition) {
    RMW_SET_ERROR_MSG("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    guard condition handle,
    guard_condition->implementation_identifier, implementation_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  RMW_TRY_DESTRUCTOR(
    static_cast<DDSGuardCondition *>(guard_condition->data)->~DDSGuardCondition(),
    DDSGuardCondition, result = RMW_RET_ERROR)
  rmw_free(guard_condition->data);
  rmw_guard_condition_free(guard_condition);
  return result;
}
