// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include <string>

#include "gtest/gtest.h"

#include "rmw_connext_shared_cpp/init.hpp"
#include "rmw_connext_shared_cpp/ndds_include.hpp"

// In src folder
#include "qos_impl.hpp"

#include "./environment_variable_names.hpp"
#include "../custom_set_env.hpp"

namespace
{

class QosProfilesTestNoProfileFile : public ::testing::Test
{
public:
  void SetUp()
  {
    custom_setenv(allow_topic_qos_vn, "");
    custom_setenv(do_not_override_pub_mode_vn, "");
    custom_setenv(profile_library_vn, "");
    custom_setenv(default_qos_profile_vn, "");
    init();
  }

  void TearDown()
  {
  }
};
}  // namespace

TEST_F(QosProfilesTestNoProfileFile, test_normal_usage)
{
  EXPECT_TRUE(rmw_connext_shared_cpp::is_publish_mode_overriden()) <<
    "env variable '" << do_not_override_pub_mode_vn << "' was not set";
  EXPECT_FALSE(rmw_connext_shared_cpp::are_topic_profiles_allowed()) <<
    "env variable '" << allow_topic_qos_vn << "' was not set";

  DDS::DomainParticipantFactory * dpf = DDS::DomainParticipantFactory::get_instance();
  ASSERT_TRUE(dpf) << "domain participant factory is null";
  EXPECT_FALSE(dpf->get_default_library()) <<
    "expected library name to be null";
  EXPECT_FALSE(dpf->get_default_profile()) <<
    "found default profile to be null";
}
