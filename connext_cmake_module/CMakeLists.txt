cmake_minimum_required(VERSION 2.8.3)

project(connext_cmake_module)

find_package(ament_cmake REQUIRED)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
find_package(Connext MODULE)
if(Connext_HOME)
  normalize_path(Connext_HOME "${Connext_HOME}")
  normalize_path(Connext_LIBRARY_DIR "${Connext_LIBRARY_DIR}")
  if(WIN32)
    ament_environment_hooks(env_hook/connext.bat.in)
  else()
    ament_environment_hooks(env_hook/connext.sh.in)
  endif()
endif()

if(AMENT_ENABLE_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package(
  CONFIG_EXTRAS "connext_cmake_module-extras.cmake"
)

install(DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME})
