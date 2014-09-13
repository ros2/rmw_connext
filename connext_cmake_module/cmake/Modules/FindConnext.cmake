###############################################################################
#
# CMake module for finding RTI Connext.
#
# Input variables:
#
# - NDDSHOME (optional): When specified, header files and libraries
#   will be searched for in `${NDDSHOME}/include/ndds` and
#   `${NDDSHOME}/lib` respectively.
#
# Output variables:
#
# - Connext_FOUND: flag indicating if the package was found
# - Connext_INCLUDE_DIRS: Paths to the header files
# - Connext_LIBRARIES: Name to the C++ libraries including the path
# - Connext_LIBRARY_DIRS: Paths to the libraries
# - Connext_DEFINITIONS: Definitions to be passed on
# - Connext_DDSGEN2: Path to the idl2code generator
#
# Example usage:
#
#   find_package(connext_cmake_module REQUIRED)
#   find_package(Connext MODULE)
#   # use Connext_* variables
#
###############################################################################

set(Connext_FOUND FALSE)

if(NOT "$ENV{NDDSHOME} " STREQUAL " ")
  # look inside of NDDSHOME if defined
  message(STATUS "Found RTI Connext: $ENV{NDDSHOME}")
  set(Connext_INCLUDE_DIRS "$ENV{NDDSHOME}/include/ndds")

  # find library nddscpp
  set(Connext_LIBRARIES "")
  set(_lib_path "$ENV{NDDSHOME}/lib")
  file(GLOB_RECURSE _libs
    RELATIVE "${_lib_path}"
    "$ENV{NDDSHOME}/lib/*/libnddscpp.so")

  # remove libraries from non-matching platforms
  set(_i 0)
  while(TRUE)
    list(LENGTH _libs _length)
    if(NOT ${_i} LESS ${_length})
      break()
    endif()
    list(GET _libs ${_i} _lib)
    set(_match TRUE)
    # ignore libraries in folders with 'jdk' suffix
    string(FIND "${_lib}" "jdk/" _found)
    if(NOT ${_found} EQUAL -1)
      set(_match FALSE)
    endif()
    # ignore libraries in folders containing 'Linux2'
    string(FIND "${_lib}" "Linux2" _found)
    if(NOT ${_found} EQUAL -1)
      set(_match FALSE)
    endif()
    if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      # on 64 bit platforms ignore libraries in folders containing 'i86'
      string(FIND "${_lib}" "i86" _found)
      if(NOT ${_found} EQUAL -1)
        set(_match FALSE)
      endif()
    else()
      # on 32 bit platforms ignore libraries in folders containing 'x64'
      string(FIND "${_lib}" "x64" _found)
      if(NOT ${_found} EQUAL -1)
        set(_match FALSE)
      endif()
    endif()
    # keep libraries matching this platform
    if(_match)
      math(EXPR _i "${_i} + 1")
    else()
      list(REMOVE_AT _libs ${_i})
    endif()
  endwhile()

  if("${_libs} " STREQUAL " ")
    message(FATAL_ERROR "NNDSHOME set to '$ENV{NDDSHOME}' but could not find 'libnddscpp.so' under '${_lib_path}'")
  endif()
  if(_length GREATER 1)
    message(FATAL_ERROR "NNDSHOME set to '$ENV{NDDSHOME}' but found multiple files named 'libnddscpp.so' under '${_lib_path}': ${_libs}")
  endif()

  list(GET _libs 0 _lib)
  list(APPEND Connext_LIBRARIES "${_lib_path}/${_lib}")

  get_filename_component(_lib_path "${Connext_LIBRARIES}" DIRECTORY)
  set(Connext_LIBRARY_DIRS "${_lib_path}")

  set(Connext_DEFINITIONS "-DRTI_LINUX" "-DRTI_UNIX")
  set(Connext_DDSGEN2 "$ENV{NDDSHOME}/scripts/rtiddsgen2")
  set(Connext_FOUND TRUE)
else()
  # try to find_package() it
  find_package(ndds_cpp)
  if(ndds_cpp_FOUND)
    message(STATUS "Found RTI Connext: ${ndds_cpp_DIR}")
    set(Connext_INCLUDE_DIRS ${ndds_cpp_INCLUDE_DIRS})
    set(Connext_LIBRARIES ${ndds_cpp_LIBRARIES})
    set(Connext_LIBRARY_DIRS "")
    set(Connext_DEFINITIONS ${ndds_cpp_DEFINITIONS})
    set(Connext_DDSGEN2 "/usr/bin/rtiddsgen2")
    set(Connext_FOUND TRUE)
  endif()
endif()

if(Connext_FOUND)
  list(APPEND Connext_LIBRARIES "pthread" "dl")
endif()
