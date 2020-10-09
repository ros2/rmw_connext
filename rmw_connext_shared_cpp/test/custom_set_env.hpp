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

#ifndef CUSTOM_SET_ENV_HPP_
#define CUSTOM_SET_ENV_HPP_

#include <string>

#include "gtest/gtest.h"

static
void
custom_setenv(const std::string & variable_name, const std::string & value)
{
#ifdef _WIN32
  auto ret = _putenv_s(variable_name.c_str(), value.c_str());
#else
  auto ret = setenv(variable_name.c_str(), value.c_str(), 1);
#endif
  if (ret != 0) {
    ADD_FAILURE() << "Unable to set environment variable: expected 0, got " << ret;
  }
}

#endif  // CUSTOM_SET_ENV_HPP_
