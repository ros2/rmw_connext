message(" - rosidl_typesupport_connext_cpp_generate_interfaces.cmake")
message("   - target: ${rosidl_generate_interfaces_TARGET}")
message("   - interface files: ${rosidl_generate_interfaces_IDL_FILES}")
message("   - dependency package names: ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES}")

rosidl_generate_dds_interfaces(
  ${rosidl_generate_interfaces_TARGET}__dds_connext_idl
  IDL_FILES ${rosidl_generate_interfaces_IDL_FILES}
  DEPENDENCY_PACKAGE_NAMES ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES}
  OUTPUT_SUBFOLDERS "dds_connext"
)

set(_dds_idl_files "")
set(_dds_idl_base_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_dds_idl")
set(_dds_idl_path "${_dds_idl_base_path}/${PROJECT_NAME}/dds_connext")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
  if("${_extension}" STREQUAL ".msg")
    get_filename_component(name "${_idl_file}" NAME_WE)
    list(APPEND _dds_idl_files "${_dds_idl_path}/${name}_.idl")
  endif()
endforeach()

set(_output_path "${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_connext_cpp/${PROJECT_NAME}/dds_connext")
set(_generated_files "")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
  if("${_extension}" STREQUAL ".msg")
    get_filename_component(name "${_idl_file}" NAME_WE)
    list(APPEND _generated_files "${_output_path}/${name}_.h")
    list(APPEND _generated_files "${_output_path}/${name}_.cxx")
    list(APPEND _generated_files "${_output_path}/${name}_Plugin.h")
    list(APPEND _generated_files "${_output_path}/${name}_Plugin.cxx")
    list(APPEND _generated_files "${_output_path}/${name}_Support.h")
    list(APPEND _generated_files "${_output_path}/${name}_Support.cxx")
    list(APPEND _generated_files "${_output_path}/${name}_TypeSupport.h")
    list(APPEND _generated_files "${_output_path}/${name}_TypeSupport.cpp")
  elseif("${_extension}" STREQUAL ".srv")
    get_filename_component(name "${_idl_file}" NAME_WE)
    list(APPEND _generated_files "${_output_path}/${name}_ServiceTypeSupport.cpp")
  endif()
endforeach()

set(_dependency_files "")
set(_dependencies "")
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  foreach(_idl_file ${${_pkg_name}_INTERFACE_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
    if("${_extension}" STREQUAL ".msg")
      get_filename_component(name "${_idl_file}" NAME_WE)
      set(_abs_idl_file "${${_pkg_name}_DIR}/../dds_connext/${name}_.idl")
      normalize_path(_abs_idl_file "${_abs_idl_file}")
      list(APPEND _dependency_files "${_abs_idl_file}")
      list(APPEND _dependencies "${_pkg_name}:${_abs_idl_file}")
    endif()
  endforeach()
endforeach()

message("   - generated files: ${_generated_files}")
message("   - dependencies: ${_dependencies}")

add_custom_command(
  OUTPUT ${_generated_files}
  COMMAND ${PYTHON_EXECUTABLE} ${rosidl_typesupport_connext_cpp_BIN}
  --pkg-name ${PROJECT_NAME}
  --ros-interface-files ${rosidl_generate_interfaces_IDL_FILES}
  --dds-interface-files ${_dds_idl_files}
  --dds-interface-base-path ${_dds_idl_base_path}
  --deps ${_dependencies}
  --output-dir "${_output_path}"
  --idl-pp "${Connext_DDSGEN2}"
  --template-dir ${rosidl_typesupport_connext_cpp_TEMPLATE_DIR}
  DEPENDS
  ${rosidl_typesupport_connext_cpp_BIN}
  ${rosidl_typesupport_connext_cpp_GENERATOR_FILES}
  ${rosidl_typesupport_connext_cpp_TEMPLATE_DIR}/msg_TypeSupport.h.template
  ${rosidl_typesupport_connext_cpp_TEMPLATE_DIR}/msg_TypeSupport.cpp.template
  ${rosidl_typesupport_connext_cpp_TEMPLATE_DIR}/srv_ServiceTypeSupport.cpp.template
  ${_dds_idl_files}
  ${_dependency_files}
  COMMENT "Generating C++ type support for RTI Connext"
  VERBATIM
)

set(_target_suffix "__dds_connext_cpp")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-tautological-compare")

link_directories(${Connext_LIBRARY_DIRS})
add_library(${rosidl_generate_interfaces_TARGET}${_target_suffix} SHARED ${_generated_files})
target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_cpp
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_typesupport_connext_cpp
)
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  set(_include_dir "${${_pkg_name}_DIR}/../../../include/${_pkg_name}/dds_connext")
  normalize_path(_include_dir "${_include_dir}")
  target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PUBLIC
    "${_include_dir}"
  )
  ament_target_dependencies(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    ${_pkg_name})
endforeach()
ament_target_dependencies(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  "Connext"
  "rosidl_generator_cpp")

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

install(
  FILES ${_generated_files}
  DESTINATION "include/${PROJECT_NAME}/dds_connext"
)
install(
  TARGETS ${rosidl_generate_interfaces_TARGET}${_target_suffix}
  DESTINATION "lib"
)

ament_export_include_directories(include)
ament_export_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix} ${Connext_LIBRARIES})
