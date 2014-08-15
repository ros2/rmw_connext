cmake_minimum_required(VERSION 2.8.3)

set(CMAKE_CXX_FLAGS "-std=c++0x")

project(ros_middleware_connext_xtypes_dynamic_cpp)

find_package(ament_cmake REQUIRED)
find_package(ros_middleware_interface REQUIRED)
find_package(rosidl_generator_cpp REQUIRED)

find_package(ndds_cpp REQUIRED)
set(CONNEXT_INCLUDE_DIRS ${ndds_cpp_INCLUDE_DIRS})
set(CONNEXT_LIBRARIES ${ndds_cpp_LIBRARIES})
set(CONNEXT_DEFINITIONS ${ndds_cpp_DEFINITIONS})


find_package(rosidl_typesupport_introspection_cpp REQUIRED)
ament_export_dependencies(rosidl_typesupport_introspection_cpp)
include_directories(${rosidl_typesupport_introspection_cpp_INCLUDE_DIRS})

ament_export_dependencies(ros_middleware_interface rosidl_generator_cpp)

ament_export_libraries(ros_middleware_connext_xtypes_dynamic_cpp dl pthread)

register_ros_middleware_implementation()

ament_package()

include_directories(${ros_middleware_interface_INCLUDE_DIRS} ${rosidl_generator_cpp_INCLUDE_DIRS} ${CONNEXT_INCLUDE_DIRS})
add_definitions(${CONNEXT_DEFINITIONS})
add_library(ros_middleware_connext_xtypes_dynamic_cpp SHARED src/functions.cpp)
target_link_libraries(ros_middleware_connext_xtypes_dynamic_cpp ${rosidl_typesupport_introspection_cpp_LIBRARIES} ${CONNEXT_LIBRARIES})

install(
  TARGETS ros_middleware_connext_xtypes_dynamic_cpp
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
