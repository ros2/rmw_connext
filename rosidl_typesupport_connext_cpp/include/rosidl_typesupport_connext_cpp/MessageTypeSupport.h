
#ifndef __ros_middleware_connext_cpp__MessageTypeSupport__h__
#define __ros_middleware_connext_cpp__MessageTypeSupport__h__

#include "rosidl_generator_cpp/MessageTypeSupport.h"

class DDSDomainParticipant;
class DDSDataWriter;
class DDSDataReader;

namespace ros_middleware_interface
{

extern const char * _rti_connext_identifier;

}  // namespace ros_middleware_interface

namespace ros_middleware_connext_cpp
{

typedef struct MessageTypeSupportCallbacks {
  const char * _package_name;
  const char * _message_name;
  void (*_register_type)(DDSDomainParticipant * participant, const char * type_name);
  void (*_publish)(DDSDataWriter * topic_writer, const void * ros_message);
  bool (*_take)(DDSDataReader * topic_reader, void * ros_message);
  void (*_create_client)(const char * service_name);
} MessageTypeSupportCallbacks;

template<typename T>
const rosidl_generator_cpp::MessageTypeSupportHandle& get_type_support_handle();

}  // namespace ros_middleware_connext_cpp

#endif  // __ros_middleware_connext_cpp__MessageTypeSupport__h__
