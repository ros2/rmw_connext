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

#ifndef QOS_IMPL_HPP_
#define QOS_IMPL_HPP_

// NOLINT, link in docblock is too long.
/**
 * Return `true` if `RMW_CONNEXT_IGNORE_ROS_QOS` was set to 1.
 *
 * In this case, the ros provided qos profile is completely ignored.
 *
 * The profile matching the fully qualified node name will be used if `does_node_profile_override()` is true.
 * If not the default profile will be used.
 *
 * Topic filters are used both for the profile matching the node name and the default profile, see:
 * \ref https://community.rti.com/static/documentation/connext-dds/5.2.0/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_UsersManual/Content/UsersManual/Topic_Filters.htm
 *
 * Prefer using `RMW_CONNEXT_NODE_QOS_PROFILE_OVERRIDE` instead of this, which is less error prone.
 * See \ref does_node_profile_override().
 *
 * By modifying QoS profiles provided in code you may be breaking contracts the developer assumed.
 * Use only if you know whay you're doing.
 */
bool
is_ros_qos_ignored();

// NOLINT, link in docblock is too long.
/**
 * Return `true` if `RMW_CONNEXT_NODE_QOS_PROFILE_OVERRIDE` was set to 1.
 *
 * The profile matching the fully qualified node name will be used if found.
 *
 * Topic filters are used, see:
 * \ref https://community.rti.com/static/documentation/connext-dds/5.2.0/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_UsersManual/Content/UsersManual/Topic_Filters.htm
 *
 * By modifying QoS profiles provided in code you may be breaking contracts the developer assumed.
 * Use only if you know whay you're doing.
 */
bool
does_node_profile_override();

#endif  // QOS_IMPL_HPP_
