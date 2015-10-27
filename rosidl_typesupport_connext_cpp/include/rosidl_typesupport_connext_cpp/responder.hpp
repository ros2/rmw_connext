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

#ifndef ROSIDL_TYPESUPPORT_CONNEXT_CPP__RESPONDER_HPP_
#define ROSIDL_TYPESUPPORT_CONNEXT_CPP__RESPONDER_HPP_

#include <string>

#include "rosidl_typesupport_connext_cpp/impl/error_checking.hpp"
#include "rosidl_typesupport_connext_cpp/message_type_support.h"
#include "rosidl_typesupport_connext_cpp/service_type_support.h"
#include "rmw/rmw.h"

namespace rosidl_typesupport_connext_cpp
{

template<typename RequestT, typename ResponseT>
class Responder
{
public:
  Responder(
    DDSDomainParticipant * participant, const std::string & service_name,
    const std::string & service_type_name)
  : participant_(participant),
    service_name_(service_name), service_type_name_(service_type_name)
  {}

  const char * init(const DDS_DataReaderQos * datareader_qos,
    const DDS_DataWriterQos * datawriter_qos)
  {
    DDS_ReturnCode_t status;
    DDS_TopicQos default_topic_qos;
    DDS_SubscriberQos subscriber_qos;
    DDS_PublisherQos publisher_qos;
    std::string request_type_name = service_type_name_ + "_Request_";
    std::string request_topic_name = service_name_ + "_Request";
    std::string response_type_name = service_type_name_ + "_Response_";
    std::string response_topic_name = service_name_ + "_Response";
    const char * estr = nullptr;
    request_topic_ = nullptr;
    request_subscriber_ = nullptr;
    request_datareader_ = nullptr;
    response_publisher_ = nullptr;
    response_topic_ = nullptr;
    response_datawriter_ = nullptr;

    // Create request Publisher and DataWriter
    status = participant_->get_default_topic_qos(default_topic_qos);
    if (nullptr != (estr = impl::check_get_default_topic_qos(status))) {
      goto fail;
    }

    request_topic_ = participant_->create_topic(
      request_topic_name.c_str(), request_type_name.c_str(), default_topic_qos, NULL,
      DDS_STATUS_MASK_NONE);
    if (!request_topic_) {
      estr = "DomainParticipant::create_topic: failed";
      goto fail;
    }

    // Create response Subscriber and DataReader
    status = participant_->get_default_subscriber_qos(subscriber_qos);
    if (nullptr != (estr = impl::check_get_default_subscriber_qos(status))) {
      goto fail;
    }

    request_subscriber_ = participant_->create_subscriber(
      subscriber_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!request_subscriber_) {
      estr = "DomainParticipant::create_subscriber: failed";
      goto fail;
    }

    request_datareader_ = request_subscriber_->create_datareader(
      request_topic_,
      *datareader_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!request_datareader_) {
      estr = "Subscriber::create_datareader: failed";
      goto fail;
    }

    // Create request Publisher and DataWriter
    status = participant_->get_default_publisher_qos(publisher_qos);
    if (nullptr != (estr = impl::check_get_default_publisher_qos(status))) {
      goto fail;
    }

    response_publisher_ = participant_->create_publisher(
      publisher_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!response_publisher_) {
      estr = "DomainParticipant::create_publisher: failed";
      goto fail;
    }

    response_topic_ = participant_->create_topic(
      response_topic_name.c_str(), response_type_name.c_str(), default_topic_qos, NULL,
      DDS_STATUS_MASK_NONE);
    if (!response_topic_) {
      estr = "DomainParticipant::create_topic: failed";
      goto fail;
    }

    response_datawriter_ = response_publisher_->create_datawriter(
      response_topic_, *datawriter_qos, NULL, DDS_STATUS_MASK_NONE);
    if (!response_datawriter_) {
      estr = "Publisher::create_datawriter: failed";
      goto fail;
    }
    return nullptr;
fail:
    if (response_datawriter_) {
      // Assumption: publisher is not null at this point.
      status = response_publisher_->delete_datawriter(response_datawriter_);
      if (nullptr != impl::check_delete_datawriter(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_datawriter(status));
      }
    }
    if (response_topic_) {
      status = participant_->delete_topic(response_topic_);
      if (nullptr != impl::check_delete_topic(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_topic(status));
      }
    }
    if (response_publisher_) {
      status = participant_->delete_publisher(response_publisher_);
      if (nullptr != impl::check_delete_publisher(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_publisher(status));
      }
    }
    if (request_datareader_) {
      // Assumption: subscriber is not null at this point.
      status = request_subscriber_->delete_datareader(request_datareader_);
      if (nullptr != impl::check_delete_datareader(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_datareader(status));
      }
    }
    if (request_subscriber_) {
      status = participant_->delete_subscriber(request_subscriber_);
      if (nullptr != impl::check_delete_subscriber(status)) {
        fprintf(stderr, "%s\n", impl::check_delete_subscriber(status));
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

  DDSDataReader * get_request_datareader()
  {
    return request_datareader_;
  }

  const char * take_request(Sample<RequestT> & request, bool * taken)
  noexcept
  {
    return TemplateDataReader<Sample<RequestT>>::take_sample(request_datareader_, request, taken);
  }

  const char * send_response(const rmw_request_id_t & request_header, Sample<ResponseT> & response)
  noexcept
  {
    response.sequence_number_ = request_header.sequence_number;
    response.client_guid_0_ = *(reinterpret_cast<const int32_t *>(&request_header.writer_guid[0]));
    response.client_guid_1_ = *(reinterpret_cast<const int32_t *>(
        &request_header.writer_guid[0] + sizeof(response.client_guid_0_)));
    response.client_guid_2_ = *(reinterpret_cast<const int32_t *>(
        &request_header.writer_guid[0] + 2 * sizeof(response.client_guid_0_)));
    response.client_guid_3_ = *(reinterpret_cast<const int32_t *>(
        &request_header.writer_guid[0] + 3 * sizeof(response.client_guid_0_)));

    return TemplateDataWriter<Sample<ResponseT>>::write_sample(response_datawriter_, response);
  }

private:
  DDSDomainParticipant * participant_;
  std::string service_name_;
  std::string service_type_name_;
  DDSDataReader * request_datareader_;
  DDSTopic * request_topic_;
  DDSSubscriber * request_subscriber_;
  DDSDataWriter * response_datawriter_;
  DDSPublisher * response_publisher_;
  DDSTopic * response_topic_;
};

}  // namespace rosidl_typesupport_connext_cpp

#endif  // ROSIDL_TYPESUPPORT_CONNEXT_CPP__RESPONDER_HPP_
