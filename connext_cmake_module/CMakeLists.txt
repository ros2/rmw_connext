cmake_minimum_required(VERSION 2.8.3)

project(connext_cmake_module)

find_package(ament_cmake REQUIRED)

ament_package(
  CONFIG_EXTRAS "connext_cmake_module-extras.cmake"
)

install(DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME})
