// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#ifndef MACROS_HPP_
#define MACROS_HPP_

#include <limits>
#include <string>

// Two different kinds of concatenation are needed for these namespaces
#define INTROSPECTION_CPP_TYPE(A) rosidl_typesupport_introspection_cpp::A

#define INTROSPECTION_C_TYPE(A) rosidl_typesupport_introspection_c__ ## A

#define C_STRING_ASSIGN(A, B) rosidl_generator_c__String__assignn(&A, B, size - 1);

#define CPP_STRING_ASSIGN(A, B) A = B;

#endif  // MACROS_HPP_
