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

bool
register_type__@(spec.base_type.type)(
  void * untyped_participant,
  const char * type_name)
{
  DDSDomainParticipant * participant = static_cast<DDSDomainParticipant *>(untyped_participant);
  DDS_ReturnCode_t status =
    @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_TypeSupport::register_type(participant, type_name);

  return status == DDS_RETCODE_OK;
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
publish__@(spec.base_type.type)(
  void * untyped_topic_writer,
  ConnextStaticCDRStream * cdr_stream)
{
  bool success = true;
  DDSDataWriter * topic_writer = static_cast<DDSDataWriter *>(untyped_topic_writer);

  @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataWriter * data_writer = @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataWriter::narrow(topic_writer);
  if (!data_writer) {
    fprintf(stderr, "failed to narrow data writer\n");
    success = false;
  } else {
    @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_ * sample  = reinterpret_cast<@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_ *>(cdr_stream);
    DDS_ReturnCode_t status = data_writer->write(*sample, DDS_HANDLE_NIL);
    success = status == DDS_RETCODE_OK;
  }

  return success;
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
take__@(spec.base_type.type)(
  void * untyped_topic_reader,
  bool ignore_local_publications,
  ConnextStaticCDRStream * cdr_stream,
  bool * taken,
  void * sending_publication_handle)
{
  if (!untyped_topic_reader) {
    throw std::runtime_error("topic reader handle is null");
  }
  if (!cdr_stream) {
    throw std::runtime_error("cdr stream handle is null");
  }
  if (!taken) {
    throw std::runtime_error("taken handle is null");
  }

  DDSDataReader * topic_reader = static_cast<DDSDataReader *>(untyped_topic_reader);

  @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader * data_reader =
    @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader::narrow(topic_reader);
  if (!data_reader) {
    fprintf(stderr, "failed to narrow data reader\n");
    return false;
  }

  @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_Seq dds_messages;
  DDS_SampleInfoSeq sample_infos;
  DDS_ReturnCode_t status = data_reader->take(
    dds_messages,
    sample_infos,
    1,
    DDS_ANY_SAMPLE_STATE,
    DDS_ANY_VIEW_STATE,
    DDS_ANY_INSTANCE_STATE);
  if (status == DDS_RETCODE_NO_DATA) {
    *taken = false;
    return true;
  }
  if (status != DDS_RETCODE_OK) {
    fprintf(stderr, "take failed with status = %d\n", status);
    return false;
  }

  bool ignore_sample = false;
  DDS_SampleInfo & sample_info = sample_infos[0];
  if (!sample_info.valid_data) {
    // skip sample without data
    ignore_sample = true;
  } else if (ignore_local_publications) {
    // compare the lower 12 octets of the guids from the sender and this receiver
    // if they are equal the sample has been sent from this process and should be ignored
    DDS_GUID_t sender_guid = sample_info.original_publication_virtual_guid;
    DDS_InstanceHandle_t receiver_instance_handle = topic_reader->get_instance_handle();
    ignore_sample = true;
    for (size_t i = 0; i < 12; ++i) {
      DDS_Octet * sender_element = &(sender_guid.value[i]);
      DDS_Octet * receiver_element = &(reinterpret_cast<DDS_Octet *>(&receiver_instance_handle)[i]);
      if (*sender_element != *receiver_element) {
        ignore_sample = false;
        break;
      }
    }
  }
  if (sample_info.valid_data && sending_publication_handle) {
    *static_cast<DDS_InstanceHandle_t *>(sending_publication_handle) =
      sample_info.publication_handle;
  }

  if (!ignore_sample) {
    ConnextStaticCDRStream * input_stream = reinterpret_cast<ConnextStaticCDRStream *>(&dds_messages[0]);
    cdr_stream->raw_message = input_stream->raw_message;
    cdr_stream->message_length = input_stream->message_length;
    fprintf(stderr, "Received stream of length %u\n", cdr_stream->message_length);
    *taken = true;
  } else {
    *taken = false;
  }
  data_reader->return_loan(dds_messages, sample_infos);

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
  fprintf(stderr, "message length: %d\n", cdr_stream->message_length);
  // allocate enough memory for the CDR stream
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
  &register_type__@(spec.base_type.type),
  &publish__@(spec.base_type.type),
  &take__@(spec.base_type.type),
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
