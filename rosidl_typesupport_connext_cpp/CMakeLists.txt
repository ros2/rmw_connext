cmake_minimum_required(VERSION 2.8.3)

project(rosidl_typesupport_connext_cpp)

find_package(ament_cmake REQUIRED)

find_package(connext_cmake_module REQUIRED)
find_package(Connext MODULE)
if(NOT Connext_FOUND)
  ament_package()
  message(WARNING "Could not find RTI Connext - skipping '${PROJECT_NAME}'")
  return()
endif()

find_package(ament_cmake_python REQUIRED)

ament_export_dependencies(rosidl_cmake)
ament_export_dependencies(rosidl_generator_dds_idl)
ament_export_include_directories(include)

ament_python_install_package(${PROJECT_NAME})

if(AMENT_ENABLE_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package(
  CONFIG_EXTRAS "rosidl_typesupport_connext_cpp-extras.cmake.in"
)

install(
  PROGRAMS bin/rosidl_typesupport_connext_cpp
  DESTINATION lib/rosidl_typesupport_connext_cpp
)
install(
  DIRECTORY cmake resource
  DESTINATION share/${PROJECT_NAME}
)
install(
  DIRECTORY include/
  DESTINATION include
)
