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

#ifndef RMW_CONNEXT_CPP_TOPIC_CACHE_H
#define RMW_CONNEXT_CPP_TOPIC_CACHE_H


#include <unordered_map>
#include <utility>
#include <set>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>

#include "rcutils/logging_macros.h"

#include "ndds/ndds_cpp.h"
#include "ndds/ndds_namespace_cpp.h"


/**
 * Topic cache data structure. Manages relationships between participants and topics.
 */
template<typename GUID_t>
class TopicCache{
private:
  typedef std::unordered_map<std::string, std::vector<std::string>> TopicToTypes;
  typedef std::map<GUID_t, TopicToTypes> ParticipantTopicMap;

  /**
   * Map of topic names to a vector of types that topic may use.
   * Topics here are represented as one to many, DDS XTypes 1.2
   * specifies application code 'generally' uses a 1-1 relationship.
   * However, generic services such as logger and monitor, can discover
   * multiple types on the same topic.
   *
   */
  TopicToTypes topic_to_types_;

  /**
   * Map of participant GUIDS to a set of topic-type.
   */
  ParticipantTopicMap participant_to_topics_;

  /**
   * Helper function to initialize a topic vector.
   *
   * @param topic_name
   */
  void initializeTopic(const std::string & topic_name, TopicToTypes & topic_to_types)
  {
    if (topic_to_types.find(topic_name) == topic_to_types.end()) {
      topic_to_types[topic_name] = std::vector<std::string>();
    }
  }

  /**
   * Helper function to initialize the set inside a participant map.
   *
   * @param map
   * @param guid
   */
  void initializeParticipantMap(ParticipantTopicMap & map,
                                GUID_t guid)
  {
    if (map.find(guid) == map.end()) {
      map[guid] = TopicToTypes();
    }
  }

public:
  /**
   * @return a map of topic name to the vector of topic types used.
   */
  const TopicToTypes & getTopicToTypes() const
  {
    return topic_to_types_;
  }

  /**
   * @return a map of participant guid to the vector of topic names used.
   */
  const ParticipantTopicMap & getParticipantToTopics() const
  {
    return participant_to_topics_;
  }

  /**
   * Add a topic based on discovery.
   *
   * @param guid
   * @param topic_name
   * @param type_name
   * @return true if a change has been recorded
   */
  bool addTopic(const GUID_t & guid,
                const std::string & topic_name,
                const std::string & type_name)
  {
    initializeTopic(topic_name, topic_to_types_);
    initializeParticipantMap(participant_to_topics_, guid);
    initializeTopic(topic_name, participant_to_topics_[guid]);
    if (rcutils_logging_logger_is_enabled_for("rmw_connext_shared_cpp", RCUTILS_LOG_SEVERITY_DEBUG)) {
      std::stringstream guid_stream;
      guid_stream << guid;
      RCUTILS_LOG_DEBUG_NAMED(
        "rmw_connext_shared_cpp",
        "Adding topic '%s' with type '%s' for node '%s'",
        topic_name.c_str(), type_name.c_str(), guid_stream.str().c_str());
    }
    topic_to_types_[topic_name].push_back(type_name);
    participant_to_topics_[guid][topic_name].push_back(type_name);
    return true;
  }

