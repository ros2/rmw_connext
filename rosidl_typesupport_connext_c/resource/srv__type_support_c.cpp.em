// generated from rosidl_typesupport_connext_c/resource/srv__type_support_c.cpp.em
// generated code does not contain a copyright notice

@#######################################################################
@# EmPy template for generating <srv>__type_support_c.cpp files
@#
@# Context:
@#  - spec (rosidl_parser.ServiceSpecification)
@#    Parsed specification of the .srv file
@#  - get_header_filename_from_msg_name (function)
@#######################################################################
@
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Winfinite-recursion"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif
#include <ndds/connext_cpp/connext_cpp_requester_details.h>
#include <ndds/ndds_cpp.h>
#include <ndds/ndds_requestreply_cpp.h>
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rosidl_generator_cpp/service_type_support.hpp"
// this is defined in the rosidl_typesupport_connext_cpp package and
// is in the include/rosidl_typesupport_connext_cpp/impl folder
#include "rosidl_generator_c/message_type_support.h"
#include "rosidl_typesupport_connext_c/identifier.h"
#include "rosidl_typesupport_connext_c/visibility_control.h"
// Provides the definition of the service_type_support_callbacks_t struct.
#include "rosidl_typesupport_connext_cpp/service_type_support.h"

#include "@(spec.pkg_name)/srv/dds_connext_c/@(get_header_filename_from_msg_name(spec.srv_name + '_Request'))__type_support.h"
#include "@(spec.pkg_name)/srv/dds_connext_c/@(get_header_filename_from_msg_name(spec.srv_name + '_Response'))__type_support.h"

#include "@(spec.pkg_name)/srv/dds_connext/@(spec.srv_name)_Request_Support.h"
#include "@(spec.pkg_name)/srv/dds_connext/@(get_header_filename_from_msg_name(spec.srv_name + '_Request'))__type_support.hpp"
#include "@(spec.pkg_name)/srv/dds_connext/@(spec.srv_name)_Response_Support.h"
#include "@(spec.pkg_name)/srv/dds_connext/@(get_header_filename_from_msg_name(spec.srv_name + '_Response'))__type_support.hpp"

// Re-use most of the functions from C++ typesupport
#include "@(spec.pkg_name)/srv/dds_connext/@(get_header_filename_from_msg_name(spec.srv_name))__type_support.hpp"

#include "@(spec.pkg_name)/msg/rosidl_generator_c__visibility_control.h"

class DDSDomainParticipant;
class DDSDataReader;
struct DDS_SampleIdentity_t;

