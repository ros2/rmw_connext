// generated from rosidl_typesupport_connext_c/resource/msg__type_support_c.cpp.em
// generated code does not contain a copyright notice

@##########################################################################
@# EmPy template for generating <msg>__type_support_c.cpp files for Connext
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
#include "@(spec.base_type.pkg_name)/@(subfolder)/@(get_header_filename_from_msg_name(spec.base_type.type))__rosidl_typesupport_connext_c.h"

#include <cassert>
#include <limits>

// Provides the rosidl_typesupport_connext_c__identifier symbol declaration.
#include "rosidl_typesupport_connext_c/identifier.h"
// Provides the definition of the message_type_support_callbacks_t struct.
#include "rosidl_typesupport_connext_cpp/message_type_support.h"

#include "@(pkg)/msg/rosidl_typesupport_connext_c__visibility_control.h"
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

// includes and forward declarations of message dependencies and their conversion functions

@# // Include the message header for each non-primitive field.
#if defined(__cplusplus)
extern "C"
{
#endif

@{
includes = {}
for field in spec.fields:
    keys = set([])
    if field.type.is_primitive_type():
        if field.type.is_array:
            keys.add('rosidl_generator_c/primitives_array.h')
            keys.add('rosidl_generator_c/primitives_array_functions.h')
        if field.type.type == 'string':
            keys.add('rosidl_generator_c/string.h')
            keys.add('rosidl_generator_c/string_functions.h')
    else:
        header_file_name = get_header_filename_from_msg_name(field.type.type)
        keys.add('%s/msg/%s__functions.h' % (field.type.pkg_name, header_file_name))
    for key in keys:
        if key not in includes:
            includes[key] = set([])
        includes[key].add(field.name)
}@
@[for key in sorted(includes.keys())]@
#include "@(key)"  // @(', '.join(includes[key]))
@[end for]@

// forward declare type support functions
@{
forward_declares = {}
for field in spec.fields:
    if not field.type.is_primitive_type():
        key = (field.type.pkg_name, field.type.type)
        if key not in includes:
            forward_declares[key] = set([])
        forward_declares[key].add(field.name)
}@
@[for key in sorted(forward_declares.keys())]@
@[  if key[0] != pkg]@
ROSIDL_TYPESUPPORT_CONNEXT_C_IMPORT_@(pkg)
@[  end if]@
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_connext_c, @(key[0]), msg, @(key[1]))();
@[end for]@

@# // Make callback functions specific to this message type.

@{
__dds_msg_type_prefix = "{0}::{1}::dds_::{2}_".format(
  spec.base_type.pkg_name, subfolder, spec.base_type.type)
}@
using __dds_msg_type = @(__dds_msg_type_prefix);
using __ros_msg_type = @(pkg)__@(subfolder)__@(type);

static bool
register_type(void * untyped_participant, const char * type_name)
{
  if (!untyped_participant) {
    fprintf(stderr, "untyped participant handle is null\n");
    return false;
  }
  if (!type_name) {
    fprintf(stderr, "type name handle is null\n");
    return false;
  }
  DDSDomainParticipant * participant = static_cast<DDSDomainParticipant *>(untyped_participant);

  DDS_ReturnCode_t status =
    @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_TypeSupport::register_type(participant, type_name);
  switch (status) {
    case DDS_RETCODE_ERROR:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_TypeSupport::register_type: "
        "an internal error has occurred\n");
      return false;
    case DDS_RETCODE_BAD_PARAMETER:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_TypeSupport::register_type: "
        "bad domain participant or type name parameter\n");
      return false;
    case DDS_RETCODE_OUT_OF_RESOURCES:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_TypeSupport::register_type: "
        "out of resources\n");
      return false;
    case DDS_RETCODE_PRECONDITION_NOT_MET:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_TypeSupport::register_type: "
        "already registered with a different TypeSupport class\n");
      return false;
    case DDS_RETCODE_OK:
      return true;
    default:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_TypeSupport::register_type: unknown return code\n");
  }
  return false;
}

