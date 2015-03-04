
#ifndef __rmw__get_type_support_handle__h__
#define __rmw__get_type_support_handle__h__

#include "rosidl_typesupport_connext_cpp/MessageTypeSupport.h"

namespace rmw
{
template<typename T>
const rosidl_generator_cpp::MessageTypeSupportHandle& get_type_support_handle()
{
  return rmw_connext_cpp::get_type_support_handle<T>();
}

}  // namespace rmw

#endif  // __rmw__get_type_support_handle__h__