#if defined(__cplusplus)
extern "C"
{
#endif

void * create_requester__@(spec.srv_name)(
  void * untyped_participant,
  const char * service_name,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void ** untyped_reader,
  void * (*allocator)(size_t))
{
  return @(spec.pkg_name)::srv::typesupport_connext_cpp::create_requester__@(spec.srv_name)(
    untyped_participant,
    service_name,
    untyped_datareader_qos,
    untyped_datawriter_qos,
    untyped_reader,
    allocator);
}
const char * destroy_requester__@(spec.srv_name)(
  void * untyped_requester,
  void (* deallocator)(void *))
{
  return @(spec.pkg_name)::srv::typesupport_connext_cpp::destroy_requester__@(spec.srv_name)(
    untyped_requester, deallocator);
}

int64_t send_request__@(spec.srv_name)(
  void * untyped_requester,
  const void * untyped_ros_request)
{
  using RequesterType = connext::Requester<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  connext::WriteSample<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_> request;
  bool converted = @(spec.pkg_name)__@(spec.srv_name)_Request__convert_ros_to_dds(
    untyped_ros_request, static_cast<void*>(&request.data()));
  if (!converted) {
    fprintf(stderr, "Unable to convert request!\n");
    return -1;
  }

  RequesterType * requester = reinterpret_cast<RequesterType *>(untyped_requester);

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
  void * (*allocator)(size_t))
{
  return @(spec.pkg_name)::srv::typesupport_connext_cpp::create_replier__@(spec.srv_name)(
    untyped_participant,
    service_name,
    untyped_datareader_qos,
    untyped_datawriter_qos,
    untyped_reader,
    allocator);
}

const char * destroy_replier__@(spec.srv_name)(
  void * untyped_replier,
  void (* deallocator)(void *))
{
  return @(spec.pkg_name)::srv::typesupport_connext_cpp::destroy_replier__@(spec.srv_name)(
    untyped_replier, deallocator);
}

bool take_request__@(spec.srv_name)(
  void * untyped_replier,
  rmw_request_id_t * request_header,
  void * untyped_ros_request)
{
  using ReplierType = connext::Replier<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  if (!untyped_replier || !request_header || !untyped_ros_request) {
    return false;
  }

  ReplierType * replier = reinterpret_cast<ReplierType *>(untyped_replier);

  connext::Sample<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_> request;
  bool taken = replier->take_request(request);
  if (!taken) {
    return false;
  }
  if (!request.info().valid_data) {
    return false;
  }

  bool converted =
    @(spec.pkg_name)__@(spec.srv_name)_Request__convert_dds_to_ros(
      static_cast<const void *>(&request.data()), untyped_ros_request);
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
  if (!untyped_requester || !request_header || !untyped_ros_response) {
    return false;
  }

  RequesterType * requester = reinterpret_cast<RequesterType *>(untyped_requester);

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
    @(spec.pkg_name)__@(spec.srv_name)_Response__convert_dds_to_ros(
      static_cast<const void *>(&response.data()), untyped_ros_response);
  return converted;
}

bool send_response__@(spec.srv_name)(
  void * untyped_replier,
  const rmw_request_id_t * request_header,
  const void * untyped_ros_response)
{
  using ReplierType = connext::Replier<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_, @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  if (!untyped_replier || !request_header || !untyped_ros_response) {
    return false;
  }

  connext::WriteSample<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_> response;
  bool converted =
    @(spec.pkg_name)__@(spec.srv_name)_Response__convert_ros_to_dds(
      untyped_ros_response, static_cast<void *>(&response.data()));
  if (!converted) {
    return false;
  }

  DDS_SampleIdentity_t request_identity;

  size_t SAMPLE_IDENTITY_SIZE = 16;
  memcpy(request_identity.writer_guid.value, &request_header->writer_guid[0], SAMPLE_IDENTITY_SIZE);

  request_identity.sequence_number.high = (int32_t)((request_header->sequence_number & 0xFFFFFFFF00000000) >> 32);
  request_identity.sequence_number.low = (uint32_t)(request_header->sequence_number & 0xFFFFFFFF);

  ReplierType * replier = reinterpret_cast<ReplierType *>(untyped_replier);

  replier->send_reply(response, request_identity);
  return true;
}

const char *
get_request_topic_name__@(spec.srv_name)(void * untyped_requester)
{
  return @(spec.pkg_name)::srv::typesupport_connext_cpp::get_request_topic_name__@(spec.srv_name)(
    untyped_requester);
}

static service_type_support_callbacks_t __callbacks = {
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
  &get_request_topic_name__@(spec.srv_name),
};

static rosidl_service_type_support_t __type_support = {
  rosidl_typesupport_connext_c__identifier,
  &__callbacks
};


ROSIDL_GENERATOR_C_EXPORT_@(spec.pkg_name)
const rosidl_service_type_support_t *
ROSIDL_GET_TYPE_SUPPORT_FUNCTION(@(spec.pkg_name), srv, @(spec.srv_name))() {
  if (!__type_support.typesupport_identifier) {
    __type_support.typesupport_identifier = rosidl_typesupport_connext_c__identifier;
  }
  return &__type_support;
}

#if defined(__cplusplus)
}
#endif
