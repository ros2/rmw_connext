cmake_minimum_required(VERSION 2.8.3)

project(ros_middleware_connext_cpp)

find_package(ament_cmake REQUIRED)

find_package(connext_cmake_module REQUIRED)
find_package(Connext MODULE)
if(NOT Connext_FOUND)
  ament_package()
  message(WARNING "Could not find RTI Connext - skipping '${PROJECT_NAME}'")
  return()
endif()

find_package(ros_middleware_interface REQUIRED)
find_package(rosidl_generator_cpp REQUIRED)
find_package(rosidl_typesupport_connext_cpp REQUIRED)

ament_export_definitions(${Connext_DEFINITIONS})
ament_export_dependencies(ros_middleware_interface rosidl_generator_cpp rosidl_typesupport_connext_cpp)
ament_export_include_directories(include/${PROJECT_NAME}/impl ${Connext_INCLUDE_DIRS})
ament_export_libraries(ros_middleware_connext_cpp ${Connext_LIBRARIES})

register_ros_middleware_implementation()

ament_package()

link_directories(${Connext_LIBRARY_DIRS})
add_library(ros_middleware_connext_cpp SHARED src/functions.cpp)
ament_target_dependencies(ros_middleware_connext_cpp
  "ros_middleware_interface"
  "rosidl_generator_cpp"
  "rosidl_typesupport_connext_cpp"
  "Connext")

install(
  DIRECTORY include/
  DESTINATION include
)
install(
  TARGETS ros_middleware_connext_cpp
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
