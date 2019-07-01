// generated from rosidl_typesupport_connext_cpp/resource/srv__type_support.cpp.em
// generated code does not contain a copyright notice

@#######################################################################
@# EmPy template for generating <srv>__type_support.cpp files
@#
@# Context:
@#  - spec (rosidl_parser.ServiceSpecification)
@#    Parsed specification of the .srv file
@#  - get_header_filename_from_msg_name (function)
@#######################################################################
@
#include "@(spec.pkg_name)/srv/@(get_header_filename_from_msg_name(spec.srv_name))__rosidl_typesupport_connext_cpp.hpp"

#ifdef Connext_GLIBCXX_USE_CXX11_ABI_ZERO
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif
#include <ndds/connext_cpp/connext_cpp_requester_details.h>
#include <ndds/ndds_cpp.h>
#include <ndds/ndds_requestreply_cpp.h>
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

#include "rmw/error_handling.h"
#include "rosidl_typesupport_connext_cpp/identifier.hpp"
#include "rosidl_typesupport_connext_cpp/service_type_support.h"
#include "rosidl_typesupport_connext_cpp/service_type_support_decl.hpp"

#include "@(spec.pkg_name)/srv/@(get_header_filename_from_msg_name(spec.srv_name))__struct.hpp"
#include "@(spec.pkg_name)/srv/dds_connext/@(spec.srv_name)_Request_Support.h"
#include "@(spec.pkg_name)/srv/@(get_header_filename_from_msg_name(spec.srv_name + '_Request'))__rosidl_typesupport_connext_cpp.hpp"
#include "@(spec.pkg_name)/srv/dds_connext/@(spec.srv_name)_Response_Support.h"
#include "@(spec.pkg_name)/srv/@(get_header_filename_from_msg_name(spec.srv_name + '_Response'))__rosidl_typesupport_connext_cpp.hpp"

class DDSDomainParticipant;
class DDSDataReader;
struct DDS_SampleIdentity_t;

