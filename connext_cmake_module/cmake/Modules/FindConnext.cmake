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

###############################################################################
#
# CMake module for finding RTI Connext.
#
# Input variables:
#
# - NDDSHOME (optional): When specified, header files and libraries
#   will be searched for in `${NDDSHOME}/include`, `${NDDSHOME}/include/ndds`
#   and `${NDDSHOME}/lib` respectively.
#
# Output variables:
#
# - Connext_FOUND: flag indicating if the package was found
# - Connext_INCLUDE_DIRS: Paths to the header files
# - Connext_HOME: Root directory for the NDDS install.
# - Connext_ARCHITECTURE_NAME: Architecture name according to the lib subfolder
# - Connext_LIBRARIES: Name to the C++ libraries including the path
# - Connext_LIBRARY_DIRS: Paths to the libraries
# - Connext_LIBRARY_DIR: Path to libraries; guaranteed to be a single path
# - Connext_DEFINITIONS: Definitions to be passed on
# - Connext_DDSGEN: Path to the idl2code generator
# - Connext_DDSGEN_SERVER: Path to the idl2code generator in server mode
#   (if available and runnable)
#
# Example usage:
#
#   find_package(connext_cmake_module REQUIRED)
#   find_package(Connext MODULE)
#   # use Connext_* variables
#
# In order to make repeated calls to find_package(Connext ...) after a success
# then you must first reset the Connext_FOUND variable to False.
#
###############################################################################

# lint_cmake: -convention/filename, -package/stdargs

if(Connext_FOUND)
  return()
endif()

set(Connext_FOUND FALSE)

# get the platform specific library names
function(_get_expected_library_names var postfix base_names)
  set(library_names "")
  foreach(base_name IN LISTS base_names)
    # append optional postfix
    set(base_name "${base_name}${postfix}")
    if(WIN32)
      set(library_name "${base_name}.lib")
    elseif(APPLE)
      set(library_name "lib${base_name}.dylib")
    else()
      set(library_name "lib${base_name}.so")
    endif()
    list(APPEND library_names "${library_name}")
  endforeach()

  set(${var} "${library_names}" PARENT_SCOPE)
endfunction()

# get the platform specific libraries
function(_find_connext_libraries var library_names basepath)
  # search for specific file names
  set(search_paths "")
  foreach(library_name ${library_names})
    list(APPEND search_paths "${basepath}/${library_name}")
  endforeach()
  # find libraries
  file(GLOB_RECURSE libraries
    RELATIVE "${basepath}"
    ${search_paths}
  )
  # remove libraries from non-matching platforms
  set(i 0)
  while(TRUE)
    list(LENGTH libraries length)
    if(NOT ${i} LESS ${length})
      break()
    endif()
    list(GET libraries ${i} library)
    set(match TRUE)
    # ignore libraries in folders with 'jdk' suffix
    string(FIND "${library}" "jdk/" _found)
    if(NOT ${_found} EQUAL -1)
      set(match FALSE)
    endif()
    # ignore libraries in folders containing 'Linux2'
    string(FIND "${library}" "Linux2" _found)
    if(NOT ${_found} EQUAL -1)
      set(match FALSE)
    endif()
    # If this is Darwin, only accept Darwin
    if(APPLE)
      set(match FALSE)
      string(FIND "${library}" "x64Darwin15clang7.0" _found)
      if(NOT ${_found} EQUAL -1)
        set(match TRUE)
      else()
        string(FIND "${library}" "x64Darwin14clang6.0" _found)
        if(NOT ${_found} EQUAL -1)
          set(match TRUE)
        endif()
      endif()
    endif()
    if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      # on 64 bit platforms ignore libraries in folders containing 'i86'
      string(FIND "${library}" "i86" _found)
      if(NOT ${_found} EQUAL -1)
        set(match FALSE)
      endif()
    else()
      # on 32 bit platforms ignore libraries in folders containing 'x64'
      string(FIND "${library}" "x64" _found)
      if(NOT ${_found} EQUAL -1)
        set(match FALSE)
      endif()
    endif()
    if(match AND WIN32)
      string(FIND "${library}" "VS2015/" _found)
      if(${_found} EQUAL -1)
        set(match FALSE)
      endif()
    endif()
    # keep libraries matching this platform
    if(match)
      math(EXPR i "${i} + 1")
    else()
      list(REMOVE_AT libraries ${i})
    endif()
  endwhile()

  set(${var} "${libraries}" PARENT_SCOPE)