  /**
   * Remove a topic based on discovery.
   *
   * @param guid
   * @param topic_name
   * @param type_name
   * @return true if a change has been recorded
   */
  bool removeTopic(const GUID_t & guid,
                   const std::string & topic_name,
                   const std::string & type_name)
  {
    if (topic_to_types_.find(topic_name) == topic_to_types_.end()) {
      RCUTILS_LOG_DEBUG_NAMED(
        "rmw_connext_shared_cpp",
        "unexpected removal on topic '%s' with type '%s'",
        topic_name.c_str(), type_name.c_str());
      return false;
    }

    auto guid_topics_iter = participant_to_topics_.find(guid);
    if (guid_topics_iter != participant_to_topics_.end()
        && guid_topics_iter->second.find(topic_name) != guid_topics_iter->second.end()) {

      std::vector<std::string> & ref_type_vec = guid_topics_iter->second[topic_name];
      std::vector<std::string> & tgt_type_vec = topic_to_types_[topic_name];
      auto type_iter = std::find(ref_type_vec.begin(), ref_type_vec.end(), type_name);
      if (type_iter != ref_type_vec.end()) {
        ref_type_vec.erase(type_iter);
        tgt_type_vec.erase(std::find(tgt_type_vec.begin(), tgt_type_vec.end(), type_name));
      } else {
        RCUTILS_LOG_DEBUG_NAMED(
          "rmw_connext_shared_cpp",
          "Unable to remove topic type, topic '%s' does not contain type '%s'",
        topic_name.c_str(), type_name.c_str());
        return false;
      }

      if (ref_type_vec.size() == 0) {
        participant_to_topics_[guid].erase(topic_name);
      }
      if (tgt_type_vec.size() == 0) {
        topic_to_types_.erase(topic_name);
      } 

      if (participant_to_topics_[guid].size() == 0) {
        participant_to_topics_.erase(guid);
      }
    } else {
      RCUTILS_LOG_DEBUG_NAMED(
        "rmw_connext_shared_cpp",
        "Unable to remove topic, does not exist '%s' with type '%s'",
        topic_name.c_str(), type_name.c_str());
      return false;
    }

    return true;
  }

  /**
   * Remove participant and its topics and topic types
   *
   * @param guid
   * @return true if a change has been recorded
   */
  bool removeParticipant(const GUID_t & guid)
  {
    auto p_iter = participant_to_topics_.find(guid);
    if (p_iter == participant_to_topics_.end()) {
      if (rcutils_logging_logger_is_enabled_for("rmw_connext_shared_cpp", RCUTILS_LOG_SEVERITY_DEBUG)) {
        std::stringstream guid_stream;
        guid_stream << guid;
        RCUTILS_LOG_DEBUG_NAMED(
          "rmw_connext_shared_cpp",
          "unexpected removal of participant '%s'",
          guid_stream.str().c_str());
      }
      return false;
    }

    const TopicToTypes & topic_map = p_iter->second;
    for (auto name_iter = topic_map.begin(); name_iter != topic_map.end(); ++name_iter) {
      const std::vector<std::string> & ref_type_vec = name_iter->second;
      std::vector<std::string> & tgt_type_vec = topic_to_types_[name_iter->first];
      for (auto type_iter = ref_type_vec.begin(); type_iter != ref_type_vec.end(); ++type_iter) {
        tgt_type_vec.erase(std::find(tgt_type_vec.begin(), tgt_type_vec.end(), *type_iter));
      }
      if (tgt_type_vec.size() == 0) {
        topic_to_types_.erase(name_iter->first);
      }
    }

    participant_to_topics_.erase(p_iter);

    return true;
  }
};

template<typename GUID_t>
inline std::ostream & operator<<(std::ostream & ostream,
                                 const TopicCache<GUID_t> & topic_cache)
{
  std::stringstream map_ss;
  map_ss << "Participant Info: " << std::endl;
  for (auto & elem : topic_cache.getParticipantToTopics()) {
    std::ostringstream stream;
    stream << "  Topics: " << std::endl;
    for (auto & types : elem.second) {
      stream << "    " << types.first << ": ";
      std::copy(types.second.begin(), types.second.end(), std::ostream_iterator<std::string>(stream, ","));
      stream << std::endl;
    }
    map_ss << elem.first << std::endl << stream.str();
  }
  std::stringstream topics_ss;
  topics_ss << "Cumulative TopicToTypes: " << std::endl;
  for (auto & elem : topic_cache.getTopicToTypes()) {
    std::ostringstream stream;
    std::copy(elem.second.begin(), elem.second.end(), std::ostream_iterator<std::string>(stream, ","));
    topics_ss << "  " << elem.first << " : " << stream.str() << std::endl;
  }
  ostream << map_ss.str() << topics_ss.str();
  return ostream;
}

template <class T>
class LockedObject : public T
{
private:
  mutable std::mutex cache_mutex_;
public:
  /**
  * @return a reference to this object to lock.
  */
  std::mutex & getMutex() const
  {
    return cache_mutex_;
  }
};

#endif //RMW_CONNEXT_CPP_TOPIC_CACHE_H
