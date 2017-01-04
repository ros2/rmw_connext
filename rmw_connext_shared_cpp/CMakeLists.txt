cmake_minimum_required(VERSION 3.5)

project(rmw_connext_shared_cpp)

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Wpedantic")
endif()

find_package(ament_cmake REQUIRED)
find_package(connext_cmake_module REQUIRED)
find_package(Connext MODULE)
if(NOT Connext_FOUND)
  ament_package()
  message(WARNING "Could not find RTI Connext - skipping '${PROJECT_NAME}'")
  return()
endif()
find_package(rmw REQUIRED)

include_directories(include)

ament_export_definitions(${Connext_DEFINITIONS})
ament_export_include_directories(include ${Connext_INCLUDE_DIRS})

ament_export_dependencies(rmw)

link_directories(${Connext_LIBRARY_DIRS})
add_library(rmw_connext_shared_cpp SHARED src/shared_functions.cpp)
ament_target_dependencies(rmw_connext_shared_cpp
  "rmw"
  "Connext")
ament_export_libraries(rmw_connext_shared_cpp ${Connext_LIBRARIES})

if(WIN32)
  # Causes the visibility macros to use dllexport rather than dllimport
  # which is appropriate when building the dll but not consuming it.
  target_compile_definitions(rmw_connext_shared_cpp
    PRIVATE "RMW_CONNEXT_SHARED_CPP_BUILDING_DLL"
            "_CRT_NONSTDC_NO_DEPRECATE")
endif()

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package(
  CONFIG_EXTRAS "rmw_connext_shared_cpp-extras.cmake"
)

install(
  DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  TARGETS rmw_connext_shared_cpp
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
