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

#include "rmw_connext_shared_cpp/init.hpp"

#include "rcutils/get_env.h"
#include "rmw/error_handling.h"

#include "rmw_connext_shared_cpp/ndds_include.hpp"

static std::once_flag g_run_once_flag;

/// Tri-state retcode used in `set_default_qos_library`.
enum class SetDefaultLibraryRet {SET, NOT_SET, FAILED};

/// Set the default Connext qos profile library.
/**
 * If `RMW_CONNEXT_QOS_PROFILE_LIBRARY` environment variable is set,
 * the library matching that name is used.
 * If only one user provided library is found, that library is set as the default one.
 * If not, not qos profile library is set.
 *
 * \return `SetDefaultLibraryRet::SET` if the default library was set, or
 * \return `SetDefaultLibraryRet::NOT_SET` if the default library was not set, or
 * \return `SetDefaultLibraryRet::FAILED` if failed to read the environment variable
 *  or to set the default library.
 */
static SetDefaultLibraryRet
set_default_qos_library(DDS::DomainParticipantFactory * dpf);

/// Set the default Connext qos profile.
/**
 * If `RMW_CONNEXT_DEFAULT_QOS_PROFILE`, that profiles is set as the default.
 * It uses the globally set default library.
 *
 * \return false if failed to get the environment variable or to set the default profile,
 *  else false.
 */
static bool
set_default_qos_profile(DDS::DomainParticipantFactory * dpf);

rmw_ret_t
init()
{
  rmw_ret_t ret = RMW_RET_OK;
  std::call_once(
    g_run_once_flag, [&ret]() {
      DDS::DomainParticipantFactory * dpf = DDS::DomainParticipantFactory::get_instance();
      if (!dpf) {
        RMW_SET_ERROR_MSG("failed to get participant factory");
        ret = RMW_RET_ERROR;
        return;
      }
      DDS::DomainParticipantFactoryQos factory_qos;
      // TODO(ivanpauno): The default is 1024, this should be configurable.
      // See https://github.com/ros2/rmw_connext/pull/394.
      dpf->get_qos(factory_qos);
      factory_qos.resource_limits.max_objects_per_thread = 8192;
      dpf->set_qos(factory_qos);

      auto set_library_ret = set_default_qos_library(dpf);
      if (SetDefaultLibraryRet::FAILED == set_library_ret) {
        ret = RMW_RET_ERROR;
        return;
      }
      if (SetDefaultLibraryRet::SET == set_library_ret) {
        set_default_qos_profile(dpf);
      }
    }
  );
  return ret;
}

static SetDefaultLibraryRet
set_default_qos_library(DDS::DomainParticipantFactory * dpf)
{
  const char * qos_profile_library_name = NULL;
  const char * error = rcutils_get_env(
    "RMW_CONNEXT_QOS_PROFILE_LIBRARY", &qos_profile_library_name);
  if (error) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("rcutils_get_env() failed: '%s'", error);
    return SetDefaultLibraryRet::FAILED;
  }

  if (qos_profile_library_name && 0 == strcmp("", qos_profile_library_name)) {
    return SetDefaultLibraryRet::NOT_SET;
  }
  if (DDS::RETCODE_OK != dpf->set_default_library(qos_profile_library_name)) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "failed to set default library \"%s\"", qos_profile_library_name);
    return SetDefaultLibraryRet::FAILED;
  }
  return SetDefaultLibraryRet::SET;
}

static bool
set_default_qos_profile(DDS::DomainParticipantFactory * dpf)
{
  const char * default_qos_profile = NULL;
  const char * error = rcutils_get_env(
    "RMW_CONNEXT_DEFAULT_QOS_PROFILE", &default_qos_profile);
  if (error) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("rcutils_get_env() failed: '%s'", error);
    return false;
  }
  if (default_qos_profile && 0 == strcmp("", default_qos_profile)) {
    return true;
  }

  const char * default_library = dpf->get_default_library();
  if (DDS::RETCODE_OK != dpf->set_default_profile(default_library, default_qos_profile)) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "failed to set the default qos profile, library=\"%s\", profile=\"%s\"",
      default_library, default_qos_profile);
    return false;
  }
  return true;
}
