
#ifndef __ros_middleware_interface__get_type_support_handle__h__
#define __ros_middleware_interface__get_type_support_handle__h__

#include "ros_middleware_connext_cpp/MessageTypeSupport.h"

namespace ros_middleware_interface
{
template<typename T>
const rosidl_generator_cpp::MessageTypeSupportHandle& get_type_support_handle()
{
  return ros_middleware_connext_cpp::get_type_support_handle<T>();
}

}  // namespace ros_middleware_interface

#endif  // __ros_middleware_interface__get_type_support_handle__h__
