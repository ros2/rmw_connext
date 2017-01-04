cmake_minimum_required(VERSION 3.5)

project(rosidl_typesupport_connext_c)

set(CONNEXT_STATIC_DISABLE $ENV{CONNEXT_STATIC_DISABLE}
  CACHE BOOL "If Connext Static should be disabled.")

if(NOT WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Wpedantic")
endif()

find_package(ament_cmake REQUIRED)

find_package(connext_cmake_module REQUIRED)
find_package(Connext MODULE)
if(NOT Connext_FOUND OR CONNEXT_STATIC_DISABLE)
  ament_package()
  if(NOT Connext_FOUND)
    message(WARNING "Could not find RTI Connext - skipping '${PROJECT_NAME}'")
  else()
    message(STATUS "Connext static rmw implementation explicitly disabled - skipping '${PROJECT_NAME}'")
  endif()
  return()
endif()

find_package(ament_cmake_python REQUIRED)
find_package(rosidl_typesupport_connext_cpp REQUIRED)

ament_export_dependencies(rmw)
ament_export_dependencies(rosidl_cmake)
ament_export_dependencies(rosidl_generator_c)
ament_export_dependencies(rosidl_generator_dds_idl)
ament_export_dependencies(rosidl_typesupport_connext_cpp)
ament_export_include_directories(include)

ament_python_install_package(${PROJECT_NAME})

add_library(${PROJECT_NAME} SHARED src/identifier.cpp)
if(WIN32)
  target_compile_definitions(${PROJECT_NAME}
    PRIVATE "ROSIDL_TYPESUPPORT_CONNEXT_C_BUILDING_DLL")
endif()
target_include_directories(${PROJECT_NAME} PUBLIC include)
ament_target_dependencies(${PROJECT_NAME} "rosidl_typesupport_connext_cpp")
ament_export_libraries(${PROJECT_NAME})

ament_index_register_resource("rosidl_typesupport_c")

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package(
  CONFIG_EXTRAS "rosidl_typesupport_connext_c-extras.cmake.in"
)

install(
  PROGRAMS bin/rosidl_typesupport_connext_c
  DESTINATION lib/rosidl_typesupport_connext_c
)

install(
  DIRECTORY cmake resource
  DESTINATION share/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
