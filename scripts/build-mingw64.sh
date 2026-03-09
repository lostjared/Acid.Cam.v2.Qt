#!/bin/bash
# build-mingw64.sh - Cross-compile libacidcam and Acid.Cam.v2.Qt with MinGW64 on Fedora
#
# Prerequisites (install on Fedora):
#   sudo dnf install mingw64-gcc-c++ mingw64-pkg-config \
#       mingw64-qt6-qtbase mingw64-qt6-qtbase-devel \
#       mingw64-opencv mingw64-SDL2 cmake make
#
# Usage:
#   ./build-mingw64.sh [clean]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLCHAIN_FILE="${SCRIPT_DIR}/mingw64-toolchain.cmake"
MINGW_SYSROOT="/usr/x86_64-w64-mingw32/sys-root/mingw"
JOBS=$(nproc 2>/dev/null || echo 4)

if [ "$1" = "clean" ]; then
    echo "=== Cleaning build directories ==="
    rm -rf "${SCRIPT_DIR}/libacidcam/build-mingw64"
    rm -rf "${SCRIPT_DIR}/Acid.Cam.v2.Qt/src/build-mingw64"
    echo "Done."
    exit 0
fi

echo "========================================"
echo " Cross-compiling with MinGW64 for Windows"
echo "========================================"
echo ""

# Step 1: Build and install libacidcam
echo "=== Building libacidcam ==="
mkdir -p "${SCRIPT_DIR}/libacidcam/build-mingw64"
cd "${SCRIPT_DIR}/libacidcam/build-mingw64"

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DACIDCAM_STATIC_GNU_RUNTIME=ON \
    -DACIDCAM_ENABLE_TESTS=OFF \
    -DACIDCAM_ENABLE_EXAMPLES=OFF \
    -DCMAKE_INSTALL_PREFIX="${MINGW_SYSROOT}"

cmake --build . -j "${JOBS}"

echo ""
echo "=== Installing libacidcam to MinGW sysroot ==="
echo "    (requires root if installing to system sysroot)"
sudo cmake --install .

echo ""
echo "=== libacidcam built and installed successfully ==="
echo ""

# Step 2: Build Acid.Cam.v2.Qt
echo "=== Building Acid.Cam.v2.Qt ==="
mkdir -p "${SCRIPT_DIR}/Acid.Cam.v2.Qt/src/build-mingw64"
cd "${SCRIPT_DIR}/Acid.Cam.v2.Qt/src/build-mingw64"

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DACIDCAM_STATIC_GNU_RUNTIME=ON

cmake --build . -j "${JOBS}"

echo ""
echo "========================================"
echo " Build complete!"
echo " Executable: Acid.Cam.v2.Qt/src/build-mingw64/Acid_Cam_v2_Qt.exe"
echo ""
echo " Run ./deploy-mingw64.sh to collect DLLs for deployment"
echo "========================================"
