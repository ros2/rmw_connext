cmake_minimum_required(VERSION 2.8.3)

project(rmw_connext_xtypes_dynamic_cpp)

find_package(ament_cmake REQUIRED)

find_package(connext_cmake_module REQUIRED)
find_package(Connext MODULE)
if(NOT Connext_FOUND)
  ament_package()
  message(WARNING "Could not find RTI Connext - skipping '${PROJECT_NAME}'")
  return()
endif()

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

find_package(rmw REQUIRED)
find_package(rosidl_generator_cpp REQUIRED)

find_package(rosidl_typesupport_introspection_cpp REQUIRED)
ament_export_dependencies(rosidl_typesupport_introspection_cpp)
ament_export_dependencies(rmw rosidl_generator_cpp)
ament_export_include_directories(include/${PROJECT_NAME}/impl ${Connext_INCLUDE_DIRS})

register_rmw_implementation()

link_directories("${Connext_LIBRARY_DIRS}")
add_library(rmw_connext_xtypes_dynamic_cpp SHARED src/functions.cpp)
ament_target_dependencies(rmw_connext_xtypes_dynamic_cpp
  "rosidl_typesupport_introspection_cpp"
  "rmw"
  "rosidl_generator_cpp"
  "Connext")
configure_rmw_library(rmw_connext_xtypes_dynamic_cpp)

if(NOT WIN32)
  set(rmw_connext_xtypes_dynamic_cpp_ADDITIONAL_LIBRARIES dl pthread)
endif()
ament_export_libraries(rmw_connext_xtypes_dynamic_cpp
  ${rmw_connext_xtypes_dynamic_cpp_ADDITIONAL_LIBRARIES})

if(AMENT_ENABLE_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  TARGETS rmw_connext_xtypes_dynamic_cpp
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
