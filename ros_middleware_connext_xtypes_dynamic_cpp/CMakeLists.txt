cmake_minimum_required(VERSION 2.8.3)

project(ros_middleware_connext_xtypes_dynamic_cpp)

find_package(ament_cmake REQUIRED)

find_package(connext_cmake_module REQUIRED)
find_package(Connext MODULE)
if(NOT Connext_FOUND)
  ament_package()
  message(WARNING "Could not find RTI Connext - skipping '${PROJECT_NAME}'")
  return()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(ros_middleware_interface REQUIRED)
find_package(rosidl_generator_cpp REQUIRED)

find_package(rosidl_typesupport_introspection_cpp REQUIRED)
ament_export_dependencies(rosidl_typesupport_introspection_cpp)
include_directories(${rosidl_typesupport_introspection_cpp_INCLUDE_DIRS})

ament_export_dependencies(ros_middleware_interface rosidl_generator_cpp)

ament_export_libraries(ros_middleware_connext_xtypes_dynamic_cpp dl pthread)

register_ros_middleware_implementation()

ament_package()

include_directories(${ros_middleware_interface_INCLUDE_DIRS} ${rosidl_generator_cpp_INCLUDE_DIRS} ${Connext_INCLUDE_DIRS})
link_directories(${Connext_LIBRARY_DIRS})
add_definitions(${Connext_DEFINITIONS})
add_library(ros_middleware_connext_xtypes_dynamic_cpp SHARED src/functions.cpp)
target_link_libraries(ros_middleware_connext_xtypes_dynamic_cpp ${rosidl_typesupport_introspection_cpp_LIBRARIES} ${Connext_LIBRARIES})

install(
  TARGETS ros_middleware_connext_xtypes_dynamic_cpp
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
