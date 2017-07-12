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

#include "rmw/rmw.h"
#include "rmw_connext_shared_cpp/shared_functions.hpp"

#include "identifier.hpp"

extern "C"
{
rmw_node_t *
rmw_create_node(
  const char * name, const char * namespace_, size_t domain_id,
  const rmw_node_security_options_t * security_options)
{
  return create_node(
    rti_connext_identifier, name, namespace_, domain_id, security_options);
}
}  // extern "C"
