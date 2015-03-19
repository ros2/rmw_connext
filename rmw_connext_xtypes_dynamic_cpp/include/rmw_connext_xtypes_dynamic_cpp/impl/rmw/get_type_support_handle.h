
#ifndef __rmw__get_type_support_handle__h__
#define __rmw__get_type_support_handle__h__

#include <iostream>
#include "rosidl_typesupport_connext_cpp/MessageTypeSupport.h"

extern "C" {
  extern const char * rti_connext_dynamic_identifier;
}

namespace rmw
{
template<typename T>
const rosidl_message_type_support_t * get_type_support_handle()
{
  std::cout << rti_connext_dynamic_identifier << std::endl;
  return rmw_connext_cpp::get_type_support_handle<T>();
}

}  // namespace rmw

#endif  // __rmw__get_type_support_handle__h__
