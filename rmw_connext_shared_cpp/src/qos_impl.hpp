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
 * Return `true` if `RMW_CONNEXT_ALLOW_TOPIC_QOS_PROFILES` was set to 1 when init was called..
 *
 * The profile matching the dds topic name will be used if found, if not the default profile will be used.
 * This setting is independent from `RMW_CONNEXT_DO_NOT_OVERRIDE_PUBLICATION_MODE`.
 */
bool
are_topic_profiles_allowed();

/**
 * Return `true` if `RMW_CONNEXT_DO_NOT_OVERRIDE_PUBLICATION_MODE` was set to 1 when init was called.
 *
 * If that's the case, ROS will not set the publication mode to asynchronous, and the publication
 * mode specified in the DDS QoS profile file will be used.
 */
bool
is_publish_mode_overriden();

#endif  // QOS_IMPL_HPP_
