# Copyright 2014-2015 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(_ros_idl_files "")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
  # Skip .srv files
  if("${_extension} " STREQUAL ".msg ")
    list(APPEND _ros_idl_files "${_idl_file}")
  endif()
endforeach()

rosidl_generate_dds_interfaces(
  ${rosidl_generate_interfaces_TARGET}__dds_connext_idl
  IDL_FILES ${_ros_idl_files}
  DEPENDENCY_PACKAGE_NAMES ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES}
  OUTPUT_SUBFOLDERS "dds_connext"
)

set(_dds_idl_files "")
set(_dds_idl_base_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_dds_idl")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
  if("${_extension} " STREQUAL ".msg ")
    get_filename_component(_parent_folder "${_idl_file}" DIRECTORY)
    get_filename_component(_parent_folder "${_parent_folder}" NAME)
    get_filename_component(_name "${_idl_file}" NAME_WE)
    list(APPEND _dds_idl_files
      "${_dds_idl_base_path}/${PROJECT_NAME}/${_parent_folder}/dds_connext/${_name}_.idl")
  endif()
endforeach()

set(_output_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_connext_cpp/${PROJECT_NAME}")
set(_generated_msg_files "")
set(_generated_external_msg_files "")
set(_generated_srv_files "")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
  get_filename_component(_msg_name "${_idl_file}" NAME_WE)
  string_camel_case_to_lower_case_underscore("${_msg_name}" _header_name)
  if("${_extension} " STREQUAL ".msg ")
    get_filename_component(_parent_folder "${_idl_file}" DIRECTORY)
    get_filename_component(_parent_folder "${_parent_folder}" NAME)
    list(APPEND _generated_external_msg_files "${_output_path}/${_parent_folder}/dds_connext/${_msg_name}_.h")
    list(APPEND _generated_external_msg_files "${_output_path}/${_parent_folder}/dds_connext/${_msg_name}_.cxx")
    list(APPEND _generated_external_msg_files "${_output_path}/${_parent_folder}/dds_connext/${_msg_name}_Plugin.h")
    list(APPEND _generated_external_msg_files "${_output_path}/${_parent_folder}/dds_connext/${_msg_name}_Plugin.cxx")
    list(APPEND _generated_external_msg_files "${_output_path}/${_parent_folder}/dds_connext/${_msg_name}_Support.h")
    list(APPEND _generated_external_msg_files "${_output_path}/${_parent_folder}/dds_connext/${_msg_name}_Support.cxx")
    list(APPEND _generated_msg_files "${_output_path}/${_parent_folder}/dds_connext/${_header_name}__type_support.hpp")
    list(APPEND _generated_msg_files "${_output_path}/${_parent_folder}/dds_connext/${_header_name}__type_support.cpp")
  elseif("${_extension} " STREQUAL ".srv ")
    list(APPEND _generated_srv_files "${_output_path}/srv/dds_connext/${_header_name}__type_support.cpp")
    list(APPEND _generated_srv_files "${_output_path}/srv/dds_connext/${_header_name}__type_support.hpp")
  else()
    message(FATAL_ERROR "Interface file with unknown extension: ${_idl_file}")
  endif()
endforeach()

# If not on Windows, disable some warnings with Connext's generated code
if(NOT WIN32)
  set(_connext_compile_flags)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(_connext_compile_flags
      "-Wno-deprecated-register"
      "-Wno-mismatched-tags"
      "-Wno-return-type-c-linkage"
      "-Wno-sometimes-uninitialized"
      "-Wno-tautological-compare"
      "-Wno-unused-parameter"
      "-Wno-unused-variable"
    )
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(_connext_compile_flags
      "-Wno-deprecated-declarations"
      "-Wno-strict-aliasing"
      "-Wno-unused-but-set-variable"
      "-Wno-unused-parameter"
      "-Wno-unused-variable"
    )
  endif()
  if(NOT "${_connext_compile_flags} " STREQUAL " ")
    string(REPLACE ";" " " _connext_compile_flags "${_connext_compile_flags}")
    foreach(_gen_file ${_generated_external_msg_files})
      set_source_files_properties("${_gen_file}"
        PROPERTIES COMPILE_FLAGS ${_connext_compile_flags})
    endforeach()
  endif()
endif()

set(_dependency_files "")
set(_dependencies "")
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  foreach(_idl_file ${${_pkg_name}_INTERFACE_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
    if("${_extension} " STREQUAL ".msg ")
      get_filename_component(_parent_folder "${_idl_file}" DIRECTORY)
      get_filename_component(_parent_folder "${_parent_folder}" NAME)
      get_filename_component(_name "${_idl_file}" NAME_WE)
      set(_abs_idl_file "${${_pkg_name}_DIR}/../${_parent_folder}/dds_connext/${_name}_.idl")
      normalize_path(_abs_idl_file "${_abs_idl_file}")
      list(APPEND _dependency_files "${_abs_idl_file}")
      set(_abs_idl_file "${${_pkg_name}_DIR}/../${_idl_file}")
      normalize_path(_abs_idl_file "${_abs_idl_file}")
      list(APPEND _dependencies "${_pkg_name}:${_abs_idl_file}")
    endif()
  endforeach()
endforeach()

set(target_dependencies
  "${rosidl_typesupport_connext_cpp_BIN}"
  ${rosidl_typesupport_connext_cpp_GENERATOR_FILES}
  "${rosidl_typesupport_connext_cpp_TEMPLATE_DIR}/msg__type_support.hpp.em"
  "${rosidl_typesupport_connext_cpp_TEMPLATE_DIR}/msg__type_support.cpp.em"
  "${rosidl_typesupport_connext_cpp_TEMPLATE_DIR}/srv__type_support.hpp.em"
  "${rosidl_typesupport_connext_cpp_TEMPLATE_DIR}/srv__type_support.cpp.em"
  ${_dependency_files})
foreach(dep ${target_dependencies})
  if(NOT EXISTS "${dep}")
    message(FATAL_ERROR "Target dependency '${dep}' does not exist")
  endif()
endforeach()

set(generator_arguments_file "${CMAKE_BINARY_DIR}/rosidl_typesupport_connext_cpp__arguments.json")
rosidl_write_generator_arguments(
  "${generator_arguments_file}"
  PACKAGE_NAME "${PROJECT_NAME}"
  ROS_INTERFACE_FILES "${rosidl_generate_interfaces_IDL_FILES}"
  ROS_INTERFACE_DEPENDENCIES "${_dependencies}"
  OUTPUT_DIR "${_output_path}"
  TEMPLATE_DIR "${rosidl_typesupport_connext_cpp_TEMPLATE_DIR}"
  TARGET_DEPENDENCIES ${target_dependencies}
  ADDITIONAL_FILES ${_dds_idl_files}
)

set(_idl_pp "${Connext_DDSGEN}")
if(NOT "${Connext_DDSGEN_SERVER} " STREQUAL " ")
  # use the code generator in server mode when available
  # because it speeds up the code generation step significantly
  set(_idl_pp "${Connext_DDSGEN_SERVER}")
endif()
add_custom_command(
  OUTPUT ${_generated_msg_files} ${_generated_external_msg_files} ${_generated_srv_files}
  COMMAND ${PYTHON_EXECUTABLE} ${rosidl_typesupport_connext_cpp_BIN}
  --generator-arguments-file "${generator_arguments_file}"
  --dds-interface-base-path "${_dds_idl_base_path}"
  --idl-pp "${_idl_pp}"
  DEPENDS ${target_dependencies} ${_dds_idl_files}
  COMMENT "Generating C++ type support for RTI Connext (using '${_idl_pp}')"
  VERBATIM
)

set(_target_suffix "__rosidl_typesupport_connext_cpp")

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

link_directories(${Connext_LIBRARY_DIRS})
add_library(${rosidl_generate_interfaces_TARGET}${_target_suffix} SHARED
  ${_generated_msg_files} ${_generated_external_msg_files} ${_generated_srv_files})
ament_target_dependencies(${rosidl_generate_interfaces_TARGET}${_target_suffix} "rmw")
if(WIN32)
  target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PRIVATE "ROSIDL_BUILDING_DLL")
  target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PRIVATE "ROSIDL_TYPESUPPORT_CONNEXT_CPP_BUILDING_DLL")
  target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PRIVATE "NDDS_USER_DLL_EXPORT_${PROJECT_NAME}")
endif()
if(NOT WIN32)
  set(_target_compile_flags "-Wall -Wextra")
else()
  set(_target_compile_flags
    "/W4"
    "/wd4100"
    "/wd4127"
    "/wd4275"
    "/wd4305"
    "/wd4458"
    "/wd4701"
  )
endif()
string(REPLACE ";" " " _target_compile_flags "${_target_compile_flags}")
set_target_properties(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  PROPERTIES COMPILE_FLAGS ${_target_compile_flags})
target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_cpp
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_connext_cpp
)
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  set(_msg_include_dir "${${_pkg_name}_DIR}/../../../include/${_pkg_name}/msg/dds_connext")
  set(_srv_include_dir "${${_pkg_name}_DIR}/../../../include/${_pkg_name}/srv/dds_connext")
  normalize_path(_msg_include_dir "${_msg_include_dir}")
  normalize_path(_srv_include_dir "${_srv_include_dir}")
  target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PUBLIC
    "${_msg_include_dir}"
    "${_srv_include_dir}"
  )
  ament_target_dependencies(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    ${_pkg_name})
