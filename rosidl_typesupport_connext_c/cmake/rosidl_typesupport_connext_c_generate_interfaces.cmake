# Copyright 2016 Open Source Robotics Foundation, Inc.
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

set(_output_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_connext_c/${PROJECT_NAME}")
set(_dds_output_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_connext_cpp/${PROJECT_NAME}")
set(_generated_msg_files "")
set(_generated_external_msg_files "")
set(_generated_srv_files "")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
  get_filename_component(_msg_name "${_idl_file}" NAME_WE)
  string_camel_case_to_lower_case_underscore("${_msg_name}" _header_name)
  if("${_extension} " STREQUAL ".msg " AND ${_msg_name} MATCHES "Response$|Request$")
    # Don't do anything
  elseif("${_extension} " STREQUAL ".msg ")
    get_filename_component(_parent_folder "${_idl_file}" DIRECTORY)
    get_filename_component(_parent_folder "${_parent_folder}" NAME)
    list(APPEND _generated_external_msg_files "${_dds_output_path}/${_parent_folder}/dds_connext/${_msg_name}_.h")
    list(APPEND _generated_external_msg_files "${_dds_output_path}/${_parent_folder}/dds_connext/${_msg_name}_.cxx")
    list(APPEND _generated_external_msg_files "${_dds_output_path}/${_parent_folder}/dds_connext/${_msg_name}_Plugin.h")
    list(APPEND _generated_external_msg_files "${_dds_output_path}/${_parent_folder}/dds_connext/${_msg_name}_Plugin.cxx")
    list(APPEND _generated_external_msg_files "${_dds_output_path}/${_parent_folder}/dds_connext/${_msg_name}_Support.h")
    list(APPEND _generated_external_msg_files "${_dds_output_path}/${_parent_folder}/dds_connext/${_msg_name}_Support.cxx")
    list(APPEND _generated_msg_files "${_output_path}/msg/dds_connext_c/${_header_name}__type_support_c.cpp")
  elseif("${_extension} " STREQUAL ".srv ")
    list(APPEND _generated_srv_files "${_output_path}/srv/dds_connext_c/${_header_name}__type_support_c.cpp")
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
      "-Wno-unused-but-set-variable"
      "-Wno-unused-parameter"
      "-Wno-unused-variable")
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
  "${rosidl_typesupport_connext_c_BIN}"
  ${rosidl_typesupport_connext_c_GENERATOR_FILES}
  "${rosidl_typesupport_connext_c_TEMPLATE_DIR}/msg__type_support_c.cpp.template"
  "${rosidl_typesupport_connext_c_TEMPLATE_DIR}/srv__type_support_c.cpp.template"
  ${_dependency_files})
foreach(dep ${target_dependencies})
  if(NOT EXISTS "${dep}")
    message(FATAL_ERROR "Target dependency '${dep}' does not exist")
  endif()
endforeach()

set(generator_arguments_file "${CMAKE_BINARY_DIR}/rosidl_typesupport_connext_c__arguments.json")
rosidl_write_generator_arguments(
  "${generator_arguments_file}"
  PACKAGE_NAME "${PROJECT_NAME}"
  ROS_INTERFACE_FILES "${rosidl_generate_interfaces_IDL_FILES}"
  ROS_INTERFACE_DEPENDENCIES "${_dependencies}"
  OUTPUT_DIR "${_output_path}"
  TEMPLATE_DIR "${rosidl_typesupport_connext_c_TEMPLATE_DIR}"
  TARGET_DEPENDENCIES ${target_dependencies}
  ADDITIONAL_FILES ${_dds_idl_files}
)

add_custom_command(
  OUTPUT ${_generated_msg_files} ${_generated_srv_files}
  COMMAND ${PYTHON_EXECUTABLE} ${rosidl_typesupport_connext_c_BIN}
  --generator-arguments-file "${generator_arguments_file}"
  DEPENDS
    ${target_dependencies}
    ${generated_external_msg_files}
    ${generated_external_srv_files}
    ${_dds_idl_files}
  COMMENT "Generating C type support for RTI Connext"
  VERBATIM
)

set(_visibility_control_file
  "${_output_path}/msg/dds_connext_c/visibility_control.h")
configure_file(
  "${rosidl_typesupport_connext_c_TEMPLATE_DIR}/visibility_control.h.in"
  "${_visibility_control_file}"
  @ONLY
)

list(APPEND _generated_msg_files "${_visibility_control_file}")

set(_target_suffix "__rosidl_typesupport_connext_c")

link_directories(${Connext_LIBRARY_DIRS})
add_library(${rosidl_generate_interfaces_TARGET}${_target_suffix} SHARED
  ${_generated_msg_files} ${_generated_external_msg_files} ${_generated_srv_files}
  ${_generated_external_srv_files})
ament_target_dependencies(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  "rmw"
  "rosidl_typesupport_connext_cpp"
  "rosidl_generator_c")
if(WIN32)
  target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PRIVATE "ROSIDL_BUILDING_DLL")
  target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PRIVATE "ROSIDL_TYPESUPPORT_CONNEXT_C_BUILDING_DLL")
  target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PRIVATE "NDDS_USER_DLL_EXPORT_${PROJECT_NAME}")
endif()
# The following still uses CPP because the Connext code which uses it was generated for CPP.
target_compile_definitions(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  PRIVATE "ROSIDL_TYPESUPPORT_CONNEXT_CPP_BUILDING_DLL_${PROJECT_NAME}")

if(NOT WIN32)
  set(_target_compile_flags "-Wall -Wextra")
else()
  set(_target_compile_flags
    "/W4"  # Enable level 3 warnings
    "/wd4100"  # Ignore unreferenced formal parameter warnings
    "/wd4127"  # Ignore conditional expression is constant warnings
    "/wd4275"  # Ignore "an exported class derived from a non-exported class" warnings
    "/wd4458"  # Ignore class hides member variable warnings
    "/wd4701"  # Ignore unused variable warnings
  )
endif()
string(REPLACE ";" " " _target_compile_flags "${_target_compile_flags}")
set_target_properties(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  PROPERTIES COMPILE_FLAGS ${_target_compile_flags})
target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_c
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_connext_c
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_connext_cpp
)
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  set(_msg_include_dir "${${_pkg_name}_DIR}/../../../include/${_pkg_name}/msg/dds_connext_c")
  set(_srv_include_dir "${${_pkg_name}_DIR}/../../../include/${_pkg_name}/srv/dds_connext_c")
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
  "rosidl_typesupport_connext_c")
target_link_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  ${rosidl_generate_interfaces_TARGET}__rosidl_generator_c)

add_dependencies(
  ${rosidl_generate_interfaces_TARGET}
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
)
add_dependencies(
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
  ${rosidl_generate_interfaces_TARGET}__cpp
)
add_dependencies(
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
  ${rosidl_generate_interfaces_TARGET}__dds_connext_idl
)

if(NOT rosidl_generate_interfaces_SKIP_INSTALL)
  if(NOT "${_generated_msg_files} " STREQUAL " ")
    install(
      FILES ${_generated_msg_files}
      DESTINATION "include/${PROJECT_NAME}/msg/dds_connext_c"
    )
  endif()
  if(NOT "${_generated_srv_files} " STREQUAL " ")
    install(
      FILES ${_generated_srv_files}
      DESTINATION "include/${PROJECT_NAME}/srv/dds_connext_c"
    )
  endif()

  if(NOT "${_generated_msg_files} " STREQUAL " " OR NOT "${_generated_srv_files} " STREQUAL " ")
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
