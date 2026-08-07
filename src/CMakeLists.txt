cmake_minimum_required(VERSION 3.16)
project(Acid_Cam_v2_Qt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(ACIDCAM_STATIC_GNU_RUNTIME "Link libgcc and libstdc++ statically" ON)

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
    set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/lib/pkgconfig")
endif()

pkg_check_modules(ACIDCAM REQUIRED acidcam)
pkg_check_modules(SDL2 REQUIRED sdl2)

# Remove SDL2main from link libraries - this Qt app has its own main(),
# and SDL2main provides a conflicting main() wrapper.
list(REMOVE_ITEM SDL2_LIBRARIES SDL2main -lSDL2main)
list(FILTER SDL2_LDFLAGS EXCLUDE REGEX "SDL2main")

# Use find_package for OpenCV (pkg-config opencv4.pc not always available, e.g. MinGW)
find_package(OpenCV REQUIRED)
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
if(ACIDCAM_STATIC_GNU_RUNTIME AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
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
    RUNTIME DESTINATION bin
)
