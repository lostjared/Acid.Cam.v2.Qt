cmake_minimum_required(VERSION 3.16)
project(Acid_Cam_v2_Qt VERSION 2.0.3 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(_acidcam_static_gnu_runtime_default ON)
if(APPLE)
    # AppleClang uses libc++ and the Darwin runtime. It does not provide the
    # GNU-only -static-libgcc or -static-libstdc++ linker options.
    set(_acidcam_static_gnu_runtime_default OFF)
endif()
option(ACIDCAM_STATIC_GNU_RUNTIME "Link libgcc and libstdc++ statically"
    ${_acidcam_static_gnu_runtime_default})
unset(_acidcam_static_gnu_runtime_default)

# Make native Homebrew dependencies discoverable on both Apple Silicon
# (/opt/homebrew) and Intel (/usr/local) without replacing user search paths.
if(APPLE)
    find_program(HOMEBREW_EXECUTABLE brew
        HINTS /opt/homebrew/bin /usr/local/bin)
    if(HOMEBREW_EXECUTABLE)
        execute_process(
            COMMAND "${HOMEBREW_EXECUTABLE}" --prefix
            OUTPUT_VARIABLE HOMEBREW_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE HOMEBREW_PREFIX_RESULT
        )
        if(HOMEBREW_PREFIX_RESULT EQUAL 0 AND HOMEBREW_PREFIX)
            list(PREPEND CMAKE_PREFIX_PATH
                "${HOMEBREW_PREFIX}"
                "${HOMEBREW_PREFIX}/opt/qt"
                "${HOMEBREW_PREFIX}/opt/qt@6"
                "${HOMEBREW_PREFIX}/opt/opencv"
                "${HOMEBREW_PREFIX}/opt/sdl2"
            )
            message(STATUS "Homebrew prefix: ${HOMEBREW_PREFIX}")
        endif()
    endif()
endif()

# When cross-compiling with MinGW, help CMake find packages
if(MINGW)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a" ".a" ".lib")
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL Network QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui Widgets OpenGL Network)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

# OpenGL - on MinGW, link opengl32 directly if FindOpenGL fails
find_package(OpenGL QUIET)
if(NOT OPENGL_FOUND AND MINGW)
    message(STATUS "FindOpenGL failed on MinGW, using opengl32 directly")
    set(OPENGL_LIBRARIES opengl32)
    set(OPENGL_FOUND TRUE)
elseif(NOT OPENGL_FOUND)
    message(FATAL_ERROR "OpenGL not found")
endif()

find_package(PkgConfig REQUIRED)
if(NOT CMAKE_CROSSCOMPILING)
    set(_acidcam_pkg_config_paths "/usr/local/lib/pkgconfig")
    if(APPLE AND HOMEBREW_PREFIX)
        list(PREPEND _acidcam_pkg_config_paths
            "${HOMEBREW_PREFIX}/lib/pkgconfig"
            "${HOMEBREW_PREFIX}/share/pkgconfig"
        )
    endif()
    list(JOIN _acidcam_pkg_config_paths ":" _acidcam_pkg_config_path)
    if(DEFINED ENV{PKG_CONFIG_PATH} AND NOT "$ENV{PKG_CONFIG_PATH}" STREQUAL "")
        string(APPEND _acidcam_pkg_config_path ":$ENV{PKG_CONFIG_PATH}")
    endif()
    set(ENV{PKG_CONFIG_PATH} "${_acidcam_pkg_config_path}")
    unset(_acidcam_pkg_config_path)
    unset(_acidcam_pkg_config_paths)
endif()

pkg_check_modules(ACIDCAM REQUIRED acidcam)
pkg_check_modules(SDL2 REQUIRED sdl2)

# Remove SDL2main from link libraries - this Qt app has its own main(),
# and SDL2main provides a conflicting main() wrapper.
list(REMOVE_ITEM SDL2_LIBRARIES SDL2main -lSDL2main)
list(FILTER SDL2_LDFLAGS EXCLUDE REGEX "SDL2main")

# Use find_package for OpenCV (pkg-config opencv4.pc not always available, e.g. MinGW)
find_package(OpenCV REQUIRED)

