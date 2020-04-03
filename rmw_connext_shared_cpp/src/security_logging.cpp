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

#include <tinyxml2.h>

#include <rcutils/snprintf.h>
#include <rmw/error_handling.h>
#include <rmw/qos_profiles.h>
#include <rmw/types.h>

#include "rmw_connext_shared_cpp/security_logging.hpp"

namespace
{
// These properties are being taken from table 10.1 of
// https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/manuals/connext_dds/dds_security/RTI_SecurityPlugins_GettingStarted.pdf
const char log_file_property_name[] = "com.rti.serv.secure.logging.log_file";
const char verbosity_property_name[] = "com.rti.serv.secure.logging.log_level";
const char distribute_enable_property_name[] = "com.rti.serv.secure.logging.distribute.enable";
const char distribute_depth_property_name[] =
  "com.rti.serv.secure.logging.distribute.writer_history_depth";

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

const struct
{
  const char * const name;
  rmw_qos_profile_t profile;
} supported_profiles[] =
{
  {"SENSOR_DATA", rmw_qos_profile_sensor_data},
  {"PARAMETERS", rmw_qos_profile_parameters},
  {"DEFAULT", rmw_qos_profile_default},
  {"SERVICES_DEFAULT", rmw_qos_profile_services_default},
  {"PARAMETER_EVENTS", rmw_qos_profile_parameter_events},
  {"SYSTEM_DEFAULT", rmw_qos_profile_system_default},
};

bool string_to_rmw_qos_profile(const char * str, rmw_qos_profile_t & profile)
{
  for (const auto & item : supported_profiles) {
    if (strcmp(str, item.name) == 0) {
      profile = item.profile;
      return true;
    }
  }

  return false;
}

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

bool get_element_text(
  const tinyxml2::XMLElement & element,
  const char * const tag_name,
  const char ** text)
{
  *text = element.GetText();
  if (*text == nullptr) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "failed to set security logging %s: improper format",
      tag_name);
    return false;
  }

  return true;
}

bool apply_property(
  DDS::PropertyQosPolicy & policy, const char * const property_name,
  const char * const human_readable_property_name, const char * const value)
{
  // Overwrite existing properties, so remove it if it already exists
  DDS::PropertyQosPolicyHelper::remove_property(policy, property_name);

  auto status = DDS::PropertyQosPolicyHelper::add_property(
    policy,
    property_name,
    value,
    DDS::BOOLEAN_FALSE);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "failed to set security logging %s",
      human_readable_property_name);
    return false;
  }

  return true;
}

bool apply_property_from_element(
  DDS::PropertyQosPolicy & policy, const char * const property_name,
  const tinyxml2::XMLElement & element, const char * const tag_name)
{
  auto tag = element.FirstChildElement(tag_name);
  if (tag != nullptr) {
    const char * text;
    if (!get_element_text(*tag, tag_name, &text)) {
      return false;
    }

    return apply_property(policy, property_name, tag_name, text);
  }

  return true;
}

bool apply_qos_profile(DDS::PropertyQosPolicy & policy, const rmw_qos_profile_t & profile)
{
  char depth_str[256];
  int max_length = sizeof(depth_str);
  int resulting_length = rcutils_snprintf(depth_str, max_length, "%zu", profile.depth);
  if (resulting_length < 0 || resulting_length >= max_length) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "failed to set security logging depth from profile: unable to convert %zu to string",
      profile.depth);
    return false;
  }

  return apply_property(policy, distribute_depth_property_name, "depth", depth_str);
}
}  // namespace

rmw_ret_t apply_logging_configuration_from_file(
  const char * xml_file_path,
  DDS::PropertyQosPolicy & policy)
{
  tinyxml2::XMLDocument document;
  document.LoadFile(xml_file_path);

  auto log_element = document.FirstChildElement("security_log");
  if (log_element == nullptr) {
    RMW_SET_ERROR_MSG("logger xml file missing 'security_log'");
    return RMW_RET_ERROR;
  }

  bool status = apply_property_from_element(
    policy,
    log_file_property_name,
    *log_element,
    "file");
  if (!status) {
    return RMW_RET_ERROR;
  }

  status = apply_property_from_element(
    policy,
    verbosity_property_name,
    *log_element,
    "verbosity");
  if (!status) {
    return RMW_RET_ERROR;
  }

  auto verbosity_element = log_element->FirstChildElement("verbosity");
  if (verbosity_element != nullptr) {
    const char * verbosity_str;
    if (!get_element_text(*verbosity_element, "verbosity", &verbosity_str)) {
      return RMW_RET_ERROR;
    }

    const char * level;
    if (!string_to_verbosity_level(verbosity_str, &level)) {
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "failed to set security logging verbosity: %s is not a supported verbosity",
        verbosity_str);
      return RMW_RET_ERROR;
    }

    if (!apply_property(policy, verbosity_property_name, "verbosity", level)) {
      return RMW_RET_ERROR;
    }
  }

  auto publish_element = log_element->FirstChildElement("publish");
  if (publish_element != nullptr) {
    // The presence of this element indicates that the log should be distributed
    if (!apply_property(policy, distribute_enable_property_name, "distribute", "true")) {
      return RMW_RET_ERROR;
    }

    auto qos_element = publish_element->FirstChildElement("qos");
    if (qos_element != nullptr) {
      // First thing we need to do is apply any QoS profile that was specified.
      // Once that has happened, further settings can be applied to customize.
      auto profile_element = qos_element->FirstChildElement("profile");
      if (profile_element != nullptr) {
        const char * profile_str;
        if (!get_element_text(*profile_element, "profile", &profile_str)) {
          return RMW_RET_ERROR;
        }

        rmw_qos_profile_t profile;
        if (!string_to_rmw_qos_profile(profile_str, profile)) {
          RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "failed to set security logging profile: %s is not a supported profile",
            profile_str);
          return RMW_RET_ERROR;
        }

        status = apply_qos_profile(policy, profile);
        if (!status) {
          return RMW_RET_ERROR;
        }
      }

      status = apply_property_from_element(
        policy,
        distribute_depth_property_name,
        *qos_element,
        "depth");
      if (!status) {
        return RMW_RET_ERROR;
      }
    }
  }

  return RMW_RET_OK;
}