static bool
convert_ros_to_dds(const void * untyped_ros_message, void * untyped_dds_message)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  if (!untyped_dds_message) {
    fprintf(stderr, "dds message handle is null\n");
    return false;
  }
  const __ros_msg_type * ros_message = static_cast<const __ros_msg_type *>(untyped_ros_message);
  __dds_msg_type * dds_message = static_cast<__dds_msg_type *>(untyped_dds_message);
@[if not spec.fields]@
  // No fields is a no-op.
  (void)dds_message;
  (void)ros_message;
@[end if]@
@[for field in spec.fields]@
  // Field name: @(field.name)
  {
@[  if not field.type.is_primitive_type()]@
    const message_type_support_callbacks_t * @(field.type.pkg_name)__msg__@(field.type.type)__callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_connext_c, @(field.type.pkg_name), msg, @(field.type.type)
      )()->data);
@[  end if]@
@[  if field.type.is_array]@
@[    if field.type.array_size and not field.type.is_upper_bound]@
    size_t size = @(field.type.array_size);
@[    else]@
    size_t size = ros_message->@(field.name).size;
    if (size > (std::numeric_limits<DDS_Long>::max)()) {
      fprintf(stderr, "array size exceeds maximum DDS sequence size\n");
      return false;
    }
@[      if field.type.is_upper_bound]@
    if (size > @(field.type.array_size)) {
      fprintf(stderr, "array size exceeds upper bound\n");
      return false;
    }
@[      end if]@
    DDS_Long length = static_cast<DDS_Long>(size);
    if (length > dds_message->@(field.name)_.maximum()) {
      if (!dds_message->@(field.name)_.maximum(length)) {
        fprintf(stderr, "failed to set maximum of sequence\n");
        return false;
      }
    }
    if (!dds_message->@(field.name)_.length(length)) {
      fprintf(stderr, "failed to set length of sequence\n");
      return false;
    }
@[    end if]@
    for (DDS_Long i = 0; i < static_cast<DDS_Long>(size); ++i) {
@[    if field.type.array_size and not field.type.is_upper_bound]@
      auto & ros_i = ros_message->@(field.name)[i];
@[    else]@
      auto & ros_i = ros_message->@(field.name).data[i];
@[    end if]@
@[    if field.type.type == 'string']@
      const rosidl_generator_c__String * str = &ros_i;
      if (str->capacity == 0 || str->capacity <= str->size) {
        fprintf(stderr, "string capacity not greater than size\n");
        return false;
      }
      if (str->data[str->size] != '\0') {
        fprintf(stderr, "string not null-terminated\n");
        return false;
      }
      dds_message->@(field.name)_[static_cast<DDS_Long>(i)] = DDS_String_dup(str->data);
@[    elif field.type.type == 'bool']@
      dds_message->@(field.name)_[i] = 1 ? ros_i : 0;
@[    elif field.type.is_primitive_type()]@
      dds_message->@(field.name)_[i] = ros_i;
@[    else]@
      if (!@(field.type.pkg_name)__msg__@(field.type.type)__callbacks->convert_ros_to_dds(
          &ros_i, &dds_message->@(field.name)_[i]))
      {
        return false;
      }
@[    end if]@
    }
@[  elif field.type.type == 'string']@
    const rosidl_generator_c__String * str = &ros_message->@(field.name);
    if (str->capacity == 0 || str->capacity <= str->size) {
      fprintf(stderr, "string capacity not greater than size\n");
      return false;
    }
    if (str->data[str->size] != '\0') {
      fprintf(stderr, "string not null-terminated\n");
      return false;
    }
    dds_message->@(field.name)_ = DDS_String_dup(str->data);
@[  elif field.type.is_primitive_type()]@
    dds_message->@(field.name)_ = ros_message->@(field.name);
