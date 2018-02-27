// generated from rosidl_typesupport_connext_cpp/resource/msg__type_support.cpp.em
// generated code does not contain a copyright notice

@#######################################################################
@# EmPy template for generating <msg>__type_support.cpp files
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
#include "@(spec.base_type.pkg_name)/@(subfolder)/@(get_header_filename_from_msg_name(spec.base_type.type))__rosidl_typesupport_connext_cpp.hpp"

#include <limits>
#include <stdexcept>

#include "rosidl_typesupport_cpp/message_type_support.hpp"

#include "rosidl_typesupport_connext_cpp/connext_static_cdr_stream.hpp"
#include "rosidl_typesupport_connext_cpp/identifier.hpp"
#include "rosidl_typesupport_connext_cpp/message_type_support.h"
#include "rosidl_typesupport_connext_cpp/message_type_support_decl.hpp"

// forward declaration of message dependencies and their conversion functions
@[for field in spec.fields]@
@[  if not field.type.is_primitive_type()]@
namespace @(field.type.pkg_name)
{
namespace msg
{
namespace dds_
{
class @(field.type.type)_;
}  // namespace dds_
namespace typesupport_connext_cpp
{
bool convert_ros_message_to_dds(
  const @(field.type.pkg_name)::msg::@(field.type.type) &,
  @(field.type.pkg_name)::msg::dds_::@(field.type.type)_ &);
bool convert_dds_message_to_ros(
  const @(field.type.pkg_name)::msg::dds_::@(field.type.type)_ &,
  @(field.type.pkg_name)::msg::@(field.type.type) &);
}  // namespace typesupport_connext_cpp
}  // namespace msg
}  // namespace @(field.type.pkg_name)

@[  end if]@
@[end for]@