namespace @(spec.pkg_name)
{

namespace srv
{

namespace typesupport_connext_cpp
{

void * create_requester__@(spec.srv_name)(
  void * untyped_participant,
  const char * service_name,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void ** untyped_reader,
  void ** untyped_writer,
  void * (*allocator)(size_t))
{
  using RequesterType = connext::Requester<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  if (!untyped_participant || !service_name || !untyped_reader) {
    return NULL;
  }
  auto _allocator = allocator ? allocator : &malloc;

  DDSDomainParticipant * participant = static_cast<DDSDomainParticipant *>(untyped_participant);
  const DDS_DataReaderQos * datareader_qos = static_cast<const DDS_DataReaderQos *>(untyped_datareader_qos);
  const DDS_DataWriterQos * datawriter_qos = static_cast<const DDS_DataWriterQos *>(untyped_datawriter_qos);
  connext::RequesterParams requester_params(participant);

  // we create separate publishers and subscribers
  // because the default publisher is a singleton within
  // the replier/requester context in connext.
  // if we use the implicit one, every service/client will
  // overwrite the QoS of all previous instances.
  DDS::Publisher * publisher = participant->create_publisher(
    DDS_PUBLISHER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
  if (publisher == NULL) {
    RMW_SET_ERROR_MSG("C++ exception during construction of publisher for requester");
    return NULL;
  }
  DDS::Subscriber * subscriber = participant->create_subscriber(
    DDS_SUBSCRIBER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
  if (subscriber == NULL) {
    RMW_SET_ERROR_MSG("C++ exception during construction of subscriber for requester");
    return NULL;
  }
  requester_params.publisher(publisher);
  requester_params.subscriber(subscriber);
  requester_params.service_name(service_name);
  requester_params.datareader_qos(*datareader_qos);
  requester_params.datawriter_qos(*datawriter_qos);

  RequesterType * requester = static_cast<RequesterType *>(_allocator(sizeof(RequesterType)));
  try {
    new (requester) RequesterType(requester_params);
  } catch (...) {
    RMW_SET_ERROR_MSG("C++ exception during construction of Requester");
    return NULL;
  }

  *untyped_reader = requester->get_reply_datareader();
  *untyped_writer = requester->get_request_datawriter();
  return requester;
}

const char * destroy_requester__@(spec.srv_name)(
  void * untyped_requester,
  void (*deallocator)(void *))
{
  using RequesterType = connext::Requester<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  auto requester = static_cast<RequesterType *>(untyped_requester);

  requester->~RequesterType();
  auto _deallocator = deallocator ? deallocator : &free;
  _deallocator(requester);
  return nullptr;
}

int64_t send_request__@(spec.srv_name)(
  void * untyped_requester,
  const void * untyped_ros_request)
{
  using ROSRequestType = @(spec.pkg_name)::srv::@(spec.srv_name)_Request;
  using RequesterType = connext::Requester<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  connext::WriteSample<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_> request;
  const ROSRequestType & ros_request = *(
    static_cast<const ROSRequestType *>(untyped_ros_request));
  @(spec.pkg_name)::srv::typesupport_connext_cpp::convert_ros_message_to_dds(
    ros_request, request.data());

  RequesterType * requester = static_cast<RequesterType *>(untyped_requester);

  requester->send_request(request);
  int64_t sequence_number = ((int64_t)request.identity().sequence_number.high) << 32 |
    request.identity().sequence_number.low;
  return sequence_number;
}

void * create_replier__@(spec.srv_name)(
  void * untyped_participant,
  const char * service_name,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void ** untyped_reader,
  void ** untyped_writer,
  void * (*allocator)(size_t))
{
  using ReplierType = connext::Replier<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  if (!untyped_participant || !service_name || !untyped_reader) {
    return NULL;
  }
  auto _allocator = allocator ? allocator : &malloc;

  DDSDomainParticipant * participant = static_cast<DDSDomainParticipant *>(untyped_participant);
  const DDS_DataReaderQos * datareader_qos = static_cast<const DDS_DataReaderQos *>(untyped_datareader_qos);
  const DDS_DataWriterQos * datawriter_qos = static_cast<const DDS_DataWriterQos *>(untyped_datawriter_qos);
  connext::ReplierParams<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_, @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_> replier_params(participant);

  // we create separate publishers and subscribers
  // because the default publisher is a singleton within
  // the replier/requester context in connext.
  // if we use the implicit one, every service/client will
  // overwrite the QoS of all previous instances.
  DDS::Publisher * publisher = participant->create_publisher(
    DDS_PUBLISHER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
  if (publisher == NULL) {
    RMW_SET_ERROR_MSG("C++ exception during construction of publisher for replier");
    return NULL;
  }
  DDS::Subscriber * subscriber = participant->create_subscriber(
    DDS_SUBSCRIBER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
  if (subscriber == NULL) {
    RMW_SET_ERROR_MSG("C++ exception during construction of subscriber for replier");
    return NULL;
  }
  replier_params.publisher(publisher);
  replier_params.subscriber(subscriber);
  replier_params.service_name(service_name);
  // replier_params.request_topic_name(std::string(service_name)+"Request");
  // replier_params.reply_topic_name(std::string(service_name)+"Reply");
  replier_params.datareader_qos(*datareader_qos);
  replier_params.datawriter_qos(*datawriter_qos);

  ReplierType * replier = static_cast<ReplierType *>(_allocator(sizeof(ReplierType)));
  try {
    new (replier) ReplierType(replier_params);
  } catch (...) {
    RMW_SET_ERROR_MSG("C++ exception during construction of Requester");
    return NULL;
  }

  *untyped_reader = replier->get_request_datareader();
  *untyped_writer = replier->get_reply_datawriter();
  return replier;
}

const char * destroy_replier__@(spec.srv_name)(
  void * untyped_replier,
  void (*deallocator)(void *))
{
  using ReplierType = connext::Replier<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  auto replier = static_cast<ReplierType *>(untyped_replier);

  replier->~ReplierType();
  auto _deallocator = deallocator ? deallocator : &free;
  _deallocator(replier);
  return nullptr;
}

bool take_request__@(spec.srv_name)(
  void * untyped_replier,
  rmw_request_id_t * request_header,
  void * untyped_ros_request)
{
  using ReplierType = connext::Replier<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  using ROSRequestType = @(spec.pkg_name)::srv::@(spec.srv_name)_Request;
  if (!untyped_replier || !request_header || !untyped_ros_request) {
    return false;
  }

  ReplierType * replier = static_cast<ReplierType *>(untyped_replier);

  ROSRequestType & ros_request = *static_cast<ROSRequestType *>(untyped_ros_request);

  connext::Sample<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_> request;
  bool taken = replier->take_request(request);
  if (!taken) {
    return false;
  }
  if (!request.info().valid_data) {
    return false;
  }

  bool converted =
    @(spec.pkg_name)::srv::typesupport_connext_cpp::convert_dds_message_to_ros(request.data(), ros_request);
  if (!converted) {
    return false;
  }

  size_t SAMPLE_IDENTITY_SIZE = 16;
  memcpy(&(request_header->writer_guid[0]), request.identity().writer_guid.value, SAMPLE_IDENTITY_SIZE);

  request_header->sequence_number = ((int64_t)request.identity().sequence_number.high) << 32 | request.identity().sequence_number.low;
  return true;
}

bool take_response__@(spec.srv_name)(
  void * untyped_requester,
  rmw_request_id_t * request_header,
  void * untyped_ros_response)
{
  using RequesterType = connext::Requester<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_, @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  using ROSResponseType = @(spec.pkg_name)::srv::@(spec.srv_name)_Response;
  if (!untyped_requester || !request_header || !untyped_ros_response) {
    return false;
  }

  RequesterType * requester = static_cast<RequesterType *>(untyped_requester);

  ROSResponseType & ros_response = *static_cast<ROSResponseType *>(untyped_ros_response);

  connext::Sample<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_> response;
  bool received = requester->take_reply(response);
  if (!received) {
    return false;
  }
  if (!response.info().valid_data) {
    return false;
  }

  int64_t sequence_number =
    (((int64_t)response.related_identity().sequence_number.high) << 32) |
    response.related_identity().sequence_number.low;
  request_header->sequence_number = sequence_number;

  bool converted =
    @(spec.pkg_name)::srv::typesupport_connext_cpp::convert_dds_message_to_ros(response.data(), ros_response);
  return converted;
}

bool send_response__@(spec.srv_name)(
  void * untyped_replier,
  const rmw_request_id_t * request_header,
  const void * untyped_ros_response)
{
  using ROSResponseType = const @(spec.pkg_name)::srv::@(spec.srv_name)_Response;
  using ReplierType = connext::Replier<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_, @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  if (!untyped_replier || !request_header || !untyped_ros_response) {
    return false;
  }

  connext::WriteSample<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_> response;
  ROSResponseType & ros_response = *(reinterpret_cast<ROSResponseType *>(untyped_ros_response));
  bool converted =
    @(spec.pkg_name)::srv::typesupport_connext_cpp::convert_ros_message_to_dds(ros_response, response.data());
  if (!converted) {
    return false;
  }

  DDS_SampleIdentity_t request_identity;

  size_t SAMPLE_IDENTITY_SIZE = 16;
  memcpy(request_identity.writer_guid.value, &request_header->writer_guid[0], SAMPLE_IDENTITY_SIZE);

  request_identity.sequence_number.high = (int32_t)((request_header->sequence_number & 0xFFFFFFFF00000000) >> 32);
  request_identity.sequence_number.low = (uint32_t)(request_header->sequence_number & 0xFFFFFFFF);

  ReplierType * replier = static_cast<ReplierType *>(untyped_replier);

  replier->send_reply(response, request_identity);
  return true;
}

void *
get_request_datawriter__@(spec.srv_name)(void * untyped_requester)
{
  if (!untyped_requester) {
    return NULL;
  }
  using RequesterType = connext::Requester<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  RequesterType * requester = reinterpret_cast<RequesterType *>(untyped_requester);
  return requester->get_request_datawriter();
}

void *
get_reply_datareader__@(spec.srv_name)(void * untyped_requester)
{
  if (!untyped_requester) {
    return NULL;
  }
  using RequesterType = connext::Requester<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  RequesterType * requester = reinterpret_cast<RequesterType *>(untyped_requester);
  return requester->get_reply_datareader();
}

void *
get_request_datareader__@(spec.srv_name)(void * untyped_replier)
{
  if (!untyped_replier) {
    return NULL;
  }
  using ReplierType = connext::Replier<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  ReplierType * replier = reinterpret_cast<ReplierType *>(untyped_replier);
  return replier->get_request_datareader();
}

void *
get_reply_datawriter__@(spec.srv_name)(void * untyped_replier)
{
  if (!untyped_replier) {
    return NULL;
  }
  using ReplierType = connext::Replier<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  ReplierType * replier = reinterpret_cast<ReplierType *>(untyped_replier);
  return replier->get_reply_datawriter();
}

static service_type_support_callbacks_t callbacks = {
  "@(spec.pkg_name)",
  "@(spec.srv_name)",
  &create_requester__@(spec.srv_name),
  &destroy_requester__@(spec.srv_name),
  &create_replier__@(spec.srv_name),
  &destroy_replier__@(spec.srv_name),
  &send_request__@(spec.srv_name),
  &take_request__@(spec.srv_name),
  &send_response__@(spec.srv_name),
  &take_response__@(spec.srv_name),
  &get_request_datawriter__@(spec.srv_name),
  &get_reply_datareader__@(spec.srv_name),
  &get_request_datareader__@(spec.srv_name),
  &get_reply_datawriter__@(spec.srv_name),
};

static rosidl_service_type_support_t handle = {
  rosidl_typesupport_connext_cpp::typesupport_identifier,
  &callbacks,
  get_service_typesupport_handle_function,
};

}  // namespace typesupport_connext_cpp

}  // namespace srv

}  // namespace @(spec.pkg_name)

namespace rosidl_typesupport_connext_cpp
{

template<>
ROSIDL_TYPESUPPORT_CONNEXT_CPP_EXPORT_@(spec.pkg_name)
const rosidl_service_type_support_t *
get_service_type_support_handle<@(spec.pkg_name)::srv::@(spec.srv_name)>()
{
  return &@(spec.pkg_name)::srv::typesupport_connext_cpp::handle;
}

}  // namespace rosidl_typesupport_connext_cpp

#ifdef __cplusplus
extern "C"
{
#endif

const rosidl_service_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_connext_cpp, @(spec.pkg_name), @(spec.srv_name))() {
  return &@(spec.pkg_name)::srv::typesupport_connext_cpp::handle;
}

#ifdef __cplusplus
}
#endif
