cmake_minimum_required(VERSION 3.5)

project(rmw_connext_cpp)

set(CONNEXT_STATIC_DISABLE $ENV{CONNEXT_STATIC_DISABLE}
  CACHE BOOL "If Connext Static should be disabled.")

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
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

find_package(rcutils REQUIRED)

find_package(rosidl_typesupport_connext_c)
if(NOT rosidl_typesupport_connext_c_FOUND)
  ament_package()
  message(WARNING "Could not find ROSIDL TypeSupport for Connext (rosidl_typesupport_connext_c) - skipping '${PROJECT_NAME}'")
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
find_package(rosidl_generator_c REQUIRED)
find_package(rosidl_generator_cpp REQUIRED)

include_directories(include)

ament_export_include_directories(include)
ament_export_dependencies(
  rcutils
  rmw
  rmw_connext_shared_cpp
  rosidl_generator_c
  rosidl_generator_cpp
  rosidl_typesupport_connext_c
  rosidl_typesupport_connext_cpp)

register_rmw_implementation(
  "c:rosidl_typesupport_c:rosidl_typesupport_connext_c"
  "cpp:rosidl_typesupport_cpp:rosidl_typesupport_connext_cpp"
)

# generate code for raw data
set(_idl_pp "${Connext_DDSGEN}")
if(NOT "${Connext_DDSGEN_SERVER}" STREQUAL "")
  # use the code generator in server mode when available
  # because it speeds up the code generation step significantly
  set(_idl_pp "${Connext_DDSGEN_SERVER}")
endif()
set(generated_files
  ${CMAKE_CURRENT_BINARY_DIR}/resources/generated/connext_static_raw_data.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/resources/generated/connext_static_raw_data.h
  ${CMAKE_CURRENT_BINARY_DIR}/resources/generated/connext_static_raw_dataPlugin.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/resources/generated/connext_static_raw_dataPlugin.h
  ${CMAKE_CURRENT_BINARY_DIR}/resources/generated/connext_static_raw_dataSupport.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/resources/generated/connext_static_raw_dataSupport.h
)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/resources/generated")
add_custom_command(
  OUTPUT ${generated_files}
  COMMAND "${_idl_pp}" -language C++ -unboundedSupport "connext_static_raw_data.idl" -d ${CMAKE_CURRENT_BINARY_DIR}/resources/generated
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources"
  COMMENT "Generating raw type support for RTI Connext (using '${_idl_pp}')"
  VERBATIM
)

# patch the generate code for raw data
set(patch_files
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/patch_files/connext_static_raw_data.cxx.patch
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/patch_files/connext_static_raw_data.h.patch
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/patch_files/connext_static_raw_dataPlugin.cxx.patch
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/patch_files/connext_static_raw_dataPlugin.h.patch
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/patch_files/connext_static_raw_dataSupport.cxx.patch
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/patch_files/connext_static_raw_dataSupport.h.patch
)
set(patched_files
  ${CMAKE_CURRENT_BINARY_DIR}/resources/patched/connext_static_raw_data.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/resources/patched/connext_static_raw_data.h
  ${CMAKE_CURRENT_BINARY_DIR}/resources/patched/connext_static_raw_dataPlugin.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/resources/patched/connext_static_raw_dataPlugin.h
  ${CMAKE_CURRENT_BINARY_DIR}/resources/patched/connext_static_raw_dataSupport.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/resources/patched/connext_static_raw_dataSupport.h
)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/resources/patched")
add_custom_command(
  OUTPUT ${patched_files}
  COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/bin/apply-patch.py" --input ${generated_files} --patch ${patch_files} --out ${patched_files}
  DEPENDS ${generated_files}
  COMMENT "Patching raw type support for RTI Connext"
  VERBATIM
)

add_library(
  rmw_connext_cpp
  SHARED
  ${patched_files}
  src/get_client.cpp
  src/get_participant.cpp
  src/get_publisher.cpp
  src/get_service.cpp
  src/get_subscriber.cpp
  src/identifier.cpp
  src/process_topic_and_service_names.cpp
  src/rmw_client.cpp
  src/rmw_compare_gid_equals.cpp
  src/rmw_count.cpp
  src/rmw_get_gid_for_publisher.cpp
  src/rmw_get_implementation_identifier.cpp
  src/rmw_guard_condition.cpp
  src/rmw_init.cpp
  src/rmw_logging.cpp
  src/rmw_node.cpp
  src/rmw_node_names.cpp
  src/rmw_publish.cpp
  src/rmw_publisher.cpp
  src/rmw_request.cpp
  src/rmw_response.cpp
  src/rmw_service.cpp
  src/rmw_service_names_and_types.cpp
  src/rmw_service_server_is_available.cpp
  src/rmw_subscription.cpp
  src/rmw_take.cpp
  src/rmw_topic_names_and_types.cpp
  src/rmw_trigger_guard_condition.cpp
  src/rmw_wait.cpp
  src/rmw_wait_set.cpp)
ament_target_dependencies(rmw_connext_cpp
  "rcutils"
  "rmw"
  "rmw_connext_shared_cpp"
  "rosidl_generator_c"
  "rosidl_generator_cpp"
  "rosidl_typesupport_connext_c"
  "rosidl_typesupport_connext_cpp"
  "Connext")
ament_export_libraries(rmw_connext_cpp)

# Causes the visibility macros to use dllexport rather than dllimport
# which is appropriate when building the library but not consuming it.
target_compile_definitions(rmw_connext_cpp
  PRIVATE "RMW_CONNEXT_CPP_BUILDING_DLL")

# On Windows this adds the RMW_BUILDING_DLL definition.
# On Unix (GCC or Clang) it hides the symbols with -fvisibility=hidden.
configure_rmw_library(rmw_connext_cpp)

if(WIN32)
  target_compile_definitions(rmw_connext_cpp
    PRIVATE "_CRT_NONSTDC_NO_DEPRECATE")
endif()

if(BUILD_TESTING)
  find_package(ament_cmake_cppcheck REQUIRED)
  find_package(ament_cmake_cpplint REQUIRED)
  find_package(ament_cmake_lint_cmake REQUIRED)
  find_package(ament_cmake_uncrustify REQUIRED)
  find_package(ament_cmake_copyright REQUIRED)
  ament_cppcheck()
  ament_cpplint("FILTERS" "-legal/copyright")
  ament_lint_cmake()
  ament_uncrustify()
  ament_copyright(
    "--exclude"
    "connext_static_raw_data.hpp"
    "connext_static_raw_data_plugin.hpp"
    "connext_static_raw_data_support.hpp"
    "connext_static_raw_data.cpp"
    "connext_static_raw_data_plugin.cpp"
    "connext_static_raw_data_support.cpp"
  )

  # ament_lint_auto_find_test_dependencies()
endif()

ament_package(CONFIG_EXTRAS "${PROJECT_NAME}-extras.cmake")

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  TARGETS rmw_connext_cpp
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

get_rmw_connext_output_filter("rmw_connext_output_filter")
ament_index_register_resource("rmw_output_prefixes" CONTENT "${rmw_connext_output_filter}")
