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

if(NOT "$ENV{NDDSHOME}" STREQUAL "")
  # look inside of NDDSHOME if defined
  message(STATUS "Found RTI Connext: $ENV{NDDSHOME}")
  set(Connext_INCLUDE_DIRS "$ENV{NDDSHOME}/include/ndds")

  # find library nddscpp
  set(Connext_LIBRARIES "")
  file(GLOB_RECURSE _lib
    "$ENV{NDDSHOME}/lib/*/libnddscpp.so")
  if("${_lib}" STREQUAL "")
    message(FATAL_ERROR "NNDSHOME set to '$ENV{NDDSHOME} but could not find 'libnddscpp.so'")
  endif()
  list(LENGTH _lib _length)
  if(_length GREATER 1)
    message(FATAL_ERROR "NNDSHOME set to '$ENV{NDDSHOME} but found multiple files named 'libnddscpp.so': ${_lib}")
  endif()
  list(APPEND Connext_LIBRARIES ${_lib})

  # find library nddsc relative to the first one
  # get_filename_component(_lib_path "${_lib}" DIRECTORY)
  # file(GLOB_RECURSE _lib
  #   "${_lib_path}/libnddsc.so")
  # if("${_lib}" STREQUAL "")
  #   message(FATAL_ERROR "NNDSHOME set to '$ENV{NDDSHOME} but could not find 'libnddsc.so'")
  # endif()
  # list(LENGTH _lib _length)
  # if(_length GREATER 1)
  #   message(FATAL_ERROR "NNDSHOME set to '$ENV{NDDSHOME} but found multiple files named 'libnddsc.so': ${_lib}")
  # endif()
  # list(APPEND Connext_LIBRARIES ${_lib})

  get_filename_component(_lib_path "${_lib}" DIRECTORY)
  set(Connext_LIBRARY_DIRS "${_lib_path}")

  # find library nddscore relative to the first one
  # file(GLOB_RECURSE _lib
  #   "${_lib_path}/libnddscore.so")
  # if("${_lib}" STREQUAL "")
  #   message(FATAL_ERROR "NNDSHOME set to '$ENV{NDDSHOME} but could not find 'libnddscore.so'")
  # endif()
  # list(LENGTH _lib _length)
  # if(_length GREATER 1)
  #   message(FATAL_ERROR "NNDSHOME set to '$ENV{NDDSHOME} but found multiple files named 'libnddscore.so': ${_lib}")
  # endif()
  # list(APPEND Connext_LIBRARIES ${_lib})

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
