// Copyright 2015-2017 Open Source Robotics Foundation, Inc.
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

#include "rmw_connext_shared_cpp/types.hpp"

// Uncomment this to get extra console output about discovery.
// #define DISCOVERY_DEBUG_LOGGING 1

void CustomPublisherListener::on_data_available(DDSDataReader * reader)
{
  DDSPublicationBuiltinTopicDataDataReader * builtin_reader =
    static_cast<DDSPublicationBuiltinTopicDataDataReader *>(reader);

  DDS_PublicationBuiltinTopicDataSeq data_seq;
  DDS_SampleInfoSeq info_seq;
  DDS_ReturnCode_t retcode = builtin_reader->take(
    data_seq, info_seq, DDS_LENGTH_UNLIMITED,
    DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);

  if (retcode == DDS_RETCODE_NO_DATA) {
    return;
  }
  if (retcode != DDS_RETCODE_OK) {
    fprintf(stderr, "failed to access data from the built-in reader\n");
    return;
  }

  for (auto i = 0; i < data_seq.length(); ++i) {
    if (info_seq[i].valid_data) {
      auto pub_fqdn = std::string("");
      pub_fqdn = data_seq[i].topic_name;
      add_information(
        info_seq[i].instance_handle,
        pub_fqdn,
        data_seq[i].type_name,
        EntityType::Publisher);
    } else {
      remove_information(info_seq[i].instance_handle, EntityType::Publisher);
    }
  }

  if (data_seq.length() > 0) {
    this->trigger_graph_guard_condition();
  }

  builtin_reader->return_loan(data_seq, info_seq);
}
