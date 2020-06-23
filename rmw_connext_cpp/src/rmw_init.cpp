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

#include "rmw/init.h"

#include "rcutils/strdup.h"

#include "rmw/impl/cpp/macros.hpp"
#include "rmw_connext_shared_cpp/init.hpp"

#include "rmw_connext_cpp/identifier.hpp"

struct rmw_context_impl_t
{
  // Shutdown flag
  bool is_shutdown{false};
};

extern "C"
{
rmw_ret_t
rmw_init_options_init(rmw_init_options_t * init_options, rcutils_allocator_t allocator)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(init_options, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR(&allocator, return RMW_RET_INVALID_ARGUMENT);
  if (NULL != init_options->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected zero-initialized init_options");
    return RMW_RET_INVALID_ARGUMENT;
  }
  init_options->instance_id = 0;
  init_options->implementation_identifier = rti_connext_identifier;
  init_options->allocator = allocator;
  init_options->impl = nullptr;
  init_options->security_options = rmw_get_zero_initialized_security_options();
  init_options->domain_id = RMW_DEFAULT_DOMAIN_ID;
  init_options->localhost_only = RMW_LOCALHOST_ONLY_DEFAULT;
  init_options->enclave = NULL;
  return RMW_RET_OK;
}

rmw_ret_t
rmw_init_options_copy(const rmw_init_options_t * src, rmw_init_options_t * dst)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(src, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(dst, RMW_RET_INVALID_ARGUMENT);
  if (NULL == src->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected initialized src");
    return RMW_RET_INVALID_ARGUMENT;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    src,
    src->implementation_identifier,
    rti_connext_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  if (NULL != dst->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected zero-initialized dst");
    return RMW_RET_INVALID_ARGUMENT;
  }
  const rcutils_allocator_t * allocator = &src->allocator;
  RCUTILS_CHECK_ALLOCATOR(allocator, return RMW_RET_INVALID_ARGUMENT);

  rmw_init_options_t tmp = *src;
  tmp.enclave = rcutils_strdup(src->enclave, *allocator);
  if (NULL != src->enclave && NULL == tmp.enclave) {
    return RMW_RET_BAD_ALLOC;
  }
  tmp.security_options = rmw_get_zero_initialized_security_options();
  rmw_ret_t ret =
    rmw_security_options_copy(&src->security_options, allocator, &tmp.security_options);
  if (RMW_RET_OK != ret) {
    allocator->deallocate(tmp.enclave, allocator->state);
    return ret;
  }
  *dst = tmp;
  return RMW_RET_OK;
}

rmw_ret_t
rmw_init_options_fini(rmw_init_options_t * init_options)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(init_options, RMW_RET_INVALID_ARGUMENT);
  if (NULL == init_options->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected initialized init_options");
    return RMW_RET_INVALID_ARGUMENT;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    init_options,
    init_options->implementation_identifier,
    rti_connext_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  const rcutils_allocator_t * allocator = &init_options->allocator;
  RCUTILS_CHECK_ALLOCATOR(allocator, return RMW_RET_INVALID_ARGUMENT);

  rmw_ret_t ret = rmw_security_options_fini(&init_options->security_options, allocator);
  if (RMW_RET_OK == ret) {
    allocator->deallocate(init_options->enclave, allocator->state);
    *init_options = rmw_get_zero_initialized_init_options();
  }
  return ret;
}

rmw_ret_t
rmw_init(const rmw_init_options_t * options, rmw_context_t * context)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(options, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    options,
    options->implementation_identifier,
    rti_connext_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  context->instance_id = options->instance_id;
  context->implementation_identifier = rti_connext_identifier;
  context->impl = new (std::nothrow) rmw_context_impl_t();
  if (!context->impl) {
    RMW_SET_ERROR_MSG("failed to allocate context impl");
    return RMW_RET_BAD_ALLOC;
  }
  context->impl->is_shutdown = false;
  rmw_ret_t ret = rmw_init_options_copy(options, &context->options);
  if (RMW_RET_OK != ret) {
    delete context->impl;
    return ret;
  }
  ret = init();
  if (RMW_RET_OK != ret) {
    if (RMW_RET_OK != rmw_init_options_fini(&context->options)) {
      RMW_SAFE_FWRITE_TO_STDERR(
        "'rmw_init_options_fini' failed while being executed due to '"
        RCUTILS_STRINGIFY(__function__) "' failing.\n");
    }
    delete context->impl;
  }
  return ret;
}

rmw_ret_t
rmw_shutdown(rmw_context_t * context)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    context,
    context->implementation_identifier,
    rti_connext_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context->impl, RMW_RET_INVALID_ARGUMENT);
  context->impl->is_shutdown = true;
  return RMW_RET_OK;
}

rmw_ret_t
rmw_context_fini(rmw_context_t * context)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    context,
    context->implementation_identifier,
    rti_connext_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context->impl, RMW_RET_INVALID_ARGUMENT);
  if (!context->impl->is_shutdown) {
    RCUTILS_SET_ERROR_MSG("context has not been shutdown");
    return RMW_RET_INVALID_ARGUMENT;
  }
  delete context->impl;
  *context = rmw_get_zero_initialized_context();
  return RMW_RET_OK;
}
}  // extern "C"
