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

#ifndef RMW_CONNEXT_CPP__CONNEXT_STATIC_PUBLISHER_INFO_HPP_
#define RMW_CONNEXT_CPP__CONNEXT_STATIC_PUBLISHER_INFO_HPP_

#include <atomic>

#include "rmw_connext_shared_cpp/ndds_include.hpp"
#include "rmw_connext_shared_cpp/types.hpp"

#include "ndds/ndds_cpp.h"
#include "ndds/ndds_namespace_cpp.h"

#include "rosidl_typesupport_connext_cpp/message_type_support.h"
#include "rmw_connext_shared_cpp/connext_static_event_info.hpp"

#include "rmw/types.h"
#include "rmw/ret_types.h"


class ConnextPublisherListener;

extern "C"
{
struct ConnextStaticPublisherInfo : ConnextCustomEventInfo
{
  DDS::Publisher * dds_publisher_;
  ConnextPublisherListener * listener_;
  DDS::DataWriter * topic_writer_;
  const message_type_support_callbacks_t * callbacks_;
  rmw_gid_t publisher_gid;
  rmw_ret_t get_status(const DDS_StatusMask mask, void * event) override;
  DDSEntity * get_entity() override;
};
}  // extern "C"

class ConnextPublisherListener : public PublisherListener
{
public:
  virtual void on_publication_matched(
    DDSDataWriter *,
    const DDS_PublicationMatchedStatus & status)
  {
    current_count_ = status.current_count;
  }

  std::size_t current_count() const
  {
    return current_count_;
  }

private:
  std::atomic<std::size_t> current_count_;
};

/**
 * Remap the specific RTI Connext DDS DataWriter Status to a generic RMW status type.
 *
 * @param mask input status mask
 * @param event
 */
inline rmw_ret_t ConnextStaticPublisherInfo::get_status(const DDS_StatusMask mask, void * event)
{
  switch (mask) {
    case DDS_StatusKind::DDS_LIVELINESS_LOST_STATUS: {
        DDS_LivelinessLostStatus liveliness_lost;
        DDS_ReturnCode_t dds_return_code =
          topic_writer_->get_liveliness_lost_status(liveliness_lost);

        rmw_ret_t from_dds = check_dds_ret_code(dds_return_code);
        if (from_dds != RMW_RET_OK) {
          return from_dds;
        }

        rmw_liveliness_lost_t * rmw_liveliness_lost = static_cast<rmw_liveliness_lost_t *>(event);
        rmw_liveliness_lost->total_count = liveliness_lost.total_count;
        rmw_liveliness_lost->total_count_change = liveliness_lost.total_count_change;

        break;
      }
    case DDS_StatusKind::DDS_OFFERED_DEADLINE_MISSED_STATUS: {
        DDS_OfferedDeadlineMissedStatus offered_deadline_missed;
        DDS_ReturnCode_t dds_return_code = topic_writer_
          ->get_offered_deadline_missed_status(offered_deadline_missed);

        rmw_ret_t from_dds = check_dds_ret_code(dds_return_code);
        if (from_dds != RMW_RET_OK) {
          return from_dds;
        }

        rmw_offered_deadline_missed_t * rmw_offered_deadline_missed =
          static_cast<rmw_offered_deadline_missed_t *>(event);
        rmw_offered_deadline_missed->total_count = offered_deadline_missed.total_count;
        rmw_offered_deadline_missed->total_count_change =
          offered_deadline_missed.total_count_change;

        break;
      }
    case DDS_StatusKind::DDS_OFFERED_INCOMPATIBLE_QOS_STATUS: {
        DDS_OfferedIncompatibleQosStatus offered_incompatible_status;
        DDS_ReturnCode_t dds_return_code = topic_writer_
          ->get_offered_incompatible_qos_status(offered_incompatible_status);

        rmw_ret_t from_dds = check_dds_ret_code(dds_return_code);
        if (from_dds != RMW_RET_OK) {
          return from_dds;
        }

        rmw_offered_incompatible_qos_t * rmw_offered_incompatible_qos =
          static_cast<rmw_offered_incompatible_qos_t *>(event);
        rmw_offered_incompatible_qos->total_count = offered_incompatible_status.total_count;
        rmw_offered_incompatible_qos->total_count_change =
          offered_incompatible_status.total_count_change;

        break;
      }
    case DDS_StatusKind::DDS_PUBLICATION_MATCHED_STATUS: {
        DDS_PublicationMatchedStatus publication_matched_status;
        DDS_ReturnCode_t dds_return_code = topic_writer_
          ->get_publication_matched_status(publication_matched_status);

        rmw_ret_t from_dds = check_dds_ret_code(dds_return_code);
        if (from_dds != RMW_RET_OK) {
          return from_dds;
        }

        rmw_publication_matched_t * rmw_publication_matched =
          static_cast<rmw_publication_matched_t *>(event);
        rmw_publication_matched->total_count = publication_matched_status.total_count;
        rmw_publication_matched->total_count_change = publication_matched_status.total_count_change;
        rmw_publication_matched->current_count = publication_matched_status.current_count;
        rmw_publication_matched->current_count_change =
          publication_matched_status.current_count_change;

        break;
      }
    default:
      return RMW_RET_EVENT_UNSUPPORTED;
  }
  return RMW_RET_OK;
}
inline DDSEntity * ConnextStaticPublisherInfo::get_entity()
{
  return dds_publisher_;
}

#endif  // RMW_CONNEXT_CPP__CONNEXT_STATIC_PUBLISHER_INFO_HPP_
