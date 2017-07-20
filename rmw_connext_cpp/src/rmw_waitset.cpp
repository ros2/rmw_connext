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

#include "rmw_connext_shared_cpp/waitset.hpp"

#include "identifier.hpp"

extern "C"
{
rmw_waitset_t *
rmw_create_waitset(size_t max_conditions)
{
  return create_waitset(rti_connext_identifier, max_conditions);
}

rmw_ret_t
rmw_destroy_waitset(rmw_waitset_t * waitset)
{
  return destroy_waitset(rti_connext_identifier, waitset);
}
}  // extern "C"
