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

#include "rmw_connext_shared_cpp/shared_functions.hpp"

rmw_waitset_t *
create_waitset(const char * implementation_identifier, size_t max_conditions)
{
  rmw_waitset_t * waitset = rmw_waitset_allocate();

  ConnextWaitSetInfo * waitset_info = nullptr;

  // From here onward, error results in unrolling in the goto fail block.
  if (!waitset) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }
  waitset->implementation_identifier = implementation_identifier;
  waitset->data = rmw_allocate(sizeof(ConnextWaitSetInfo));
  waitset_info = static_cast<ConnextWaitSetInfo *>(waitset->data);

  if (!waitset_info) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }

  waitset_info->waitset = static_cast<DDSWaitSet *>(rmw_allocate(sizeof(DDSWaitSet)));
  if (!waitset_info->waitset) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }

  RMW_TRY_PLACEMENT_NEW(
    waitset_info->waitset, waitset_info->waitset, goto fail, DDSWaitSet, )

  // Now allocate storage for the ConditionSeq objects
  waitset_info->active_conditions =
    static_cast<DDSConditionSeq *>(rmw_allocate(sizeof(DDSConditionSeq)));
  if (!waitset_info->active_conditions) {
    RMW_SET_ERROR_MSG("failed to allocate active_conditions sequence");
    goto fail;
  }

  waitset_info->attached_conditions =
    static_cast<DDSConditionSeq *>(rmw_allocate(sizeof(DDSConditionSeq)));
  if (!waitset_info->attached_conditions) {
    RMW_SET_ERROR_MSG("failed to allocate attached_conditions sequence");
    goto fail;
  }

  // If max_conditions is greater than zero, re-allocate both ConditionSeqs to max_conditions
  if (max_conditions > 0) {
    RMW_TRY_PLACEMENT_NEW(
      waitset_info->active_conditions, waitset_info->active_conditions, goto fail,
      DDSConditionSeq, static_cast<DDS_Long>(max_conditions))

    RMW_TRY_PLACEMENT_NEW(
      waitset_info->attached_conditions, waitset_info->attached_conditions, goto fail,
      DDSConditionSeq,
      static_cast<DDS_Long>(max_conditions))
  } else {
    // Else, don't preallocate: the vectors will size dynamically when rmw_wait is called.
    // Default-construct the ConditionSeqs.
    RMW_TRY_PLACEMENT_NEW(
      waitset_info->active_conditions, waitset_info->active_conditions,
      goto fail, DDSConditionSeq, )

    RMW_TRY_PLACEMENT_NEW(
      waitset_info->attached_conditions, waitset_info->attached_conditions, goto fail,
      DDSConditionSeq, )
  }

  return waitset;

fail:
  if (waitset_info) {
    if (waitset_info->active_conditions) {
      // How to know which constructor threw?
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
        waitset_info->active_conditions->~DDSConditionSeq(), DDSConditionSeq)
      rmw_free(waitset_info->active_conditions);
    }
    if (waitset_info->attached_conditions) {
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
        waitset_info->attached_conditions->~DDSConditionSeq(), DDSConditionSeq)
      rmw_free(waitset_info->attached_conditions);
    }
    if (waitset_info->waitset) {
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(waitset_info->waitset->~DDSWaitSet(), DDSWaitSet)
      rmw_free(waitset_info->waitset);
    }
    waitset_info = nullptr;
  }
  if (waitset) {
    if (waitset->data) {
      rmw_free(waitset->data);
    }
    rmw_waitset_free(waitset);
  }
  return nullptr;
}

rmw_ret_t
destroy_waitset(const char * implementation_identifier, rmw_waitset_t * waitset)
{
  if (!waitset) {
    RMW_SET_ERROR_MSG("waitset handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    waitset handle,
    waitset->implementation_identifier, implementation_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  ConnextWaitSetInfo * waitset_info = static_cast<ConnextWaitSetInfo *>(waitset->data);

  // Explicitly call destructor since the "placement new" was used
  if (waitset_info->active_conditions) {
    RMW_TRY_DESTRUCTOR(
      waitset_info->active_conditions->~DDSConditionSeq(), ConditionSeq, result = RMW_RET_ERROR)
    rmw_free(waitset_info->active_conditions);
  }
  if (waitset_info->attached_conditions) {
    RMW_TRY_DESTRUCTOR(
      waitset_info->attached_conditions->~DDSConditionSeq(), ConditionSeq, result = RMW_RET_ERROR)
    rmw_free(waitset_info->attached_conditions);
  }
  if (waitset_info->waitset) {
    RMW_TRY_DESTRUCTOR(waitset_info->waitset->~DDSWaitSet(), WaitSet, result = RMW_RET_ERROR)
    rmw_free(waitset_info->waitset);
  }
  waitset_info = nullptr;
  if (waitset->data) {
    rmw_free(waitset->data);
  }
  if (waitset) {
    rmw_waitset_free(waitset);
  }
  return result;
}
