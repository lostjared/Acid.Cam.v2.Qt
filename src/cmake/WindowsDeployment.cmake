# Keep deployment settings local to this function.
function(acidcam_enable_windows_deployment target)
    get_target_property(_qt_qmake Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_tools_dir "${_qt_qmake}" DIRECTORY)
    find_program(ACIDCAM_WINDEPLOYQT NAMES windeployqt windeployqt${QT_VERSION_MAJOR}
        HINTS "${_qt_tools_dir}")
    if(NOT ACIDCAM_WINDEPLOYQT)
        message(FATAL_ERROR "windeployqt is required for ACIDCAM_DEPLOY_RUNTIME. Set ACIDCAM_WINDEPLOYQT to its path, or disable deployment.")
    endif()
    get_filename_component(_deploy_tools_dir "${ACIDCAM_WINDEPLOYQT}" DIRECTORY)
    set(_qt_deploy_args)
    # vcpkg's Qt6 Debug plugins live under a separate install prefix.
    if(QT_VERSION_MAJOR EQUAL 6 AND EXISTS "${_deploy_tools_dir}/qtpaths.debug.bat")
        list(APPEND _qt_deploy_args
            "$<$<CONFIG:Debug>:--qtpaths>"
            "$<$<CONFIG:Debug>:${_deploy_tools_dir}/qtpaths.debug.bat>")
    endif()

    set(_runtime_dirs ${ACIDCAM_RUNTIME_DIRS})
    # Configuration-specific paths must precede generic Release search paths.
    foreach(_prefix IN LISTS CMAKE_PREFIX_PATH)
        list(APPEND _runtime_dirs "${_prefix}/$<$<CONFIG:Debug>:debug/>bin")
    endforeach()
    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        list(APPEND _runtime_dirs
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/$<$<CONFIG:Debug>:debug/>bin")
    endif()
    foreach(_library IN ITEMS Qt${QT_VERSION_MAJOR}::Core
            ${ACIDCAM_LIBRARIES} ${SDL2_LIBRARIES} ${OpenCV_LIBS})
        if(TARGET "${_library}")
            list(APPEND _runtime_dirs "$<TARGET_FILE_DIR:${_library}>")
        endif()
    endforeach()
    # FFmpeg may be exposed as .lib paths rather than imported DLL targets.
    foreach(_library_dir IN LISTS FFMPEG_LIBRARY_DIRS ACIDCAM_LIBRARY_DIRS SDL2_LIBRARY_DIRS)
        list(APPEND _runtime_dirs "${_library_dir}/../bin" "${_library_dir}")
    endforeach()
    if(CUDAToolkit_BIN_DIR)
        list(APPEND _runtime_dirs "${CUDAToolkit_BIN_DIR}")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64|arm64|aarch64)$")
                list(APPEND _runtime_dirs "${CUDAToolkit_BIN_DIR}/arm64")
            else()
                list(APPEND _runtime_dirs "${CUDAToolkit_BIN_DIR}/x64")
            endif()
        else()
            list(APPEND _runtime_dirs "${CUDAToolkit_BIN_DIR}/Win32")
        endif()
    endif()

    # Deploy the compiler DLLs themselves, including Debug CRTs for local builds.
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)
    set(_compiler_runtime)
    foreach(_dll IN LISTS CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        if(_dll MATCHES "[dD]\\.dll$|[Dd]ebug[^/]*/")
            list(APPEND _compiler_runtime "$<$<CONFIG:Debug>:${_dll}>")
        else()
            list(APPEND _compiler_runtime "$<$<NOT:$<CONFIG:Debug>>:${_dll}>")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES _runtime_dirs)

    # Generator expressions keep Visual Studio's Debug/Release output distinct.
    set(_deployment_script "${CMAKE_CURRENT_BINARY_DIR}/deploy-runtime-$<CONFIG>.cmake")
    file(GENERATE OUTPUT "${_deployment_script}" CONTENT
"set(ACIDCAM_EXECUTABLE [==[$<TARGET_FILE:${target}>]==])
set(ACIDCAM_RUNTIME_DIRS [==[${_runtime_dirs}]==])
set(ACIDCAM_COMPILER_RUNTIME [==[${_compiler_runtime}]==])
include([==[${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyRuntimeDependencies.cmake]==])
")
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${ACIDCAM_WINDEPLOYQT}"
            "$<IF:$<CONFIG:Debug>,--debug,--release>"
            ${_qt_deploy_args}
            --no-translations --no-compiler-runtime
            --dir "$<TARGET_FILE_DIR:${target}>" "$<TARGET_FILE:${target}>"
        COMMAND "${CMAKE_COMMAND}" -P "${_deployment_script}"
        COMMENT "Deploying runtime DLLs and Qt plugins beside ${target}"
        COMMAND_EXPAND_LISTS
        VERBATIM)
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/WindowsDeployment.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyRuntimeDependencies.cmake")
endfunction()
