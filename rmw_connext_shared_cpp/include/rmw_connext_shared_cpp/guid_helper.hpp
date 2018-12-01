// Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0
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

#ifndef RMW_CONNEXT_CPP_GUID_HPP
#define RMW_CONNEXT_CPP_GUID_HPP

#include <cstring>
#include <iostream>
#include "ndds/ndds_cpp.h"
#include "ndds/ndds_namespace_cpp.h"


inline bool operator==(const DDS_GUID_t & lhs, const DDS_GUID_t & rhs)
{
  /// http://community.rti.com/rti-doc/500/ndds.5.0.0/doc/html/api_cpp/group__DDSGUIDSupportModule.html#
  return DDS_BOOLEAN_TRUE == DDS_GUID_equals(&lhs, &rhs);
}

inline bool operator!=(const DDS_GUID_t & lhs, const DDS_GUID_t & rhs) { return !operator==(lhs, rhs); }

inline bool operator< (const DDS_GUID_t & lhs, const DDS_GUID_t & rhs)
{
  /// http://community.rti.com/rti-doc/500/ndds.5.0.0/doc/html/api_cpp/group__DDSGUIDSupportModule.html#
  return DDS_GUID_compare(&lhs, &rhs) < 0;
}

inline bool operator> (const DDS_GUID_t & lhs, const DDS_GUID_t & rhs) { return  operator< (rhs, lhs); }
inline bool operator<=(const DDS_GUID_t & lhs, const DDS_GUID_t & rhs) { return !operator> (lhs, rhs); }
inline bool operator>=(const DDS_GUID_t & lhs, const DDS_GUID_t & rhs) { return !operator< (lhs, rhs); }


inline std::ostream &operator<<(std::ostream &output, const DDS_GUID_t & guiP) {
  output << std::hex;
  for (uint8_t i = 0; i < 11; ++i) {
    output << (int) guiP.value[i] << ".";
  }
  output << (int) guiP.value[11];
  return output << std::dec;
}


inline void DDS_InstanceHandle_to_GUID(DDS_GUID_t * guid, DDS_InstanceHandle_t instanceHandle)
{
  memcpy(guid->value, reinterpret_cast<DDS_Octet const *>(&instanceHandle), 16);
}

/** Taken from http://community.rti.com/comment/689#comment-689 */
inline void DDS_BuiltinTopicKey_to_GUID(DDS_GUID_t * guid, DDS_BuiltinTopicKey_t builtinTopicKey)
{
#if BIG_ENDIAN
  memcpy(guid->value, reinterpret_cast<DDS_Octet const *>(&builtinTopicKey), 16);
#else
  /* Little Endian */
  DDS_Octet * guidElement
  DDS_Octet * keyBufferElement;
  for (uint i = 0; i < 3; ++i) {
    DDS_Octet * guidElement = &(guid->value[i * 3]);
    DDS_Octet * keyBufferElement  = (DDS_Octet *)(&buitinTopicKey[i * 3]);
    guidElement[0] = keyBufferElement[2];
    guidElement[1] = keyBufferElement[1];
    guidElement[2] = keyBufferElement[0];
  }
#endif
}

#endif //RMW_OPENSPLICE_CPP_GUID_HPP