@[  else]@
    if (!@(field.type.pkg_name)__msg__@(field.type.type)__callbacks->convert_ros_to_dds(
        &ros_message->@(field.name), &dds_message->@(field.name)_))
    {
      return false;
    }
@[  end if]@
  }

@[end for]@
  return true;
}

static bool
publish(void * dds_data_writer, const void * untyped_ros_message)
{
  if (!dds_data_writer) {
    fprintf(stderr, "data writer handle is null\n");
    return false;
  }
  const __ros_msg_type * ros_message = static_cast<const __ros_msg_type *>(untyped_ros_message);
  if (!ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }

  DDSDataWriter * topic_writer = static_cast<DDSDataWriter *>(dds_data_writer);

  __dds_msg_type dds_message;
  if (!convert_ros_to_dds(ros_message, &dds_message)) {
    return false;
  }
  @(pkg)::@(subfolder)::dds_::@(type)_DataWriter * data_writer =
    @(pkg)::@(subfolder)::dds_::@(type)_DataWriter::narrow(topic_writer);
  DDS_ReturnCode_t status = data_writer->write(dds_message, DDS_HANDLE_NIL);

  switch (status) {
    case DDS_RETCODE_ERROR:
      fprintf(stderr, "@(pkg)::@(subfolder)::dds_::@(type)_DataWriter.write: "
        "an internal error has occurred\n");
      return false;
    case DDS_RETCODE_BAD_PARAMETER:
      fprintf(stderr, "@(pkg)::@(subfolder)::dds_::@(type)_DataWriter.write: "
        "bad handle or instance_data parameter\n");
      return false;
    case DDS_RETCODE_ALREADY_DELETED:
      fprintf(stderr, "@(pkg)::@(subfolder)::dds_::@(type)_DataWriter.write: "
        "this @(__dds_msg_type_prefix)DataWriter has already been deleted\n");
      return false;
    case DDS_RETCODE_OUT_OF_RESOURCES:
      fprintf(stderr, "@(pkg)::@(subfolder)::dds_::@(type)_DataWriter.write: "
        "out of resources\n");
      return false;
    case DDS_RETCODE_NOT_ENABLED:
      fprintf(stderr, "@(pkg)::@(subfolder)::dds_::@(type)_DataWriter.write: "
        "this @(__dds_msg_type_prefix)DataWriter is not enabled\n");
      return false;
    case DDS_RETCODE_PRECONDITION_NOT_MET:
      fprintf(stderr, "@(pkg)::@(subfolder)::dds_::@(type)_DataWriter.write: "
        "the handle has not been registered with this @(__dds_msg_type_prefix)DataWriter\n");
      return false;
    case DDS_RETCODE_TIMEOUT:
      fprintf(stderr, "@(pkg)::@(subfolder)::dds_::@(type)_DataWriter.write: "
        "writing resulted in blocking and then exceeded the timeout set by the "
        "max_blocking_time of the ReliabilityQosPolicy\n");
      return false;
    case DDS_RETCODE_OK:
      return true;
    default:
      fprintf(stderr, "@(pkg)::@(subfolder)::dds_::@(type)_DataWriter.write: "
        "unknown return code\n");
  }
  return false;
}

static bool
convert_dds_to_ros(const void * untyped_dds_message, void * untyped_ros_message)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  if (!untyped_dds_message) {
    fprintf(stderr, "dds message handle is null\n");
    return false;
  }
  const __dds_msg_type * dds_message = static_cast<const __dds_msg_type *>(untyped_dds_message);
  __ros_msg_type * ros_message = static_cast<__ros_msg_type *>(untyped_ros_message);
@[if not spec.fields]@
  // No fields is a no-op.
  (void)dds_message;
  (void)ros_message;
