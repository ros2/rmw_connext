// generated from rosidl_typesupport_connext_cpp/resource/msg__type_support.hpp.em
// generated code does not contain a copyright notice

@#######################################################################
@# EmPy template for generating <msg>__type_support.hpp files
@#
@# Context:
@#  - spec (rosidl_parser.MessageSpecification)
@#    Parsed specification of the .msg file
@#  - subfolder (string)
@#    The subfolder / subnamespace of the message
@#    Either 'msg' or 'srv'
@#  - get_header_filename_from_msg_name (function)
@#######################################################################
@
@{
header_guard_parts = [
    spec.base_type.pkg_name, subfolder, 'dds_connext',
    get_header_filename_from_msg_name(spec.base_type.type) + '__type_support_hpp']
header_guard_variable = '__'.join([x.upper() for x in header_guard_parts]) + '_'
}@
#ifndef @(header_guard_variable)
#define @(header_guard_variable)

#include <rosidl_typesupport_connext_cpp/visibility_control.h>

#include "@(spec.base_type.pkg_name)/@(subfolder)/@(get_header_filename_from_msg_name(spec.base_type.type))__struct.hpp"
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# pragma GCC diagnostic ignored "-Wunused-variable"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif
#include "@(spec.base_type.pkg_name)/@(subfolder)/dds_connext/@(spec.base_type.type)_Support.h"
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

// forward declaration of DDS types
class DDSDomainParticipant;
class DDSDataWriter;
class DDSDataReader;

namespace @(spec.base_type.pkg_name)
{

namespace @(subfolder)
{

namespace typesupport_connext_cpp
{

bool
register_type__@(spec.base_type.type)(
  DDSDomainParticipant * participant,
  const char * type_name);

bool
ROSIDL_TYPESUPPORT_CONNEXT_CPP_PUBLIC
convert_ros_message_to_dds(
  const @(spec.base_type.pkg_name)::@(subfolder)::@(spec.base_type.type) & ros_message,
  @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_ & dds_message);

bool
publish__@(spec.base_type.type)(
  DDSDataWriter * topic_writer,
  const void * untyped_ros_message);

bool
ROSIDL_TYPESUPPORT_CONNEXT_CPP_PUBLIC
convert_dds_message_to_ros(
  const @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_ & dds_message,
  @(spec.base_type.pkg_name)::@(subfolder)::@(spec.base_type.type) & ros_message);

bool
take__@(spec.base_type.type)(
  DDSDataReader * topic_reader,
  bool ignore_local_publications,
  void * untyped_ros_message,
  bool * taken);

}  // namespace typesupport_connext_cpp

}  // namespace @(subfolder)

}  // namespace @(spec.base_type.pkg_name)

#endif  // @(header_guard_variable)
