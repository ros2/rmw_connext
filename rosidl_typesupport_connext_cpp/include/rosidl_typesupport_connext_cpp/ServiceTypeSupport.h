#ifndef __rmw_connext_cpp__ServiceTypeSupport__h__
#define __rmw_connext_cpp__ServiceTypeSupport__h__

#include <rmw/rmw.h>
#include <rosidl_generator_c/service_type_support.h>
#include "rosidl_generator_cpp/ServiceTypeSupport.h"

class DDSDomainParticipant;
class DDSDataReader;
struct DDS_SampleIdentity_t;

namespace rmw_connext_cpp
{

template<typename T>
const rosidl_service_type_support_t * get_service_type_support_handle();

}  // namespace rmw_connext_cpp

#endif  // __rmw_connext_cpp__ServiceTypeSupport__h__
