// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#ifndef ROSIDL_TYPESUPPORT_CONNEXT_CPP__REQUESTER_HPP_
#define ROSIDL_TYPESUPPORT_CONNEXT_CPP__REQUESTER_HPP_

#include <atomic>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <sstream>

#include <ndds/ndds_cpp.h>

#include "rosidl_typesupport_connext_cpp/impl/error_checking.hpp"
#include "rosidl_typesupport_connext_cpp/message_type_support.h"
#include "rosidl_typesupport_connext_cpp/service_type_support.h"

template <typename T>
struct Sample;

template <typename T>
struct TemplateDataReader;

template <typename T>
struct TemplateDataWriter;

namespace rosidl_typesupport_connext_cpp
{

template<typename RequestT, typename ResponseT>
class Requester
{
public:
  Requester(
    DDSDomainParticipant * participant, const std::string & service_name,
    const std::string & service_type_name)
  : participant_(participant), service_name_(service_name),
    service_type_name_(service_type_name), sequence_number_(0)
  {}

  const char * init(const DDS_DataReaderQos * datareader_qos,
    const DDS_DataWriterQos * datawriter_qos) noexcept
  {
    std::random_device rd;
    std::default_random_engine e1(rd());
    // NOTE: use extra parentheses to avoid macro expansion. On Windows,
    // max and min are defined as macros in <windows.h>
    // See http://stackoverflow.com/a/2561377/470581
    std::uniform_int_distribution<int32_t> uniform_dist(
      (std::numeric_limits<int32_t>::min)(),
      (std::numeric_limits<int32_t>::max)());
    writer_guid_ = std::make_tuple(
      uniform_dist(e1), uniform_dist(e1), uniform_dist(e1), uniform_dist(e1));

    std::stringstream ss;
    ss << "client_guid_0_ = " << std::get<0>(writer_guid_) <<
      " AND client_guid_1_ = " << std::get<1>(writer_guid_) <<
      " AND client_guid_2_ = " << std::get<2>(writer_guid_) <<
      " AND client_guid_3_ = " << std::get<3>(writer_guid_);

    std::string query(ss.str());

    DDS_StringSeq args;
    args.length(0);

    DDS_ReturnCode_t status;
    DDS_TopicQos default_topic_qos;
    DDS_PublisherQos publisher_qos;
    DDS_SubscriberQos subscriber_qos;
    std::string request_type_name = service_type_name_ + "_Request_";
    std::string request_topic_name = service_name_ + "_Request";
    std::string response_type_name = service_type_name_ + "_Response_";
    std::string response_topic_name = service_name_ + "_Response";
    const char * estr = nullptr;
    request_topic_ = nullptr;
    request_publisher_ = nullptr;
    request_datawriter_ = nullptr;
    response_topic_ = nullptr;
    response_subscriber_ = nullptr;
    response_datareader_ = nullptr;
    content_filtered_response_topic_ = nullptr;

    // Create request Publisher and DataWriter
    status = participant_->get_default_publisher_qos(publisher_qos);
    if (nullptr != (estr = impl::check_get_default_publisher_qos(status))) {
      goto fail;
    }

    request_publisher_ =
      participant_->create_publisher(publisher_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!request_publisher_) {
      estr = "DomainParticipant::create_publisher: failed for request";
      goto fail;
    }

    status = participant_->get_default_topic_qos(default_topic_qos);
    if (nullptr != (estr = impl::check_get_default_topic_qos(status))) {
      goto fail;
    }

    request_topic_ = participant_->create_topic(
      request_topic_name.c_str(), request_type_name.c_str(), default_topic_qos, NULL,
      DDS_STATUS_MASK_NONE);
    if (!request_topic_) {
      estr = "DomainParticipant::create_topic: failed for request";
      goto fail;
    }

    request_datawriter_ = request_publisher_->create_datawriter(
      request_topic_, *datawriter_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!request_datawriter_) {
      estr = "Publisher::create_datawriter: failed for request";
      goto fail;
    }

    // Create response Subscriber and DataReader
    status = participant_->get_default_subscriber_qos(subscriber_qos);
    if (nullptr != (estr = impl::check_get_default_datareader_qos(status))) {
      goto fail;
    }

    response_subscriber_ = participant_->create_subscriber(
      subscriber_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!response_subscriber_) {
      estr = "DomainParticipant::create_subscriber: failed for response";
      goto fail;
    }

    response_topic_ = participant_->create_topic(
      response_topic_name.c_str(), response_type_name.c_str(), default_topic_qos, NULL,
      DDS_STATUS_MASK_NONE);
    if (!response_topic_) {
      estr = "DomainParticipant::create_topic: failed for response";
      goto fail;
    }

    content_filtered_response_topic_ = participant_->create_contentfilteredtopic(
      service_name_.c_str(), response_topic_,
      query.c_str(),
      args);
    if (!content_filtered_response_topic_) {
      estr = "DomainParticipant::create_contentfilteredtopic: failed";
      goto fail;
    }

    response_datareader_ = response_subscriber_->create_datareader(
      content_filtered_response_topic_,
      *datareader_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!response_datareader_) {
      estr = "Subscriber::create_datawriter: failed for response";
      goto fail;
    }
    return nullptr;
fail:
    if (content_filtered_response_topic_) {
      status = participant_->delete_contentfilteredtopic(content_filtered_response_topic_);
      if (nullptr != impl::check_delete_contentfilteredtopic(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_contentfilteredtopic(status));
      }

    }
    if (response_datareader_) {
      // Assumption: subscriber is not null at this point.
      status = response_subscriber_->delete_datareader(response_datareader_);
      if (nullptr != impl::check_delete_datareader(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_datareader(status));
      }
    }
    if (response_subscriber_) {
      status = participant_->delete_subscriber(response_subscriber_);
      if (nullptr != impl::check_delete_subscriber(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_subscriber(status));
      }
    }
    if (response_topic_) {
      status = participant_->delete_topic(response_topic_);
      if (nullptr != impl::check_delete_topic(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_topic(status));
      }
    }
    if (request_datawriter_) {
      // Assumption: publisher is not null at this point.
      status = request_publisher_->delete_datawriter(request_datawriter_);
      if (nullptr != impl::check_delete_datawriter(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_datawriter(status));
      }
    }
    if (request_publisher_) {
      status = participant_->delete_publisher(request_publisher_);
      if (nullptr != impl::check_delete_publisher(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_publisher(status));
      }
    }
    if (request_topic_) {
      status = participant_->delete_topic(request_topic_);
      if (nullptr != impl::check_delete_topic(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_topic(status));
      }
    }
    return estr;
  }

