cmake_minimum_required(VERSION 3.16)
if(POLICY CMP0207)
    cmake_policy(SET CMP0207 NEW)
endif()

get_filename_component(_destination "${ACIDCAM_EXECUTABLE}" DIRECTORY)
foreach(_dll IN LISTS ACIDCAM_COMPILER_RUNTIME)
    if(_dll)
        file(COPY "${_dll}" DESTINATION "${_destination}")
    endif()
endforeach()

set(_search_dirs "${_destination}")
foreach(_directory IN LISTS ACIDCAM_RUNTIME_DIRS)
    if(IS_DIRECTORY "${_directory}")
        get_filename_component(_directory "${_directory}" ABSOLUTE)
        list(APPEND _search_dirs "${_directory}")
    endif()
endforeach()
list(REMOVE_DUPLICATES _search_dirs)

# Refresh previously deployed DLLs before scanning their dependencies again.
file(GLOB _local_dlls "${_destination}/*.dll")
foreach(_local_dll IN LISTS _local_dlls)
    get_filename_component(_name "${_local_dll}" NAME)
    # windeployqt patches QtCore for relocation; preserve its deployed copy.
    # Compiler runtime copies have already been selected above.
    if(_name MATCHES "^Qt[56].*\\.dll$|^(msvcp|vcruntime|concrt)[0-9].*\\.dll$")
        continue()
    endif()
    foreach(_directory IN LISTS _search_dirs)
        if(EXISTS "${_directory}/${_name}" AND NOT _directory STREQUAL _destination)
            file(COPY "${_directory}/${_name}" DESTINATION "${_destination}")
            break()
        endif()
    endforeach()
endforeach()

# Qt plugins are loaded dynamically, so scan them as well as the executable.
file(GLOB_RECURSE _plugins "${_destination}/*.dll")
file(TO_CMAKE_PATH "$ENV{SystemRoot}" _windows_dir)
string(TOLOWER "${_windows_dir}" _windows_dir)
file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES "${ACIDCAM_EXECUTABLE}"
    MODULES ${_plugins}
    DIRECTORIES ${_search_dirs}
    RESOLVED_DEPENDENCIES_VAR _resolved
    UNRESOLVED_DEPENDENCIES_VAR _unresolved
    PRE_EXCLUDE_REGEXES "^api-ms-" "^ext-ms-"
    POST_EXCLUDE_REGEXES "^[A-Za-z]:[/\\\\][Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\\\]"
)
if(_unresolved)
    message(FATAL_ERROR "Runtime DLLs not found: ${_unresolved}. Add their directories to ACIDCAM_RUNTIME_DIRS.")
endif()
foreach(_dll IN LISTS _resolved)
    # SystemRoot may be on a drive or directory other than C:/Windows.
    file(TO_CMAKE_PATH "${_dll}" _dll)
    string(TOLOWER "${_dll}" _dll_lower)
    string(FIND "${_dll_lower}" "${_windows_dir}/" _system_prefix)
    get_filename_component(_source_dir "${_dll}" DIRECTORY)
    if(NOT _system_prefix EQUAL 0 AND NOT _source_dir STREQUAL _destination)
        file(COPY "${_dll}" DESTINATION "${_destination}")
    endif()
endforeach()
message(STATUS "Runtime DLLs deployed to ${_destination}")
