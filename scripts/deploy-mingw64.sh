#!/bin/bash
# deploy-mingw64.sh - Collect DLLs and Qt plugins for Windows deployment
#
# Copies the built executable, all required DLLs from the MinGW sysroot,
# and Qt plugins into a self-contained deployment directory.
#
# Usage:
#   ./deploy-mingw64.sh [deploy_dir]
#
# Default deploy directory: ./deploy-win64/

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/Acid.Cam.v2.Qt/src/build-mingw64"
DEPLOY_DIR="${1:-${SCRIPT_DIR}/deploy-win64}"
MINGW_SYSROOT="/usr/x86_64-w64-mingw32/sys-root/mingw"
OBJDUMP="x86_64-w64-mingw32-objdump"

EXE_NAME="Acid_Cam_v2_Qt.exe"

# Verify the executable exists
if [ ! -f "${BUILD_DIR}/${EXE_NAME}" ]; then
    echo "ERROR: ${BUILD_DIR}/${EXE_NAME} not found."
    echo "Run ./build-mingw64.sh first."
    exit 1
fi

echo "========================================"
echo " Deploying Acid Cam v2 Qt for Windows"
echo " Deploy directory: ${DEPLOY_DIR}"
echo "========================================"
echo ""

# Create deployment directory structure
mkdir -p "${DEPLOY_DIR}"
mkdir -p "${DEPLOY_DIR}/platforms"
mkdir -p "${DEPLOY_DIR}/styles"
mkdir -p "${DEPLOY_DIR}/imageformats"
mkdir -p "${DEPLOY_DIR}/plugins"

# Copy the executable
echo "Copying executable..."
cp "${BUILD_DIR}/${EXE_NAME}" "${DEPLOY_DIR}/"

# Windows system DLLs that should NOT be copied (provided by Windows itself)
SYSTEM_DLLS="
KERNEL32.dll kernel32.dll
USER32.dll user32.dll
GDI32.dll gdi32.dll
MSVCRT.dll msvcrt.dll
ADVAPI32.dll advapi32.dll
SHELL32.dll shell32.dll
OLE32.dll ole32.dll
OLEAUT32.dll oleaut32.dll
WS2_32.dll ws2_32.dll
MSWSOCK.dll mswsock.dll
NTDLL.dll ntdll.dll
WINMM.dll winmm.dll
COMDLG32.dll comdlg32.dll
IMM32.dll imm32.dll
VERSION.dll version.dll
OPENGL32.dll opengl32.dll
GLU32.dll glu32.dll
SETUPAPI.dll setupapi.dll
CFGMGR32.dll cfgmgr32.dll
CRYPT32.dll crypt32.dll
WINSPOOL.DRV winspool.drv
DWMAPI.dll dwmapi.dll
UXTHEME.dll uxtheme.dll
USERENV.dll userenv.dll
NETAPI32.dll netapi32.dll
IPHLPAPI.dll iphlpapi.dll
DNSAPI.dll dnsapi.dll
SECUR32.dll secur32.dll
BCRYPT.dll bcrypt.dll
NCRYPT.dll ncrypt.dll
SHLWAPI.dll shlwapi.dll
api-ms-win-crt-runtime-l1-1-0.dll
api-ms-win-crt-stdio-l1-1-0.dll
api-ms-win-crt-heap-l1-1-0.dll
api-ms-win-crt-string-l1-1-0.dll
api-ms-win-crt-math-l1-1-0.dll
api-ms-win-crt-locale-l1-1-0.dll
api-ms-win-crt-time-l1-1-0.dll
api-ms-win-crt-environment-l1-1-0.dll
api-ms-win-crt-filesystem-l1-1-0.dll
api-ms-win-crt-convert-l1-1-0.dll
api-ms-win-crt-process-l1-1-0.dll
api-ms-win-crt-utility-l1-1-0.dll
"

