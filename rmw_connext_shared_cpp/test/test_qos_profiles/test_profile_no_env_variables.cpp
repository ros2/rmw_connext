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
#include <sstream>

#include "gtest/gtest.h"

#include "rcutils/get_env.h"

#include "rmw/qos_profiles.h"

#include "rmw_connext_shared_cpp/init.hpp"
#include "rmw_connext_shared_cpp/qos.hpp"
#include "rmw_connext_shared_cpp/ndds_include.hpp"

#include "./create_participant.hpp"
#include "./environment_variable_names.hpp"
#include "../custom_set_env.hpp"

namespace
{

class QosProfilesWithProfileFile : public ::testing::Test
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

TEST_F(QosProfilesWithProfileFile, test_normal_usage)
{
  DDS::DomainParticipantFactory * dpf = DDS::DomainParticipantFactory::get_instance();
  ASSERT_TRUE(dpf) << "domain participant factory is null";
  EXPECT_STREQ("Ros2TestQosLibrary", dpf->get_default_library()) <<
    "expected library name to match only provided library";
  EXPECT_FALSE(dpf->get_default_profile()) <<
    "found default profile, but no default profile was provided";

  auto * participant = create_participant();
  EXPECT_TRUE(participant);

  {
    DDS::DataReaderQos datareader_qos;
    EXPECT_TRUE(
      get_datareader_qos(
        participant, rmw_qos_profile_sensor_data, "rt/testing_rti_qos_topic", datareader_qos)
    ) << "failed to get datareader qos";
    EXPECT_EQ(DDS_BEST_EFFORT_RELIABILITY_QOS, datareader_qos.reliability.kind) <<
      "ROS specified reliability should have precedence here";
    EXPECT_EQ(
      DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS, datareader_qos.destination_order.kind
    ) << "expected destination order in qos file";
  }

  {
    DDS::DataReaderQos datareader_qos;
    EXPECT_TRUE(
      get_datareader_qos(
        participant, rmw_qos_profile_sensor_data, "rt/another_topic", datareader_qos)
    ) << "failed to get datareader qos";
    EXPECT_EQ(DDS_BEST_EFFORT_RELIABILITY_QOS, datareader_qos.reliability.kind) <<
      "ROS specified reliability should have precedence here";
    EXPECT_EQ(
      DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, datareader_qos.destination_order.kind
    ) <<
      "expected default destination order";
  }

  {
    DDS::DataWriterQos datawriter_qos;
    EXPECT_TRUE(
      get_datawriter_qos(
        participant, rmw_qos_profile_sensor_data, "rt/testing_rti_qos_topic", datawriter_qos)
    ) << "failed to get datawriter qos";
    EXPECT_EQ(DDS_BEST_EFFORT_RELIABILITY_QOS, datawriter_qos.reliability.kind) <<
      "ROS specified reliability should have precedence here";
    EXPECT_STREQ(
      "dds.flow_controller.token_bucket.slow_flow",
      datawriter_qos.publish_mode.flow_controller_name
    ) << "expected flow controller name in the QoS profile file";
  }

  {
    DDS::DataWriterQos datawriter_qos;
    EXPECT_TRUE(
      get_datawriter_qos(
        participant, rmw_qos_profile_sensor_data, "rt/another_topic", datawriter_qos)) <<
      "failed to get datawriter qos";
    EXPECT_EQ(DDS_BEST_EFFORT_RELIABILITY_QOS, datawriter_qos.reliability.kind) <<
      "ROS specified reliability should have precedence here";
    EXPECT_STRNE(
      "dds.flow_controller.token_bucket.slow_flow",
      datawriter_qos.publish_mode.flow_controller_name
    ) << "expected default flow controller name";
  }
}