namespace @(spec.base_type.pkg_name)
{

namespace @(subfolder)
{

namespace typesupport_connext_cpp
{

DDS_TypeCode *
get_type_code__@(spec.base_type.type)()
{
  return @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_TypeSupport::get_typecode();
}

bool
convert_ros_message_to_dds(
  const @(spec.base_type.pkg_name)::@(subfolder)::@(spec.base_type.type) & ros_message,
  @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_ & dds_message)
{
@[if not spec.fields]@
  (void)ros_message;
  (void)dds_message;
@[end if]@
@[for field in spec.fields]@
  // field.name @(field.name)
@[  if field.type.is_array]@
  {
@[    if field.type.array_size and not field.type.is_upper_bound]@
    size_t size = @(field.type.array_size);
@[    else]@
    size_t size = ros_message.@(field.name).size();
    if (size > (std::numeric_limits<DDS_Long>::max)()) {
      throw std::runtime_error("array size exceeds maximum DDS sequence size");
    }
@[      if field.type.is_upper_bound]@
    if (size > @(field.type.array_size)) {
      throw std::runtime_error("array size exceeds upper bound");
    }
@[      end if]@
    DDS_Long length = static_cast<DDS_Long>(size);
    if (length > dds_message.@(field.name)_.maximum()) {
      if (!dds_message.@(field.name)_.maximum(length)) {
        throw std::runtime_error("failed to set maximum of sequence");
      }
    }
    if (!dds_message.@(field.name)_.length(length)) {
      throw std::runtime_error("failed to set length of sequence");
    }
@[    end if]@
    for (size_t i = 0; i < size; i++) {
@[    if field.type.type == 'string']@
      DDS_String_free(dds_message.@(field.name)_[static_cast<DDS_Long>(i)]);
      dds_message.@(field.name)_[static_cast<DDS_Long>(i)] =
        DDS_String_dup(ros_message.@(field.name)[i].c_str());
@[    elif field.type.is_primitive_type()]@
      dds_message.@(field.name)_[static_cast<DDS_Long>(i)] =
        ros_message.@(field.name)[i];
@[    else]@
      if (
        !@(field.type.pkg_name)::msg::typesupport_connext_cpp::convert_ros_message_to_dds(
          ros_message.@(field.name)[i],
          dds_message.@(field.name)_[static_cast<DDS_Long>(i)]))
      {
        return false;
      }
@[    end if]@
    }
  }
@[  elif field.type.type == 'string']@
  DDS_String_free(dds_message.@(field.name)_);
  dds_message.@(field.name)_ =
    DDS_String_dup(ros_message.@(field.name).c_str());
@[  elif field.type.is_primitive_type()]@
  dds_message.@(field.name)_ =
    ros_message.@(field.name);
@[  else]@
  if (
    !@(field.type.pkg_name)::msg::typesupport_connext_cpp::convert_ros_message_to_dds(
      ros_message.@(field.name),
      dds_message.@(field.name)_))
  {
    return false;
  }
@[  end if]@

@[end for]@
  return true;
}

bool
convert_dds_message_to_ros(
  const @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_ & dds_message,
  @(spec.base_type.pkg_name)::@(subfolder)::@(spec.base_type.type) & ros_message)
{
@[if not spec.fields]@
  (void)ros_message;
  (void)dds_message;
@[end if]@
@[for field in spec.fields]@
  // field.name @(field.name)
@[  if field.type.is_array]@
  {
@[    if field.type.array_size and not field.type.is_upper_bound]@
    size_t size = @(field.type.array_size);
@[    else]@
    size_t size = dds_message.@(field.name)_.length();
    ros_message.@(field.name).resize(size);
@[    end if]@
    for (size_t i = 0; i < size; i++) {
@[    if field.type.is_primitive_type()]@
      ros_message.@(field.name)[i] =
        dds_message.@(field.name)_[static_cast<DDS_Long>(i)]@(' == DDS_BOOLEAN_TRUE' if field.type.type == 'bool' else '');
@[    else]@
      if (
        !@(field.type.pkg_name)::msg::typesupport_connext_cpp::convert_dds_message_to_ros(
          dds_message.@(field.name)_[static_cast<DDS_Long>(i)],
          ros_message.@(field.name)[i]))
      {
        return false;
      }
@[    end if]@
    }
  }
@[  elif field.type.is_primitive_type()]@
  ros_message.@(field.name) =
    dds_message.@(field.name)_@(' == DDS_BOOLEAN_TRUE' if field.type.type == 'bool' else '');
@[  else]@
  if (
    !@(field.type.pkg_name)::msg::typesupport_connext_cpp::convert_dds_message_to_ros(
      dds_message.@(field.name)_,
      ros_message.@(field.name)))
  {
    return false;
  }
@[  end if]@

@[end for]@
  return true;
}

bool
to_cdr_stream__@(spec.base_type.type)(
  const void * untyped_ros_message,
  ConnextStaticCDRStream * cdr_stream)
{
  if (!cdr_stream) {
    return false;
  }
  if (!untyped_ros_message) {
    return false;
  }

  // cast the untyped to the known ros message
  const @(spec.base_type.pkg_name)::@(subfolder)::@(spec.base_type.type) & ros_message =
    *(const @(spec.base_type.pkg_name)::@(subfolder)::@(spec.base_type.type) *)untyped_ros_message;

  // create a respective connext dds type
  @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_ * dds_message = @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_TypeSupport::create_data();
  if (!dds_message) {
    return false;
  }

  // convert ros to dds
  if (!convert_ros_message_to_dds(ros_message, *dds_message)) {
    return false;
  }

  // call the serialize function for the first time to get the expected length of the message
  if (@(spec.base_type.type)_Plugin_serialize_to_cdr_buffer(NULL, &cdr_stream->message_length, dds_message) != RTI_TRUE) {
    return false;
  }
  // allocate enough memory for the CDR stream
  // TODO(karsten1987): This allocation has to be preallocated
  // or at least bring in a custom allocator
  cdr_stream->raw_message = (char *)malloc(sizeof(char) * cdr_stream->message_length);
  // call the function again and fill the buffer this time
  if (@(spec.base_type.type)_Plugin_serialize_to_cdr_buffer(cdr_stream->raw_message, &cdr_stream->message_length, dds_message) != RTI_TRUE) {
    return false;
  }
  if (@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_TypeSupport::delete_data(dds_message) != DDS_RETCODE_OK){
    return false;
  }
  return true;
}

bool
to_message__@(spec.base_type.type)(
  const ConnextStaticCDRStream * cdr_stream,
  void * untyped_ros_message)
{
  if (!cdr_stream) {
    return false;
  }
  if (!cdr_stream->raw_message) {
    fprintf(stderr, "cdr stream doesn't contain data\n");
  }
  if (!untyped_ros_message) {
    return false;
  }

  @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_ * dds_message = @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_TypeSupport::create_data();
  if (@(spec.base_type.type)_Plugin_deserialize_from_cdr_buffer(dds_message, cdr_stream->raw_message, cdr_stream->message_length) != RTI_TRUE) {
    fprintf(stderr, "deserialize from cdr buffer failed\n");
    return false;
  }

  @(spec.base_type.pkg_name)::@(subfolder)::@(spec.base_type.type) & ros_message =
    *(@(spec.base_type.pkg_name)::@(subfolder)::@(spec.base_type.type) *)untyped_ros_message;
  bool success = convert_dds_message_to_ros(*dds_message, ros_message);
  if (@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_TypeSupport::delete_data(dds_message) != DDS_RETCODE_OK){
    return false;
  }
  return success;
}

static message_type_support_callbacks_t callbacks = {
  "@(spec.base_type.pkg_name)",
  "@(spec.base_type.type)",
  &get_type_code__@(spec.base_type.type),
  nullptr,
  nullptr,
  &to_cdr_stream__@(spec.base_type.type),
  &to_message__@(spec.base_type.type)
};

static rosidl_message_type_support_t handle = {
  rosidl_typesupport_connext_cpp::typesupport_identifier,
  &callbacks,
  get_message_typesupport_handle_function,
};

}  // namespace typesupport_connext_cpp

}  // namespace @(subfolder)

}  // namespace @(spec.base_type.pkg_name)

namespace rosidl_typesupport_connext_cpp
{

template<>
ROSIDL_TYPESUPPORT_CONNEXT_CPP_EXPORT_@(spec.base_type.pkg_name)
const rosidl_message_type_support_t *
get_message_type_support_handle<@(spec.base_type.pkg_name)::@(subfolder)::@(spec.base_type.type)>()
{
  return &@(spec.base_type.pkg_name)::@(subfolder)::typesupport_connext_cpp::handle;
}

}  // namespace rosidl_typesupport_connext_cpp

#ifdef __cplusplus
extern "C"
{
#endif

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_connext_cpp, @(spec.base_type.pkg_name), @(subfolder), @(spec.base_type.type))() {
  return &@(spec.base_type.pkg_name)::@(subfolder)::typesupport_connext_cpp::handle;
}

#ifdef __cplusplus
}
#endif
