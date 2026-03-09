# CMake Toolchain File for MinGW64 cross-compilation on Fedora
#
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=../mingw64-toolchain.cmake ..
#
# Or use Fedora's wrapper (if available):
#   mingw64-cmake ..

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# MinGW compilers
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# MinGW sysroot on Fedora
# NOTE: Do NOT set CMAKE_SYSROOT — it passes --sysroot to the compiler/linker
# which breaks MinGW's ability to find its own CRT files (crt2.o).
# CMAKE_FIND_ROOT_PATH is sufficient for CMake's find_* commands.
set(MINGW_SYSROOT "/usr/x86_64-w64-mingw32/sys-root/mingw")
set(CMAKE_FIND_ROOT_PATH "${MINGW_SYSROOT}")

# Search paths: only search the cross-compile sysroot for libraries/headers,
# but use host tools (compilers, pkg-config, etc.)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Library suffixes for MinGW
set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a" ".a" ".lib")

# pkg-config for MinGW cross-compilation
# Fedora provides mingw64-pkg-config or x86_64-w64-mingw32-pkg-config
find_program(PKG_CONFIG_EXECUTABLE
    NAMES mingw64-pkg-config x86_64-w64-mingw32-pkg-config
    PATHS /usr/bin
)
if(NOT PKG_CONFIG_EXECUTABLE)
    # Fallback: use native pkg-config with cross PKG_CONFIG_LIBDIR
    find_program(PKG_CONFIG_EXECUTABLE pkg-config)
    set(ENV{PKG_CONFIG_LIBDIR} "${MINGW_SYSROOT}/lib/pkgconfig")
    set(ENV{PKG_CONFIG_PATH} "")
    set(ENV{PKG_CONFIG_SYSROOT_DIR} "${MINGW_SYSROOT}")
endif()
message(STATUS "Using pkg-config: ${PKG_CONFIG_EXECUTABLE}")

# Qt6 host tools path (needed for moc, rcc, uic during cross-compilation)
# On Fedora, native Qt6 tools are at /usr/lib64/qt6/libexec/
if(EXISTS "/usr/lib64/qt6")
    set(QT_HOST_PATH "/usr" CACHE PATH "Path to host Qt installation")
endif()

# Install prefix defaults to MinGW sysroot for libraries
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${MINGW_SYSROOT}" CACHE PATH "Install prefix" FORCE)
endif()
