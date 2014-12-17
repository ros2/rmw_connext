
#ifndef __ros_middleware_connext_cpp__ServiceTypeSupport__h__
#define __ros_middleware_connext_cpp__ServiceTypeSupport__h__

#include "rosidl_generator_cpp/ServiceTypeSupport.h"

class DDSDomainParticipant;
class DDSDataReader;
struct DDS_SampleIdentity_t;

namespace ros_middleware_interface
{

extern const char * _rti_connext_identifier;

}  // namespace ros_middleware_interface

namespace ros_middleware_connext_cpp
{

typedef struct ServiceTypeSupportCallbacks {
  const char * _package_name;
  const char * _message_name;
  void* (*_create_requester)(DDSDomainParticipant * participant, const char * service_name);
  void* (*_create_replier)(DDSDomainParticipant * participant, const char * service_name, DDSDataReader **reader);
  void (*_send_request)(void * requester, const void * ros_request);
  bool (*_receive_response)(void * requester, void * ros_response);
  bool (*_take_request)(void * replier, void * ros_request);
  void (*_send_response)(void * replier, const void * ros_request, const void * ros_response);
} ServiceTypeSupportCallbacks;

template<typename T>
const rosidl_generator_cpp::ServiceTypeSupportHandle& get_service_type_support_handle();

}  // namespace ros_middleware_connext_cpp

#endif  // __ros_middleware_connext_cpp__ServiceTypeSupport__h__
