

#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_cpp/identifier.hpp"
/*
 * Take an event from the event handle.
 *
 * \param event subscription object to take from
 * \param taken boolean flag indicating if an event was taken or not
 * \return `RMW_RET_OK` if successful, or
 * \return `RMW_RET_BAD_ALLOC` if memory allocation failed, or
 * \return `RMW_RET_ERROR` if an unexpected error occurs.
 */
rmw_ret_t
rmw_take_event(
  const rmw_event_t * event_handle,
  void * event,
  bool * taken);