@[end if]@
@[for field in spec.fields]@
  // Field name: @(field.name)
  {
@[  if field.type.is_array]@
@[    if field.type.array_size and not field.type.is_upper_bound]@
    DDS_Long size = @(field.type.array_size);
@[    else]@
@{
if field.type.type == 'string':
    array_init = 'rosidl_generator_c__String__Array__init'
    array_fini = 'rosidl_generator_c__String__Array__fini'
elif field.type.is_primitive_type():
    array_init = 'rosidl_generator_c__{field.type.type}__Array__init'.format(**locals())
    array_fini = 'rosidl_generator_c__{field.type.type}__Array__fini'.format(**locals())
else:
    array_init = '{field.type.pkg_name}__msg__{field.type.type}__Array__init'.format(**locals())
    array_fini = '{field.type.pkg_name}__msg__{field.type.type}__Array__fini'.format(**locals())
}@
    DDS_Long size = dds_message->@(field.name)_.length();
    if (ros_message->@(field.name).data) {
      @(array_fini)(&ros_message->@(field.name));
    }
    if (!@(array_init)(&ros_message->@(field.name), size)) {
      return "failed to create array for field '@(field.name)'";
    }
@[    end if]@
    for (DDS_Long i = 0; i < size; i++) {
@[    if field.type.array_size and not field.type.is_upper_bound]@
      auto & ros_i = ros_message->@(field.name)[i];
@[    else]@
      auto & ros_i = ros_message->@(field.name).data[i];
@[    end if]@
@[    if field.type.type == 'bool']@
      ros_i = (dds_message->@(field.name)_[i] != 0);
@[    elif field.type.type == 'string']@
      if (!ros_i.data) {
        rosidl_generator_c__String__init(&ros_i);
      }
      bool succeeded = rosidl_generator_c__String__assign(
        &ros_i,
        dds_message->@(field.name)_[i]);
      if (!succeeded) {
        fprintf(stderr, "failed to assign string into field '@(field.name)'\n");
        return false;
      }
@[    elif field.type.is_primitive_type()]@
      ros_i = dds_message->@(field.name)_[i];
@[    else]@
      const rosidl_message_type_support_t * ts =
        ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_connext_c, @(field.type.pkg_name), msg, @(field.type.type))();
      const message_type_support_callbacks_t * callbacks =
        static_cast<const message_type_support_callbacks_t *>(ts->data);
      callbacks->convert_dds_to_ros(&dds_message->@(field.name)_[i], &ros_i);
@[    end if]@
    }
@[  elif field.type.type == 'string']@
    if (!ros_message->@(field.name).data) {
      rosidl_generator_c__String__init(&ros_message->@(field.name));
    }
    bool succeeded = rosidl_generator_c__String__assign(
      &ros_message->@(field.name),
      dds_message->@(field.name)_);
    if (!succeeded) {
      fprintf(stderr, "failed to assign string into field '@(field.name)'\n");
      return false;
    }
@[  elif field.type.is_primitive_type()]@
    ros_message->@(field.name) = dds_message->@(field.name)_@(' == static_cast<DDS_Boolean>(true)' if field.type.type == 'bool' else '');
@[  else]@
    const rosidl_message_type_support_t * ts =
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_connext_c, @(field.type.pkg_name), msg, @(field.type.type))();
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(ts->data);
    callbacks->convert_dds_to_ros(&dds_message->@(field.name)_, &ros_message->@(field.name));
@[  end if]@
  }

@[end for]@
  return true;
}

