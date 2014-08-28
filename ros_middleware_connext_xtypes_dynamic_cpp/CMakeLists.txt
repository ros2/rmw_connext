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

ament_export_dependencies(ros_middleware_interface rosidl_generator_cpp)

register_ros_middleware_implementation()

link_directories(${Connext_LIBRARY_DIRS})
add_library(ros_middleware_connext_xtypes_dynamic_cpp SHARED src/functions.cpp)
ament_target_dependencies(ros_middleware_connext_xtypes_dynamic_cpp
  "rosidl_typesupport_introspection_cpp"
  "ros_middleware_interface"
  "rosidl_generator_cpp"
  "Connext")

ament_export_libraries(ros_middleware_connext_xtypes_dynamic_cpp dl pthread)

ament_package()

install(
  TARGETS ros_middleware_connext_xtypes_dynamic_cpp
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