endfunction()

# collect the number of libraries matching and expected name and
# check that each expected library name is found at least once
function(_count_found_libraries var_count var_found expected_library_names libraries)
  set(found_libraries "")
  set(found_all_libraries TRUE)
  foreach(library_path ${libraries})
    get_filename_component(library_name "${library_path}" NAME)
    list(GET expected_library_names "${library_name}" index)
    if(index EQUAL -1)
      set(found_all_libraries FALSE)
    else()
      list(APPEND found_libraries "${library_path}")
    endif()
  endforeach()
  list(LENGTH found_libraries length)
  set(${var_count} ${length} PARENT_SCOPE)
  set(${var_found} ${found_all_libraries} PARENT_SCOPE)
endfunction()

# get library names
set(_base_names
  "nddsc"
  "nddscore"
  "nddscpp"
  "rticonnextmsgcpp"
)
_get_expected_library_names(_optimized_library_names "" "${_base_names}")
_get_expected_library_names(_debug_library_names "d" "${_base_names}")

file(TO_CMAKE_PATH "$ENV{NDDSHOME}" _NDDSHOME)

if(NOT _NDDSHOME STREQUAL "")
  # look inside of NDDSHOME if defined
  message(STATUS "Found RTI Connext: ${_NDDSHOME}")
  set(Connext_HOME "${_NDDSHOME}")
  set(Connext_INCLUDE_DIRS "${_NDDSHOME}/include" "${_NDDSHOME}/include/ndds")

  set(_lib_path "${_NDDSHOME}/lib")

  # find platform specific Connext libraries
  _find_connext_libraries(_optimized_libraries "${_optimized_library_names}" "${_lib_path}")
  _find_connext_libraries(_debug_libraries "${_debug_library_names}" "${_lib_path}")

  # check if all expected libraries have been found exactly once
  _count_found_libraries(_optimized_libraries_count _found_all_optimized_libraries "${_optimized_library_names}" "${_optimized_libraries}")
  _count_found_libraries(_debug_libraries_count _found_all_debug_libraries "${_debug_library_names}" "${_debug_libraries}")

  list(LENGTH _optimized_library_names _expected_length)
  if(_optimized_libraries_count GREATER _expected_length)
    message(WARNING "NDDSHOME set to '${_NDDSHOME}' but found multiple files named '${_optimized_library_names}' under '${_lib_path}': ${_optimized_libraries}")
    set(_found_all_optimized_libraries FALSE)
  endif()

  list(LENGTH _debug_library_names _expected_length)
  if(_debug_libraries_count GREATER _expected_length)
    message(WARNING "NDDSHOME set to '${_NDDSHOME}' but found multiple files named '${_debug_library_names}' under '${_lib_path}': ${_debug_libraries}")
    set(_found_all_debug_libraries FALSE)
  endif()

  if(NOT _found_all_optimized_libraries AND NOT _found_all_debug_libraries)
    message(FATAL_ERROR "NDDSHOME set to '${_NDDSHOME}' but could neither find all optimized libraries '${_optimized_library_names}' nor all debug libraries '${_debug_library_names}' under '${_lib_path}':\n- optimized: ${_optimized_libraries}\n- debug: ${_debug_libraries}")
  endif()

  set(Connext_LIBRARIES "")
  # first create an imported target for each Connext library
  foreach(_base_name IN LISTS _base_names)
    if(NOT TARGET ${_base_name})
      add_library(${_base_name} UNKNOWN IMPORTED)
    else()
      get_property(_base_name_is_imported PROPERTY IMPORTED)
      if(NOT _base_name_is_imported)
        message(FATAL_ERROR "CMake target '${_base_name}' already exists and is not an imported target.")
      endif()
    endif()
    list(APPEND Connext_LIBRARIES ${_base_name})
  endforeach()
  list(LENGTH _base_names _base_names_len)
  math(EXPR _loop_iterations "${_base_names_len} - 1")
  if(_found_all_optimized_libraries)
    # for each of the optimized libraries set the IMPORTED_LOCATION for the RELEASE config
    foreach(_val RANGE ${_loop_iterations})
      list(GET _base_names ${_val} _base_name)
      list(GET _optimized_libraries ${_val} _lib)
      set_target_properties(${_base_name} PROPERTIES IMPORTED_LOCATION_RELEASE "${_lib_path}/${_lib}")
      set_target_properties(${_base_name} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
      set_target_properties(${_base_name} PROPERTIES MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE)
    endforeach()
  endif()
  if(_found_all_debug_libraries)
    # for each of the debug libraries set the IMPORTED_LOCATION for the DEBUG config
    foreach(_val RANGE ${_loop_iterations})
      list(GET _base_names ${_val} _base_name)
      list(GET _debug_libraries ${_val} _lib)
      set_target_properties(${_base_name} PROPERTIES IMPORTED_LOCATION_DEBUG "${_lib_path}/${_lib}")
    endforeach()
  endif()
  # use the first library of one of the lists to figure out the LIBRARY_DIR
  if(_found_all_optimized_libraries)
    list(GET _optimized_libraries 0 _first_lib)
  endif()
  if(_found_all_debug_libraries)
    list(GET _debug_libraries 0 _first_lib)
  endif()
  # _first_lib is guaranteed to be set at this point
  get_filename_component(Connext_LIBRARY_DIR "${_lib_path}/${_first_lib}" DIRECTORY)
  set(Connext_LIBRARY_DIRS "${Connext_LIBRARY_DIR}")

  # extract architecture name
  get_filename_component(Connext_ARCHITECTURE_NAME "${Connext_LIBRARY_DIR}" NAME)

  if(WIN32)
    set(Connext_DEFINITIONS "RTI_WIN32" "NDDS_DLL_VARIABLE")
    # This will be a .bat file and it will be on the PATH.
    set(Connext_DDSGEN "rtiddsgen.bat")
    find_program(_found "rtiddsgen_server.bat")
    if(_found)
      set(Connext_DDSGEN_SERVER "rtiddsgen_server.bat")
    endif()
  else()
    set(Connext_DEFINITIONS "RTI_LINUX" "RTI_UNIX")
    set(Connext_DDSGEN "${Connext_HOME}/bin/rtiddsgen")
    if(NOT EXISTS "${Connext_DDSGEN}")
      message(FATAL_ERROR "Could not find executable 'rtiddsgen'")
    endif()
    if(EXISTS "${Connext_HOME}/bin/rtiddsgen_server")
      set(Connext_DDSGEN_SERVER "${Connext_HOME}/bin/rtiddsgen_server")
    endif()
  endif()
  set(Connext_FOUND TRUE)
else()
  # try to find_package() it
  find_package(nddscpp)
  find_package(rticonnextmsgcpp)
  if(nddscpp_FOUND AND rticonnextmsgcpp_FOUND)
    message(STATUS "Found RTI Connext: ${nddscpp_DIR} ${rticonnextmsgcpp_DIR}")
    set(Connext_INCLUDE_DIRS ${nddscpp_INCLUDE_DIRS})
    list_append_unique(Connext_INCLUDE_DIRS ${rticonnextmsgcpp_INCLUDE_DIRS})
    set(Connext_LIBRARIES ${nddscpp_LIBRARIES})
    list_append_unique(Connext_LIBRARIES ${rticonnextmsgcpp_LIBRARIES})
    set(Connext_LIBRARY_DIRS "")
    set(Connext_LIBRARY_DIR "")
    set(Connext_DEFINITIONS ${nddscpp_DEFINITIONS})
    list_append_unique(Connext_DEFINITIONS ${rticonnextmsgcpp_DEFINITIONS})
    set(Connext_DDSGEN "/usr/bin/rtiddsgen")
    if(NOT EXISTS "${Connext_DDSGEN}")
      message(FATAL_ERROR "Could not find executable '${Connext_DDSGEN}'")
    endif()
    if(EXISTS "/usr/bin/rtiddsgen_server")
      set(Connext_DDSGEN_SERVER "/usr/bin/rtiddsgen_server")
    endif()
    set(Connext_FOUND TRUE)

    # check if all expected libraries have been found exactly once
    _count_found_libraries(_optimized_libraries_count _found_all_optimized_libraries "${_optimized_library_names}" "${Connext_LIBRARIES}")
    _count_found_libraries(_debug_libraries_count _found_all_debug_libraries "${_debug_library_names}" "${Connext_LIBRARIES}")

    list(LENGTH _optimized_library_names _expected_length)
    if(_optimized_libraries_count GREATER _expected_length)
      message(WARNING "Found multiple files named '${_optimized_library_names}' in Connext_LIBRARIES: ${Connext_LIBRARIES}")
      set(_found_all_optimized_libraries FALSE)
    endif()

    list(LENGTH _debug_library_names _expected_length)
    if(_debug_libraries_count GREATER _expected_length)
      message(WARNING "Found multiple files named '${_debug_library_names}' in Connext_LIBRARIES: ${Connext_LIBRARIES}")
      set(_found_all_debug_libraries FALSE)
    endif()

    if(NOT _found_all_optimized_libraries AND NOT _found_all_debug_libraries)
      message(FATAL_ERROR "Connext_LIBRARIES does neither contain all optimized libraries '${_optimized_library_names}' nor all debug libraries '${_debug_library_names}': ${Connext_LIBRARIES}")
    endif()
  endif()
endif()

if(Connext_FOUND)
  if(NOT WIN32)
    list(APPEND Connext_LIBRARIES "pthread" "dl")
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(Connext_LIBRARIES "-Wl,--no-as-needed" ${Connext_LIBRARIES} "-Wl,--as-needed")

    # check with which ABI the Connext libraries are built
    configure_file(
      "${connext_cmake_module_DIR}/check_abi.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/connext_cmake_module/check_abi/CMakeLists.txt"
      @ONLY
    )
    try_compile(
      Connext_GLIBCXX_USE_CXX11_ABI_ZERO
      "${CMAKE_CURRENT_BINARY_DIR}/connext_cmake_module/check_abi/build"
      "${CMAKE_CURRENT_BINARY_DIR}/connext_cmake_module/check_abi"
      check_abi exe)
    if(Connext_GLIBCXX_USE_CXX11_ABI_ZERO)
      message(STATUS "Connext was build with an old libc++ ABI and needs _GLIBCXX_USE_CXX11_ABI 0")
    endif()
  endif()
endif()

if(Connext_DDSGEN_SERVER)
  # check that the generator is invocable / finds a Java runtime environment
  execute_process(
    COMMAND "${Connext_DDSGEN_SERVER}"
    RESULT_VARIABLE _retcode
    OUTPUT_QUIET ERROR_QUIET)
  if(NOT _retcode EQUAL 0)
    set(Connext_DDSGEN_SERVER)
  endif()
endif()

include(FindPackageHandleStandardArgs)
# Connext_HOME, Connext_ARCHITECTURE_NAME, Connext_LIBRARY_DIRS, and
# Connext_LIBRARY_DIR are not always set, depending on the source of Connext.
find_package_handle_standard_args(Connext
  FOUND_VAR Connext_FOUND
  REQUIRED_VARS
    Connext_INCLUDE_DIRS
    Connext_LIBRARIES
    Connext_DEFINITIONS
    Connext_DDSGEN
)
