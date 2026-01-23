cmake_minimum_required(VERSION 3.16)
project(Acid_Cam_v2_Qt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL Network QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui Widgets OpenGL Network)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()


find_package(OpenGL REQUIRED)


find_package(PkgConfig REQUIRED)
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/lib/pkgconfig")

pkg_check_modules(ACIDCAM REQUIRED acidcam)
pkg_check_modules(OPENCV4 REQUIRED opencv4)
pkg_check_modules(SDL2 REQUIRED sdl2)


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
)


set(RESOURCES
    qresource.qrc
)


add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)


target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ACIDCAM_INCLUDE_DIRS}
    ${OPENCV4_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)


target_link_directories(${PROJECT_NAME} PRIVATE
    ${ACIDCAM_LIBRARY_DIRS}
    ${OPENCV4_LIBRARY_DIRS}
    ${SDL2_LIBRARY_DIRS}
)


target_compile_options(${PROJECT_NAME} PRIVATE
    ${ACIDCAM_CFLAGS_OTHER}
    ${OPENCV4_CFLAGS_OTHER}
    ${SDL2_CFLAGS_OTHER}
)


if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::Network
        OpenGL::GL
        ${ACIDCAM_LIBRARIES}
        ${OPENCV4_LIBRARIES}
        ${SDL2_LIBRARIES}
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::OpenGL
        Qt5::Network
        OpenGL::GL
        ${ACIDCAM_LIBRARIES}
        ${OPENCV4_LIBRARIES}
        ${SDL2_LIBRARIES}
    )
endif()


install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
