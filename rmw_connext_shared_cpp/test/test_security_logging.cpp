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

#include <fstream>
#include <string>

#include "rmw/error_handling.h"
#include "rmw_connext_shared_cpp/security_logging.hpp"

#include "gmock/gmock.h"

using ::testing::HasSubstr;

namespace
{
const char log_file_property_name[] = "com.rti.serv.secure.logging.log_file";
const char verbosity_property_name[] = "com.rti.serv.secure.logging.log_level";
const char distribute_enable_property_name[] = "com.rti.serv.secure.logging.distribute.enable";
const char distribute_depth_property_name[] =
  "com.rti.serv.secure.logging.distribute.writer_history_depth";

std::string write_logging_xml(const std::string & xml)
{
  // mkstemp isn't cross-platform, and we don't care about security here
  char * xml_file_path = std::tmpnam(nullptr);

  std::ofstream xml_file;
  xml_file.open(xml_file_path);
  xml_file << "<?xml version='1.0' encoding='UTF-8'?>" << std::endl;
  xml_file << "<security_log version='1'>" << std::endl;
  xml_file << xml << std::endl;
  xml_file << "</security_log>" << std::endl;
  xml_file.close();

  return xml_file_path;
}

const char * lookup_property_value(DDS::PropertyQosPolicy & policy, const char * property_name)
{
  auto property = DDS::PropertyQosPolicyHelper::lookup_property(
    policy,
    property_name);

  if (property == nullptr) {
    return nullptr;
  }

  return property->value;
}

const char * log_file_property(DDS::PropertyQosPolicy & policy)
{
  return lookup_property_value(policy, log_file_property_name);
}

const char * verbosity_property(DDS::PropertyQosPolicy & policy)
{
  return lookup_property_value(policy, verbosity_property_name);
}

const char * logging_distribute_enable_property(DDS::PropertyQosPolicy & policy)
{
  return lookup_property_value(policy, distribute_enable_property_name);
}

const char * logging_distribute_depth_property(DDS::PropertyQosPolicy & policy)
{
  return lookup_property_value(policy, distribute_depth_property_name);
}

class SecurityLoggingTest : public ::testing::Test
{
public:
  void TearDown()
  {
    rmw_reset_error();
  }
};
}  // namespace

TEST_F(SecurityLoggingTest, test_log_file)
{
  std::string xml_file_path = write_logging_xml("<file>foo</file>");
  DDS::PropertyQosPolicy policy;
  EXPECT_EQ(apply_logging_configuration_from_file(xml_file_path.c_str(), policy), RMW_RET_OK);
  EXPECT_FALSE(rmw_error_is_set());

  EXPECT_STREQ(log_file_property(policy), "foo");
  EXPECT_EQ(verbosity_property(policy), nullptr);
  EXPECT_EQ(logging_distribute_enable_property(policy), nullptr);
  EXPECT_EQ(logging_distribute_depth_property(policy), nullptr);
}

TEST_F(SecurityLoggingTest, test_log_verbosity)
{
  std::string xml_file_path = write_logging_xml("<verbosity>CRITICAL</verbosity>");
  DDS::PropertyQosPolicy policy;
  EXPECT_EQ(apply_logging_configuration_from_file(xml_file_path.c_str(), policy), RMW_RET_OK);
  EXPECT_FALSE(rmw_error_is_set());

  EXPECT_EQ(log_file_property(policy), nullptr);
  EXPECT_STREQ(verbosity_property(policy), "2");
  EXPECT_EQ(logging_distribute_enable_property(policy), nullptr);
  EXPECT_EQ(logging_distribute_depth_property(policy), nullptr);
}

TEST_F(SecurityLoggingTest, test_log_verbosity_invalid)
{
  std::string xml_file_path = write_logging_xml("<verbosity>INVALID_VERBOSITY</verbosity>");
  DDS::PropertyQosPolicy policy;
  EXPECT_EQ(apply_logging_configuration_from_file(xml_file_path.c_str(), policy), RMW_RET_ERROR);
  EXPECT_TRUE(rmw_error_is_set());
  EXPECT_THAT(
    rmw_get_error_string().str, HasSubstr(
      "INVALID_VERBOSITY is not a supported verbosity"));
}

