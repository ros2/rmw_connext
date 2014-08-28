# copied from rosidl_typesupport_connext_cpp/rosidl_typesupport_connext_cpp-extras.cmake

find_package(ament_cmake_core REQUIRED)
ament_register_extension("rosidl_generate_interfaces" "rosidl_typesupport_connext_cpp"
  "rosidl_typesupport_connext_cpp_generate_interfaces.cmake")

find_package(connext_cmake_module REQUIRED)
find_package(Connext MODULE)
if(NOT Connext_FOUND)
  message(FATAL_ERROR "Could not find RTI Connext")
endif()

set(rosidl_typesupport_connext_cpp_BIN "${rosidl_typesupport_connext_cpp_DIR}/../../../lib/rosidl_typesupport_connext_cpp/rosidl_typesupport_connext_cpp")
set(rosidl_typesupport_connext_cpp_TEMPLATE_DIR "${rosidl_typesupport_connext_cpp_DIR}/../resource")