# MXWrite links directly to the FFmpeg libraries. No ffmpeg executable is
# launched at runtime.
add_subdirectory(MXWrite)
# Fix empty OpenCV_INCLUDE_DIRS during MinGW cross-compilation
if(MINGW AND (NOT OpenCV_INCLUDE_DIRS OR OpenCV_INCLUDE_DIRS STREQUAL ""))
    set(_mingw_sysroot "/usr/x86_64-w64-mingw32/sys-root/mingw")
    if(EXISTS "${_mingw_sysroot}/include/opencv4/opencv2/core.hpp")
        set(OpenCV_INCLUDE_DIRS "${_mingw_sysroot}/include/opencv4")
    elseif(EXISTS "${_mingw_sysroot}/include/opencv2/core.hpp")
        set(OpenCV_INCLUDE_DIRS "${_mingw_sysroot}/include")
    endif()
    if(OpenCV_INCLUDE_DIRS)
        message(STATUS "Fixed empty OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
        foreach(_opencv_target ${OpenCV_LIBS})
            if(TARGET ${_opencv_target})
                set_property(TARGET ${_opencv_target} APPEND PROPERTY
                    INTERFACE_INCLUDE_DIRECTORIES "${OpenCV_INCLUDE_DIRS}")
            endif()
        endforeach()
    endif()
endif()
message(STATUS "OpenCV ${OpenCV_VERSION}: includes=${OpenCV_INCLUDE_DIRS}")


set(SOURCES
    main.cpp
    main_window.cpp
    new_dialog.cpp
    plugin.cpp
    select_image.cpp
    display_window.cpp
    playback_thread.cpp
    search_box.cpp
    goto_window.cpp
    chroma_window.cpp
    user_define.cpp
    dl-man.cpp
    image_window.cpp
    options_window.cpp
    controller.cpp
    color_range.cpp
    slitscan_win.cpp
    ffmpeg_write.cpp
    ffmpeg_encoder_thread.cpp
    ffmpeg_mux_thread.cpp
)

set(HEADERS
    main_window.h
    new_dialog.h
    plugin.h
    qtheaders.h
    select_image.h
    display_window.h
    playback_thread.h
    search_box.h
    goto_window.h
    chroma_window.h
    user_define.h
    dl-man.h
    image_window.h
    options_window.h
    controller.h
    color_range.h
    slitscan_win.h
    ffmpeg_write.h
    ffmpeg_encoder_thread.h
    ffmpeg_mux_thread.h
)


set(RESOURCES
    qresource.qrc
)

# Windows resource file for icon
if(WIN32)
    set(WIN_RESOURCES win-icon.rc)
endif()

# Use WIN32 on Windows to create a GUI app (no console window)
if(WIN32)
    add_executable(${PROJECT_NAME} WIN32
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
        ${WIN_RESOURCES}
    )
else()
    add_executable(${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME acidcam-qt
)

add_library(acidcam-basic-plugin MODULE plugins/basic/basic.cpp)
set_target_properties(acidcam-basic-plugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "basic"
)


target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ACIDCAM_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)


target_link_directories(${PROJECT_NAME} PRIVATE
    ${ACIDCAM_LIBRARY_DIRS}
    ${SDL2_LIBRARY_DIRS}
)


target_compile_options(${PROJECT_NAME} PRIVATE
    ${ACIDCAM_CFLAGS_OTHER}
    ${SDL2_CFLAGS_OTHER}
)

# Prevent SDL from hijacking main() - we manage our own entry point
target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED)

# Static linking of GNU runtime libraries
if(ACIDCAM_STATIC_GNU_RUNTIME AND NOT APPLE
   AND CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
    if(MINGW)
        target_link_options(${PROJECT_NAME} PRIVATE -Wl,-Bstatic -lpthread -Wl,-Bdynamic)
    endif()
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::Network
        ${OPENGL_LIBRARIES}
        ${ACIDCAM_LIBRARIES}
        ${OpenCV_LIBS}
        ${SDL2_LIBRARIES}
        mxwrite
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::OpenGL
        Qt5::Network
        ${OPENGL_LIBRARIES}
        ${ACIDCAM_LIBRARIES}
        ${OpenCV_LIBS}
        ${SDL2_LIBRARIES}
        mxwrite
    )
endif()

# Additional Windows libraries needed for networking, multimedia, etc.
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ws2_32
        winmm
        imm32
        dwmapi
        uxtheme
    )
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS acidcam-basic-plugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/acidcam/plugins/basic
)

install(FILES ../flatpak/io.github.lostjared.AcidCam.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES ../flatpak/io.github.lostjared.AcidCam.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
)

install(FILES ../flatpak/io.github.lostjared.AcidCam.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps
)
