#ifndef ROSIDL_TYPESUPPORT_CONNEXT_CPP_ROSIDL_TYPESUPPORT_CONNEXT_CPP_MESSAGE_TYPE_SUPPORT_H_
#define ROSIDL_TYPESUPPORT_CONNEXT_CPP_ROSIDL_TYPESUPPORT_CONNEXT_CPP_MESSAGE_TYPE_SUPPORT_H_

#include "rosidl_generator_c/message_type_support.h"

typedef struct message_type_support_callbacks_t
{
  const char * package_name;
  const char * message_name;
  // Function to register type with given dds_participant
  void (*register_type)(void * dds_participant, const char * type_name);
  // Function to publish a ROS message with a given DDS data_writer
  void (*publish)(void * dds_data_writer, const void * ros_message);
  // Function to take a ROS message from a dds data reader
  bool (*take)(void * dds_data_reader, void * ros_message);
} message_type_support_callbacks_t;

#endif  /* ROSIDL_TYPESUPPORT_CONNEXT_CPP_ROSIDL_TYPESUPPORT_CONNEXT_CPP_MESSAGE_TYPE_SUPPORT_H_ */
