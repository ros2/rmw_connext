// generated from rosidl_typesupport_connext_c/resource/msg__type_support.h.em

@##########################################################################
@# EmPy template for generating <msg>__type_support.h files for Connext
@#
@# Context:
@#  - spec (rosidl_parser.MessageSpecification)
@#    Parsed specification of the .msg file
@#  - pkg (string)
@#    name of the containing package; equivalent to spec.base_type.pkg_name
@#  - msg (string)
@#    name of the message; equivalent to spec.msg_name
@#  - type (string)
@#    full type of the message; equivalent to spec.base_type.type
@#  - subfolder (string)
@#    The subfolder / subnamespace of the message
@#    Either 'msg' or 'srv'
@#  - get_header_filename_from_msg_name (function)
@##########################################################################
@
#include <cassert>
#include <limits>

// Provides the definition of the rosidl_message_type_support_t struct as well
// as the Connext specific macros, e.g. ROSIDL_GET_TYPE_SUPPORT_FUNCTION.
#include <rosidl_generator_c/message_type_support.h>
// Ensure the correct version of the above header was included.
static_assert(USING_ROSIDL_TYPESUPPORT_CONNEXT_C, "expected Connext C message type support");
// Provides the rosidl_typesupport_connext_c__identifier symbol declaration.
#include <rosidl_typesupport_connext_c/identifier.h>
// Provides the definition of the message_type_support_callbacks_t struct.
#include <rosidl_typesupport_connext_cpp/message_type_support.h>

@{header_file_name = get_header_filename_from_msg_name(type)}@
#include "@(pkg)/@(subfolder)/@(header_file_name)__struct.h"
#include "@(pkg)/@(subfolder)/@(header_file_name)__functions.h"

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

@# // Include the message header for each non-primitive field.
#if defined(__cplusplus)
extern "C"
{
#endif

bool
@(spec.base_type.pkg_name)__@(spec.msg_name)__register_type(
  void * untyped_participant, const char * type_name);

bool
@(spec.base_type.pkg_name)__@(spec.msg_name)__convert_ros_to_dds(
  const void * untyped_ros_message, void * untyped_dds_message);

bool
@(spec.base_type.pkg_name)__@(spec.msg_name)__publish(
  void * dds_data_writer, const void * untyped_ros_message);

bool
@(spec.base_type.pkg_name)__@(spec.msg_name)__convert_dds_to_ros(
  const void * untyped_dds_message, void * untyped_ros_message);

bool
@(spec.base_type.pkg_name)__@(spec.msg_name)__take(
  void * dds_data_reader,
  bool ignore_local_publications,
  void * untyped_ros_message,
  bool * taken,
  void * sending_publication_handle);

#if defined(__cplusplus)
}
#endif
