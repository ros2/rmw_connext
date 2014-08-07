# copied from ros_middleware_connext_cpp/ros_middleware_connext_cpp-extras.cmake

find_package(ament_cmake_core REQUIRED)
# TODO
# instead of being an extension for "rosidl_generate_interfaces"
# this should be an extension of "rosidl_generator_cpp"
# which can then ensure that there is only one
ament_register_extension("rosidl_generate_interfaces" "ros_middleware_connext_cpp"
  "ros_middleware_connext_cpp_generate_interfaces.cmake")

set(ros_middleware_connext_cpp_BIN "${ros_middleware_connext_cpp_DIR}/../../../lib/ros_middleware_connext_cpp/ros_middleware_connext_cpp")
set(ros_middleware_connext_cpp_TEMPLATE_DIR "${ros_middleware_connext_cpp_DIR}/../resource")
