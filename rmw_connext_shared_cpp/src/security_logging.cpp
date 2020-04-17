// Copyright 2020 Canonical Ltd
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

#include <rcutils/get_env.h>
#include <rmw/error_handling.h>
#include <rmw/qos_profiles.h>
#include <rmw/types.h>

#include "rmw_connext_shared_cpp/security_logging.hpp"

namespace
{
// Environment variable names
// TODO(security-wg): These are intended to be temporary, and need to be refactored into a proper
// abstraction.
const char log_file_variable_name[] = "ROS_SECURITY_LOG_FILE";
const char log_publish_variable_name[] = "ROS_SECURITY_LOG_PUBLISH";
const char log_verbosity_variable_name[] = "ROS_SECURITY_LOG_VERBOSITY";

// Connext property names. These are being taken from table 10.1 of
// https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/manuals/connext_dds/dds_security/RTI_SecurityPlugins_GettingStarted.pdf
const char log_file_property_name[] = "com.rti.serv.secure.logging.log_file";
const char distribute_enable_property_name[] = "com.rti.serv.secure.logging.distribute.enable";
const char verbosity_property_name[] = "com.rti.serv.secure.logging.log_level";

const struct
{
  const char * const name;
  const char * const level;
} supported_verbosities[] =
{
  {"EMERGENCY", "0"},
  {"ALERT", "1"},
  {"CRITICAL", "2"},
  {"ERROR", "3"},
  {"WARNING", "4"},
  {"NOTICE", "5"},
  {"INFORMATIONAL", "6"},
  {"DEBUG", "7"},
};

bool string_to_verbosity_level(const char * str, const char ** level)
{
  for (const auto & item : supported_verbosities) {
    if (strcmp(str, item.name) == 0) {
      *level = item.level;
      return true;
    }
  }

  return false;
}

bool validate_boolean(const char * str)
{
  return (strcmp(str, "true") == 0) || (strcmp(str, "false") == 0);
}

bool apply_property(
  DDS::PropertyQosPolicy & policy, const char * const property_name,
  const char * const value)
{
  // Overwrite existing properties, so remove it if it already exists
  DDS::PropertyQosPolicyHelper::remove_property(policy, property_name);

  auto status = DDS::PropertyQosPolicyHelper::add_property(
    policy,
    property_name,
    value,
    DDS::BOOLEAN_FALSE);

  return DDS::RETCODE_OK == status;
}

bool get_env(const char * variable_name, const char ** variable_value)
{
  const char * error_message = rcutils_get_env(variable_name, variable_value);
  if (error_message != NULL) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "unable to get %s environment variable: %s",
      variable_name,
      error_message);
    return false;
  }

  return true;
}
}  // namespace

rmw_ret_t apply_security_logging_configuration(DDS::PropertyQosPolicy & policy)
{
  const char * env_value;

  // Handle logging to file
  if (!get_env(log_file_variable_name, &env_value)) {
    return RMW_RET_ERROR;
  }
  if (strlen(env_value) > 0) {
    if (!apply_property(policy, log_file_property_name, env_value)) {
      RMW_SET_ERROR_MSG("failed to set security logging file");
      return RMW_RET_ERROR;
    }
  }

  // Handle log distribution over DDS
  if (!get_env(log_publish_variable_name, &env_value)) {
    return RMW_RET_ERROR;
  }
  if (strlen(env_value) > 0) {
    if (!validate_boolean(env_value)) {
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "unsupported value for ROS_SECURITY_LOG_FILE: '%s' (use 'true' or 'false')",
        env_value);
      return RMW_RET_ERROR;
    }

    if (!apply_property(policy, distribute_enable_property_name, env_value)) {
      RMW_SET_ERROR_MSG("failed to set security logging distribute");
      return RMW_RET_ERROR;
    }
  }

  // Handle log verbosity
  if (!get_env(log_verbosity_variable_name, &env_value)) {
    return RMW_RET_ERROR;
  }
  if (strlen(env_value) > 0) {
    const char * level;
    if (!string_to_verbosity_level(env_value, &level)) {
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "failed to set security logging verbosity: %s is not a supported verbosity",
        env_value);
      return RMW_RET_ERROR;
    }

    if (!apply_property(policy, verbosity_property_name, level)) {
      RMW_SET_ERROR_MSG("failed to set security logging verbosity");
      return RMW_RET_ERROR;
    }
  }

  return RMW_RET_OK;
}