# Check if a DLL name is a Windows system DLL
is_system_dll() {
    local dll_name="$1"
    local dll_lower
    dll_lower=$(echo "$dll_name" | tr '[:upper:]' '[:lower:]')

    # Check against known system DLLs
    for sys_dll in ${SYSTEM_DLLS}; do
        sys_lower=$(echo "$sys_dll" | tr '[:upper:]' '[:lower:]')
        if [ "$dll_lower" = "$sys_lower" ]; then
            return 0
        fi
    done

    # Skip api-ms-win-* DLLs (Windows API sets)
    if [[ "$dll_lower" == api-ms-win-* ]]; then
        return 0
    fi

    # Skip ext-ms-* DLLs
    if [[ "$dll_lower" == ext-ms-* ]]; then
        return 0
    fi

    return 1
}

# Recursively find and copy DLL dependencies
COPIED_DLLS=""
copy_dll_deps() {
    local binary="$1"
    local dll_list

    if [ ! -f "$binary" ]; then
        return
    fi

    dll_list=$("${OBJDUMP}" -p "$binary" 2>/dev/null | grep "DLL Name:" | awk '{print $3}')

    for dll in $dll_list; do
        # Skip if already copied
        if echo "$COPIED_DLLS" | grep -qi "^${dll}$"; then
            continue
        fi

        # Skip Windows system DLLs
        if is_system_dll "$dll"; then
            continue
        fi

        # Find the DLL in the MinGW sysroot
        local dll_path
        dll_path=$(find "${MINGW_SYSROOT}/bin" -maxdepth 1 -iname "$dll" 2>/dev/null | head -1)

        if [ -z "$dll_path" ]; then
            # Also search in lib directory
            dll_path=$(find "${MINGW_SYSROOT}/lib" -maxdepth 1 -iname "$dll" 2>/dev/null | head -1)
        fi

        if [ -n "$dll_path" ] && [ ! -f "${DEPLOY_DIR}/${dll}" ]; then
            cp "$dll_path" "${DEPLOY_DIR}/"
            echo "  Copied: ${dll}"
            COPIED_DLLS="${COPIED_DLLS}${dll}\n"

            # Recurse into this DLL's dependencies
            copy_dll_deps "$dll_path"
        elif [ -z "$dll_path" ]; then
            echo "  WARNING: ${dll} not found in MinGW sysroot"
        fi
    done
}

# Copy DLL dependencies for the executable
echo ""
echo "Resolving DLL dependencies..."
copy_dll_deps "${DEPLOY_DIR}/${EXE_NAME}"

# Copy libacidcam DLL (may have been installed to the sysroot)
echo ""
echo "Checking for libacidcam DLL..."
ACIDCAM_DLL=$(find "${MINGW_SYSROOT}/bin" -maxdepth 1 -name "libacidcam*.dll" -o -name "acidcam*.dll" 2>/dev/null | head -1)
if [ -z "$ACIDCAM_DLL" ]; then
    # Check the libacidcam build directory as fallback
    ACIDCAM_DLL=$(find "${SCRIPT_DIR}/libacidcam/build-mingw64" -maxdepth 1 -name "libacidcam*.dll" -o -name "acidcam*.dll" 2>/dev/null | head -1)
fi
if [ -n "$ACIDCAM_DLL" ]; then
    cp "$ACIDCAM_DLL" "${DEPLOY_DIR}/"
    echo "  Copied: $(basename "$ACIDCAM_DLL")"
    # Also resolve acidcam's own DLL dependencies
    copy_dll_deps "$ACIDCAM_DLL"
else
    echo "  WARNING: libacidcam DLL not found"
fi

# Copy Qt plugins
echo ""
echo "Copying Qt plugins..."

# Determine Qt version and plugin path
QT6_PLUGIN_DIR="${MINGW_SYSROOT}/lib/qt6/plugins"
QT5_PLUGIN_DIR="${MINGW_SYSROOT}/lib/qt5/plugins"

if [ -d "$QT6_PLUGIN_DIR" ]; then
    QT_PLUGIN_DIR="$QT6_PLUGIN_DIR"
    echo "  Using Qt6 plugins from: ${QT_PLUGIN_DIR}"
elif [ -d "$QT5_PLUGIN_DIR" ]; then
    QT_PLUGIN_DIR="$QT5_PLUGIN_DIR"
    echo "  Using Qt5 plugins from: ${QT_PLUGIN_DIR}"
