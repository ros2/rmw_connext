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

#ifndef TEST_QOS_PROFILES__CREATE_PARTICIPANT_HPP_
#define TEST_QOS_PROFILES__CREATE_PARTICIPANT_HPP_

static
DDS::DomainParticipant *
create_participant()
{
  const char * domain_id_var_val = nullptr;
  const char * error = rcutils_get_env("ROS_DOMAIN_ID", &domain_id_var_val);

  if (error) {
    ADD_FAILURE() << "failed to get environment variable: " << error;
    return nullptr;
  }

  DDS::DomainId_t domain_id;
  std::istringstream iss(domain_id_var_val);
  iss >> domain_id;
  if (!iss.good()) {  // env variable not set or wrong
    domain_id = 0;
  }

  DDS::DomainParticipantFactory * dpf = DDS::DomainParticipantFactory::get_instance();
  if (!dpf) {
    ADD_FAILURE() << "domain participant factory is null";
    return nullptr;
  }

  DDS::DomainParticipantQos participant_qos;
  DDS::ReturnCode_t status = dpf->get_default_participant_qos(participant_qos);
  if (DDS::RETCODE_OK != status) {
    ADD_FAILURE() << "failed to get participant qos";
    return nullptr;
  }
  DDS::DomainParticipant * participant = dpf->create_participant(
    domain_id,
    participant_qos,
    NULL,
    DDS::STATUS_MASK_NONE);
  if (!participant) {
    ADD_FAILURE() << "failed to create domain participant";
  }
  return participant;
}

#endif  // TEST_QOS_PROFILES__CREATE_PARTICIPANT_HPP_