static bool
take(
  void * dds_data_reader,
  bool ignore_local_publications,
  void * untyped_ros_message,
  bool * taken,
  void * sending_publication_handle)
{
  if (untyped_ros_message == 0) {
    fprintf(stderr, "invalid ros message pointer\n");
    return false;
  }

  DDSDataReader * topic_reader = static_cast<DDSDataReader *>(dds_data_reader);

  @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader * data_reader =
    @(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader::narrow(topic_reader);

  @(__dds_msg_type_prefix)Seq dds_messages;
  DDS_SampleInfoSeq sample_infos;
  DDS_ReturnCode_t status = data_reader->take(
    dds_messages,
    sample_infos,
    1,
    DDS_ANY_SAMPLE_STATE,
    DDS_ANY_VIEW_STATE,
    DDS_ANY_INSTANCE_STATE);

  bool ignore_sample = false;

  switch (status) {
    case DDS_RETCODE_ERROR:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader.take: "
        "an internal error has occurred\n");
      goto finally;
    case DDS_RETCODE_ALREADY_DELETED:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader.take: "
        "this @(__dds_msg_type_prefix)DataReader has already been deleted\n");
      goto finally;
    case DDS_RETCODE_OUT_OF_RESOURCES:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader.take: "
        "out of resources\n");
      goto finally;
    case DDS_RETCODE_NOT_ENABLED:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader.take: "
        "this @(__dds_msg_type_prefix)DataReader is not enabled\n");
      goto finally;
    case DDS_RETCODE_PRECONDITION_NOT_MET:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader.take: "
        "a precondition is not met, one of: "
        "max_samples > maximum and max_samples != LENGTH_UNLIMITED, or "
        "the two sequences do not have matching parameters (length, maximum, release), or "
        "maximum > 0 and release is false.\n");
      goto finally;
    case DDS_RETCODE_NO_DATA:
      *taken = false;
      goto finally;
    case DDS_RETCODE_OK:
      break;
    default:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader.take: "
        "unknown return code\n");
      goto finally;
  }

  {
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
      // This is nullptr when being used with plain rmw_take, so check first.
      if (sending_publication_handle) {
        *static_cast<DDS_InstanceHandle_t *>(sending_publication_handle) =
          sample_info.publication_handle;
      }
    }
  }

  if (!ignore_sample) {
    if (!convert_dds_to_ros(&dds_messages[0], untyped_ros_message)) {
      goto finally;
    }
    *taken = true;
  } else {
    *taken = false;
  }

finally:
  // Ensure the loan is returned.
  status = data_reader->return_loan(dds_messages, sample_infos);
  switch (status) {
    case DDS_RETCODE_ERROR:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader.return_loan: "
        "an internal error has occurred\n");
      return false;
    case DDS_RETCODE_ALREADY_DELETED:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader.return_loan: "
        "this @(__dds_msg_type_prefix)DataReader has already been deleted\n");
      return false;
    case DDS_RETCODE_OUT_OF_RESOURCES:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader.return_loan: "
        "out of resources\n");
      return false;
    case DDS_RETCODE_NOT_ENABLED:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader.return_loan: "
        "this @(__dds_msg_type_prefix)DataReader is not enabled\n");
      return false;
    case DDS_RETCODE_PRECONDITION_NOT_MET:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader.return_loan: "
        "a precondition is not met, one of: "
        "the data_values and info_seq do not belong to a single related pair, or "
        "the data_values and info_seq were not obtained from this "
        "@(__dds_msg_type_prefix)DataReader\n");
      return false;
    case DDS_RETCODE_OK:
      return true;
    default:
      fprintf(stderr, "@(spec.base_type.pkg_name)::@(subfolder)::dds_::@(spec.base_type.type)_DataReader.return_loan: "
        "unknown return code\n");
  }

  return false;
}
@
@# // Collect the callback functions and provide a function to get the type support struct.

static message_type_support_callbacks_t __callbacks = {
  "@(pkg)",  // package_name
  "@(msg)",  // message_name
  register_type,  // register_type
  publish,  // publish
  take,  // take
  convert_ros_to_dds,  // convert_ros_to_dds
  convert_dds_to_ros,  // convert_dds_to_ros
};

static rosidl_message_type_support_t __type_support = {
  rosidl_typesupport_connext_c__identifier,
  &__callbacks,
  get_message_typesupport_handle_function,
};

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_connext_c, @(pkg), @(subfolder), @(msg))() {
  return &__type_support;
}

#if defined(__cplusplus)
}
#endif