TEST_F(SecurityLoggingTest, test_log_distribute)
{
  std::string xml_file_path = write_logging_xml("<distribute>true</distribute>");
  DDS::PropertyQosPolicy policy;
  EXPECT_EQ(apply_logging_configuration_from_file(xml_file_path.c_str(), policy), RMW_RET_OK);
  EXPECT_FALSE(rmw_error_is_set());

  EXPECT_EQ(log_file_property(policy), nullptr);
  EXPECT_EQ(verbosity_property(policy), nullptr);
  EXPECT_STREQ(logging_distribute_enable_property(policy), "true");
  EXPECT_EQ(logging_distribute_depth_property(policy), nullptr);
}

TEST_F(SecurityLoggingTest, test_log_depth)
{
  std::string xml_file_path = write_logging_xml("<qos><depth>10</depth></qos>");
  DDS::PropertyQosPolicy policy;
  EXPECT_EQ(apply_logging_configuration_from_file(xml_file_path.c_str(), policy), RMW_RET_OK);
  EXPECT_FALSE(rmw_error_is_set());

  EXPECT_EQ(log_file_property(policy), nullptr);
  EXPECT_EQ(verbosity_property(policy), nullptr);
  EXPECT_EQ(logging_distribute_enable_property(policy), nullptr);
  EXPECT_STREQ(logging_distribute_depth_property(policy), "10");
}

TEST_F(SecurityLoggingTest, test_profile)
{
  std::string xml_file_path = write_logging_xml("<qos><profile>DEFAULT</profile></qos>");
  DDS::PropertyQosPolicy policy;
  EXPECT_EQ(apply_logging_configuration_from_file(xml_file_path.c_str(), policy), RMW_RET_OK);
  EXPECT_FALSE(rmw_error_is_set());

  EXPECT_EQ(log_file_property(policy), nullptr);
  EXPECT_EQ(verbosity_property(policy), nullptr);
  EXPECT_EQ(logging_distribute_enable_property(policy), nullptr);
  EXPECT_STREQ(logging_distribute_depth_property(policy), "10");
}

TEST_F(SecurityLoggingTest, test_profile_overwrite)
{
  std::string xml_file_path = write_logging_xml(
    "<qos>\n"
    "  <profile>DEFAULT</profile>\n"
    "  <depth>42</depth>\n"
    "</qos>");
  DDS::PropertyQosPolicy policy;
  EXPECT_EQ(apply_logging_configuration_from_file(xml_file_path.c_str(), policy), RMW_RET_OK);
  EXPECT_FALSE(rmw_error_is_set());

  EXPECT_EQ(log_file_property(policy), nullptr);
  EXPECT_EQ(verbosity_property(policy), nullptr);
  EXPECT_EQ(logging_distribute_enable_property(policy), nullptr);
  EXPECT_STREQ(logging_distribute_depth_property(policy), "42");
}

TEST_F(SecurityLoggingTest, test_profile_invalid)
{
  std::string xml_file_path = write_logging_xml("<qos><profile>INVALID_PROFILE</profile></qos>");
  DDS::PropertyQosPolicy policy;
  EXPECT_EQ(apply_logging_configuration_from_file(xml_file_path.c_str(), policy), RMW_RET_ERROR);
  EXPECT_TRUE(rmw_error_is_set());
  EXPECT_THAT(rmw_get_error_string().str, HasSubstr("INVALID_PROFILE is not a supported profile"));
}

TEST_F(SecurityLoggingTest, test_all)
{
  std::string xml_file_path = write_logging_xml(
    "<file>foo</file>\n"
    "<verbosity>ERROR</verbosity>\n"
    "<distribute>true</distribute>\n"
    "<qos>\n"
    "  <depth>10</depth>\n"
    "</qos>");
  DDS::PropertyQosPolicy policy;
  EXPECT_EQ(apply_logging_configuration_from_file(xml_file_path.c_str(), policy), RMW_RET_OK);
  EXPECT_FALSE(rmw_error_is_set());

  EXPECT_STREQ(log_file_property(policy), "foo");
  EXPECT_STREQ(verbosity_property(policy), "3");
  EXPECT_STREQ(logging_distribute_enable_property(policy), "true");
  EXPECT_STREQ(logging_distribute_depth_property(policy), "10");
}
