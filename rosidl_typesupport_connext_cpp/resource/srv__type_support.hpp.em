// generated from rosidl_typesupport_connext_cpp/resource/srv__type_support.hpp.em
// generated code does not contain a copyright notice

@#######################################################################
@# EmPy template for generating <srv>__type_support.hpp files
@#
@# Context:
@#  - spec (rosidl_parser.MessageSpecification)
@#    Parsed specification of the .srv file
@#  - get_header_filename_from_srv_name (function)
@#######################################################################
@
@{
header_guard_parts = [
    spec.pkg_name, 'srv', 'dds_connext',
    get_header_filename_from_msg_name(spec.srv_name) + '__type_support_hpp']
header_guard_variable = '__'.join([x.upper() for x in header_guard_parts]) + '_'
}@
#ifndef @(header_guard_variable)
#define @(header_guard_variable)

#include <rosidl_typesupport_connext_cpp/visibility_control.h>

#include <rmw/types.h>

namespace @(spec.pkg_name)
{

namespace srv
{

namespace typesupport_connext_cpp
{
ROSIDL_TYPESUPPORT_CONNEXT_CPP_PUBLIC
void *
create_requester__@(spec.srv_name)(
  void * untyped_participant,
  const char * service_name,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void ** untyped_reader,
  void * (*allocator)(size_t));

ROSIDL_TYPESUPPORT_CONNEXT_CPP_PUBLIC
const char *
destroy_requester__@(spec.srv_name)(
  void * untyped_requester,
  void (* deallocator)(void *));

ROSIDL_TYPESUPPORT_CONNEXT_CPP_PUBLIC
int64_t
send_request__@(spec.srv_name)(
  void * untyped_requester,
  const void * untyped_ros_request);

ROSIDL_TYPESUPPORT_CONNEXT_CPP_PUBLIC
void *
create_replier__@(spec.srv_name)(
  void * untyped_participant,
  const char * service_name,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void ** untyped_reader,
  void * (*allocator)(size_t));

ROSIDL_TYPESUPPORT_CONNEXT_CPP_PUBLIC
const char *
destroy_replier__@(spec.srv_name)(
  void * untyped_replier,
  void (* deallocator)(void *));

ROSIDL_TYPESUPPORT_CONNEXT_CPP_PUBLIC
bool
take_request__@(spec.srv_name)(
  void * untyped_replier,
  rmw_request_id_t * request_header,
  void * untyped_ros_request);

ROSIDL_TYPESUPPORT_CONNEXT_CPP_PUBLIC
bool
take_response__@(spec.srv_name)(
  void * untyped_requester,
  rmw_request_id_t * request_header,
  void * untyped_ros_response);

ROSIDL_TYPESUPPORT_CONNEXT_CPP_PUBLIC
bool
send_response__@(spec.srv_name)(
  void * untyped_replier,
  const rmw_request_id_t * request_header,
  const void * untyped_ros_response);

ROSIDL_TYPESUPPORT_CONNEXT_CPP_PUBLIC
const char *
get_request_topic_name__@(spec.srv_name)(void * untyped_requester);

}  // namespace typesupport_connext_cpp

}  // namespace srv

}  // namespace @(spec.pkg_name)

#endif  // @(header_guard_variable)
