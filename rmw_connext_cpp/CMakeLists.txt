cmake_minimum_required(VERSION 2.8.3)

project(rmw_connext_cpp)

set(CONNEXT_STATIC_DISABLE $ENV{CONNEXT_STATIC_DISABLE}
  CACHE BOOL "If Connext Static should be disabled.")

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
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

find_package(rosidl_typesupport_connext_cpp)
if(NOT rosidl_typesupport_connext_cpp_FOUND)
  ament_package()
  message(WARNING "Could not find ROSIDL TypeSupport for Connext (rosidl_typesupport_connext_cpp) - skipping '${PROJECT_NAME}'")
  return()
endif()

find_package(rmw REQUIRED)
find_package(rmw_connext_shared_cpp REQUIRED)
find_package(rosidl_generator_cpp REQUIRED)

ament_export_dependencies(
  rmw
  rmw_connext_shared_cpp
  rosidl_generator_cpp
  rosidl_typesupport_connext_cpp)

add_library(rmw_connext_cpp SHARED src/functions.cpp)
ament_target_dependencies(rmw_connext_cpp
  "rmw"
  "rmw_connext_shared_cpp"
  "rosidl_generator_cpp"
  "rosidl_typesupport_connext_cpp"
  "Connext")
ament_export_libraries(rmw_connext_cpp ${Connext_LIBRARIES})
# On Windows this adds the RMW_BUILDING_DLL definition.
# On Unix (GCC or Clang) it hides the symbols with -fvisibility=hidden.
configure_rmw_library(rmw_connext_cpp)
# Additionally, on Windows, add the ROSIDL_TYPESUPPORT_CONNEXT_CPP_BUILDING_DLL definition.
if(WIN32)
  target_compile_definitions(rmw_connext_cpp
    PRIVATE "_CRT_NONSTDC_NO_DEPRECATE")
  target_compile_definitions(rmw_connext_cpp
    PRIVATE "ROSIDL_TYPESUPPORT_CONNEXT_CPP_BUILDING_DLL"
            "RMW_CONNEXT_SHARED_BUILDING_DLL")
endif()

register_rmw_implementation(
  "c:rosidl_typesupport_connext_c"
  "cpp:rosidl_typesupport_connext_cpp"
)

if(AMENT_ENABLE_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

install(
  TARGETS rmw_connext_cpp
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