  const char * take_response(Sample<ResponseT> & response, bool * taken) noexcept
  {
    return TemplateDataReader<Sample<ResponseT>>::take_sample(
      response_datareader_, response, taken);
  }

  const char * send_request(Sample<RequestT> & request) noexcept
  {
    request.sequence_number_ = ++sequence_number_;
    request.client_guid_0_ = std::get<0>(writer_guid_);
    request.client_guid_1_ = std::get<1>(writer_guid_);
    request.client_guid_2_ = std::get<2>(writer_guid_);
    request.client_guid_3_ = std::get<3>(writer_guid_);

    return TemplateDataWriter<Sample<RequestT>>::write_sample(request_datawriter_, request);
  }

  DDSDataReader * get_response_datareader()
  {
    return response_datareader_;
  }

private:
  DDSDomainParticipant * participant_;
  std::string service_name_;
  std::string service_type_name_;
  DDSDataReader * response_datareader_;
  DDSDataWriter * request_datawriter_;
  DDSTopic * response_topic_;
  DDSContentFilteredTopic * content_filtered_response_topic_;
  DDSTopic * request_topic_;
  DDSSubscriber * response_subscriber_;
  DDSPublisher * request_publisher_;
  std::atomic<int64_t> sequence_number_;
  std::tuple<int32_t, int32_t, int32_t, int32_t> writer_guid_;
};

}  // namespace rosidl_typesupport_connext_cpp

#endif  // ROSIDL_TYPESUPPORT_CONNEXT_CPP__REQUESTER_HPP_
