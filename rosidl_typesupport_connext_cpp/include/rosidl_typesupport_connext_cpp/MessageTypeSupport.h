
#ifndef __rmw_connext_cpp__MessageTypeSupport__h__
#define __rmw_connext_cpp__MessageTypeSupport__h__

#include <rosidl_generator_c/message_type_support.h>
#include "rosidl_generator_cpp/MessageTypeSupport.h"

class DDSDomainParticipant;
class DDSDataWriter;
class DDSDataReader;

namespace rmw_connext_cpp
{

template<typename T>
const rosidl_message_type_support_t * get_type_support_handle();

}  // namespace rmw_connext_cpp

#endif  // __rmw_connext_cpp__MessageTypeSupport__h__
