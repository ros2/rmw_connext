cmake_minimum_required(VERSION 2.8.3)

project(connext_cmake_module NONE)

find_package(ament_cmake REQUIRED)

if(AMENT_ENABLE_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package(
  CONFIG_EXTRAS "connext_cmake_module-extras.cmake"
)

install(DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME})
