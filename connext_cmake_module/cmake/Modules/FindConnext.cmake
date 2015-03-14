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
# - Connext_LIBRARIES: Name to the C++ libraries including the path
# - Connext_LIBRARY_DIRS: Paths to the libraries
# - Connext_LIBRARY_DIR: Path to libraries; guaranteed to be a single path
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

# check if all libraries with an expected name have been found
function(_find_connext_ensure_libraries var expected_library_names library_paths)
  foreach(expected_library_name ${expected_library_names})
    set(found FALSE)
    foreach(library_path ${library_paths})
      get_filename_component(library_name "${library_path}" NAME)
      if("${expected_library_name}" STREQUAL "${library_name}")
        set(found TRUE)
        break()
      endif()
    endforeach()
    if(NOT found)
      set(${var} FALSE PARENT_SCOPE)
      return()
    endif()
  endforeach()
  set(${var} TRUE PARENT_SCOPE)
endfunction()

set(_expected_library_base_names
  "nddsc"
  "nddscore"
  "nddscpp"
  "rticonnextmsgcpp"
)

foreach(_base_name IN LISTS _expected_library_base_names)
  if(WIN32)
    list(APPEND _expected_library_names "${_base_name}.lib")
  else()
    list(APPEND _expected_library_names "lib${_base_name}.so")
  endif()
endforeach()

file(TO_CMAKE_PATH "$ENV{NDDSHOME}" _NDDSHOME)

if(NOT "${_NDDSHOME} " STREQUAL " ")
  # look inside of NDDSHOME if defined
  message(STATUS "Found RTI Connext: ${_NDDSHOME}")
  set(Connext_HOME "${_NDDSHOME}")
  set(Connext_INCLUDE_DIRS "${_NDDSHOME}/include" "${_NDDSHOME}/include/ndds")

  set(_lib_path "${_NDDSHOME}/lib")

  set(_search_library_paths "")
  foreach(_library_name ${_expected_library_names})
    list(APPEND _search_library_paths "${_lib_path}/*/${_library_name}")
  endforeach()

  # find library nddscpp
  file(GLOB_RECURSE _libs
    RELATIVE "${_lib_path}"
    ${_search_library_paths}
  )

  # remove libraries from non-matching platforms
  set(_i 0)
  set(_matched_VS2015 FALSE)
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
    # If matched so far, and Windows, ignore anything without VS2013 or VS2015
    if(_match AND WIN32)
      set(_match FALSE)
      string(FIND "${_lib}" "VS2015/" _found)
      if(NOT ${_found} EQUAL -1)
        set(_match TRUE)
        set(_matched_VS2015 TRUE)
      endif()
      string(FIND "${_lib}" "VS2013/" _found)
      if(NOT ${_found} EQUAL -1)
        set(_match TRUE)
      endif()
    endif()
    # keep libraries matching this platform
    if(_match)
      math(EXPR _i "${_i} + 1")
    else()
      list(REMOVE_AT _libs ${_i})
    endif()
  endwhile()

  if(_matched_VS2015)
    set(_i 0)
    while(TRUE)
      list(LENGTH _libs _length)
      if(NOT ${_i} LESS ${_length})
        break()
      endif()
      list(GET _libs ${_i} _lib)
      set(_match TRUE)
      string(FIND "${_lib}" "VS2015" _found)
      if(NOT ${_found} EQUAL -1)
        set(_match FALSE)
      endif()
      if(_match)
        math(EXPR _i "${_i} + 1")
      else()
        list(REMOVE_AT _libs ${_i})
      endif()
    endwhile()
  endif()

  _find_connext_ensure_libraries(_found_all_libraries "${_expected_library_names}" "${_libs}")
  if(NOT _found_all_libraries)
    message(FATAL_ERROR "NDDSHOME set to '${_NDDSHOME}' but could not find all libraries '${_expected_library_names}' under '${_lib_path}': ${_libs}")
  endif()
  list(LENGTH _expected_library_names _expected_length)
  if(_length GREATER _expected_length)
    message(FATAL_ERROR "NDDSHOME set to '${_NDDSHOME}' but found multiple files named '${_expected_library_names}' under '${_lib_path}': ${_libs}")
  endif()

  set(Connext_LIBRARIES "")
  foreach(_lib IN LISTS _libs)
    list(APPEND Connext_LIBRARIES "${_lib_path}/${_lib}")
  endforeach()
  list(GET _libs 0 _first_lib)
  get_filename_component(Connext_LIBRARY_DIRS "${_lib_path}/${_first_lib}" DIRECTORY)
  # Since we know Connext_LIBRARY_DIRS is a single path, just alias it.
  set(Connext_LIBRARY_DIR "${Connext_LIBRARY_DIRS}")

  if(WIN32)
    set(Connext_DEFINITIONS "-DRTI_WIN32" "-DNDDS_DLL_VARIABLE")
    # This will be a .bat file and it will be on the PATH.
    set(Connext_DDSGEN2 "rtiddsgen2.bat")
  else()
    set(Connext_DEFINITIONS "-DRTI_LINUX" "-DRTI_UNIX")
    set(Connext_DDSGEN2 "${Connext_LIBRARY_DIRS}/rtiddsgen2")
  endif()
  set(Connext_FOUND TRUE)
else()
  # try to find_package() it
  find_package(ndds_cpp)
  if(ndds_cpp_FOUND)
    message(STATUS "Found RTI Connext: ${ndds_cpp_DIR}")
    set(Connext_INCLUDE_DIRS ${ndds_cpp_INCLUDE_DIRS})
    set(Connext_LIBRARIES ${ndds_cpp_LIBRARIES})
    set(Connext_LIBRARY_DIRS "")
    set(Connext_LIBRARY_DIR "")
    set(Connext_DEFINITIONS ${ndds_cpp_DEFINITIONS})
    set(Connext_DDSGEN2 "/usr/bin/rtiddsgen2")
    set(Connext_FOUND TRUE)

    _find_connext_ensure_libraries(_found_all_libraries "${_expected_library_names}" "${Connext_LIBRARIES}")
    if(NOT _found_all_libraries)
      message(FATAL_ERROR "Connext_LIBRARIES does not contain all libraries '${_expected_library_names}': ${Connext_LIBRARIES}")
    endif()
  endif()
endif()

if(Connext_FOUND AND NOT WIN32)
  list(APPEND Connext_LIBRARIES "pthread" "dl")
endif()