endforeach()
ament_target_dependencies(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  "Connext"
  "rosidl_typesupport_connext_cpp")

add_dependencies(
  ${rosidl_generate_interfaces_TARGET}
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
)
add_dependencies(
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
  ${rosidl_generate_interfaces_TARGET}__cpp
)
add_dependencies(
  ${rosidl_generate_interfaces_TARGET}__dds_connext_idl
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
)

if(NOT rosidl_generate_interfaces_SKIP_INSTALL)
  if(NOT "${_generated_msg_files}${_generated_external_msg_files} " STREQUAL " ")
    install(
      FILES ${_generated_msg_files} ${_generated_external_msg_files}
      DESTINATION "include/${PROJECT_NAME}/msg/dds_connext"
    )
  endif()
  if(NOT "${_generated_srv_files} " STREQUAL " ")
    install(
      FILES ${_generated_srv_files}
      DESTINATION "include/${PROJECT_NAME}/srv/dds_connext"
    )
  endif()

  if(
    NOT "${_generated_msg_files}${_generated_external_msg_files} " STREQUAL " " OR
    NOT "${_generated_srv_files}${_generated_external_srv_files} " STREQUAL " "
  )
    ament_export_include_directories(include)
  endif()

  install(
    TARGETS ${rosidl_generate_interfaces_TARGET}${_target_suffix}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )

  ament_export_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix} ${Connext_LIBRARIES})
endif()

if(BUILD_TESTING AND rosidl_generate_interfaces_ADD_LINTER_TESTS)
  if(NOT "${_generated_msg_files}${_generated_srv_files} " STREQUAL " ")
    find_package(ament_cmake_cppcheck REQUIRED)
    ament_cppcheck(
      TESTNAME "cppcheck_rosidl_typesupport_connext_cpp"
      ${_generated_msg_files} ${_generated_srv_files})

    find_package(ament_cmake_cpplint REQUIRED)
    get_filename_component(_cpplint_root "${_output_path}" DIRECTORY)
    ament_cpplint(
      TESTNAME "cpplint_rosidl_typesupport_connext_cpp"
      # the generated code might contain longer lines for templated types
      MAX_LINE_LENGTH 999
      ROOT "${_cpplint_root}"
      ${_generated_msg_files} ${_generated_srv_files})

    find_package(ament_cmake_uncrustify REQUIRED)
    ament_uncrustify(
      TESTNAME "uncrustify_rosidl_typesupport_connext_cpp"
      # the generated code might contain longer lines for templated types
      MAX_LINE_LENGTH 999
      ${_generated_msg_files} ${_generated_srv_files})
  endif()
endif()