else
    # Try alternate paths
    QT_PLUGIN_DIR=$(find "${MINGW_SYSROOT}" -type d -name "plugins" -path "*/qt*" 2>/dev/null | head -1)
    if [ -n "$QT_PLUGIN_DIR" ]; then
        echo "  Using Qt plugins from: ${QT_PLUGIN_DIR}"
    else
        echo "  WARNING: Qt plugin directory not found"
    fi
fi

if [ -n "$QT_PLUGIN_DIR" ] && [ -d "$QT_PLUGIN_DIR" ]; then
    # platforms (required - qwindows.dll)
    if [ -d "${QT_PLUGIN_DIR}/platforms" ]; then
        cp "${QT_PLUGIN_DIR}/platforms/"*.dll "${DEPLOY_DIR}/platforms/" 2>/dev/null && \
            echo "  Copied platforms plugins" || true
        # Resolve plugin DLL dependencies
        for plugin_dll in "${DEPLOY_DIR}/platforms/"*.dll; do
            [ -f "$plugin_dll" ] && copy_dll_deps "$plugin_dll"
        done
    fi

    # styles
    if [ -d "${QT_PLUGIN_DIR}/styles" ]; then
        cp "${QT_PLUGIN_DIR}/styles/"*.dll "${DEPLOY_DIR}/styles/" 2>/dev/null && \
            echo "  Copied styles plugins" || true
    fi

    # imageformats
    if [ -d "${QT_PLUGIN_DIR}/imageformats" ]; then
        cp "${QT_PLUGIN_DIR}/imageformats/"*.dll "${DEPLOY_DIR}/imageformats/" 2>/dev/null && \
            echo "  Copied imageformats plugins" || true
    fi

    # tls (for Qt Network SSL support)
    if [ -d "${QT_PLUGIN_DIR}/tls" ]; then
        mkdir -p "${DEPLOY_DIR}/tls"
        cp "${QT_PLUGIN_DIR}/tls/"*.dll "${DEPLOY_DIR}/tls/" 2>/dev/null && \
            echo "  Copied tls plugins" || true
    fi

    # networkinformation
    if [ -d "${QT_PLUGIN_DIR}/networkinformation" ]; then
        mkdir -p "${DEPLOY_DIR}/networkinformation"
        cp "${QT_PLUGIN_DIR}/networkinformation/"*.dll "${DEPLOY_DIR}/networkinformation/" 2>/dev/null && \
            echo "  Copied networkinformation plugins" || true
    fi
fi

# Create a qt.conf to tell Qt where to find plugins
cat > "${DEPLOY_DIR}/qt.conf" << 'EOF'
[Paths]
Plugins = .
EOF

# Create a plugins/ directory note
echo ""
echo "NOTE: Create a 'plugins/' subdirectory in the deploy folder"
echo "      and place any Acid Cam filter plugin .dll files there."

# Summary
echo ""
echo "========================================"
echo " Deployment complete!"
echo " Directory: ${DEPLOY_DIR}"
echo ""
echo " Contents:"
ls -1 "${DEPLOY_DIR}/" | head -40
TOTAL_DLLS=$(find "${DEPLOY_DIR}" -name "*.dll" | wc -l)
echo ""
echo " Total DLLs: ${TOTAL_DLLS}"
echo ""
echo " NOTE: Make sure ffmpeg.exe is available in PATH on the"
echo "       target Windows system for video encoding features."
echo ""
echo " For Wine testing, remove OpenEXR DLLs (Wine doesn't support them):"
echo "   rm -f ${DEPLOY_DIR}/libOpenEXR*.dll ${DEPLOY_DIR}/libopenexr*.dll \\"
echo "         ${DEPLOY_DIR}/libImath*.dll ${DEPLOY_DIR}/libimath*.dll \\"
echo "         ${DEPLOY_DIR}/libIex*.dll ${DEPLOY_DIR}/libiex*.dll \\"
echo "         ${DEPLOY_DIR}/libIlmThread*.dll ${DEPLOY_DIR}/libilmthread*.dll \\"
echo "         ${DEPLOY_DIR}/libOpenEXRCore*.dll ${DEPLOY_DIR}/libopenexrcore*.dll"
echo "========================================"
