#!/bin/bash
# verify-mingw64.sh - Verify MinGW64 cross-compilation and deployment setup
#
# Checks:
#   1. libacidcam is statically linked into the exe (no DLL dependency)
#   2. The exe statically links libgcc and libstdc++
#   3. OpenCV imgcodecs links to the correct sysroot libstdc++
#   4. No duplicate/conflicting runtime DLLs
#   5. All DLL dependencies in the deploy directory are satisfied
#
# Usage:
#   ./verify-mingw64.sh [deploy_dir]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/Acid.Cam.v2.Qt/src/build-mingw64"
DEPLOY_DIR="${1:-${SCRIPT_DIR}/deploy-win64}"
MINGW_SYSROOT="/usr/x86_64-w64-mingw32/sys-root/mingw"
OBJDUMP="x86_64-w64-mingw32-objdump"
READELF="x86_64-w64-mingw32-readelf"
NM="x86_64-w64-mingw32-nm"
STRINGS="strings"

EXE_NAME="Acid_Cam_v2_Qt.exe"
PASS=0
FAIL=0
WARN=0

pass() { echo "  ✅ PASS: $1"; PASS=$((PASS + 1)); }
fail() { echo "  ❌ FAIL: $1"; FAIL=$((FAIL + 1)); }
warn() { echo "  ⚠️  WARN: $1"; WARN=$((WARN + 1)); }
info() { echo "  ℹ️  INFO: $1"; }
section() { echo ""; echo "=== $1 ==="; }

echo "========================================"
echo " MinGW64 Build & Deploy Verification"
echo "========================================"
echo " Build dir:  ${BUILD_DIR}"
echo " Deploy dir: ${DEPLOY_DIR}"
echo " Sysroot:    ${MINGW_SYSROOT}"

# ──────────────────────────────────────────────
section "1. Check build artifacts exist"
# ──────────────────────────────────────────────

if [ -f "${BUILD_DIR}/${EXE_NAME}" ]; then
    pass "Executable found: ${EXE_NAME}"
else
    fail "Executable not found: ${BUILD_DIR}/${EXE_NAME}"
    echo ""
    echo "Cannot continue without the exe. Run ./build-mingw64.sh first."
    exit 1
fi

# Check libacidcam was built as static
STATIC_LIB=$(find "${SCRIPT_DIR}/libacidcam/build-mingw64" -name "libacidcam.a" 2>/dev/null | head -1)
SHARED_LIB=$(find "${SCRIPT_DIR}/libacidcam/build-mingw64" -name "libacidcam*.dll" -o -name "acidcam*.dll" 2>/dev/null | head -1)

if [ -n "$STATIC_LIB" ]; then
    pass "libacidcam.a (static library) found in build dir"
else
    fail "libacidcam.a not found in build dir — was -DBUILD_SHARED_LIBS=OFF used?"
fi

if [ -n "$SHARED_LIB" ]; then
    fail "libacidcam DLL found in build dir: $(basename "$SHARED_LIB") — should be static only"
else
    pass "No libacidcam DLL in build dir (correct for static build)"
fi

# ──────────────────────────────────────────────
section "2. Check exe does NOT depend on libacidcam DLL"
# ──────────────────────────────────────────────

EXE_DLLS=$("${OBJDUMP}" -p "${BUILD_DIR}/${EXE_NAME}" 2>/dev/null | grep "DLL Name:" | awk '{print $3}')

if echo "$EXE_DLLS" | grep -qi "acidcam"; then
    fail "Exe dynamically links libacidcam — static linking failed!"
    echo "       DLL dependency: $(echo "$EXE_DLLS" | grep -i acidcam)"
else
    pass "Exe does NOT reference libacidcam DLL (statically linked)"
fi

# ──────────────────────────────────────────────
section "3. Check exe does NOT depend on libstdc++/libgcc DLLs"
# ──────────────────────────────────────────────

if echo "$EXE_DLLS" | grep -qi "libstdc++"; then
    fail "Exe dynamically links libstdc++-6.dll — add -static-libstdc++"
else
    pass "Exe does NOT reference libstdc++-6.dll (statically linked)"
fi

if echo "$EXE_DLLS" | grep -qi "libgcc"; then
    fail "Exe dynamically links libgcc_s_seh-1.dll — add -static-libgcc"
else
    pass "Exe does NOT reference libgcc_s_seh-1.dll (statically linked)"
fi

if echo "$EXE_DLLS" | grep -qi "libwinpthread"; then
    warn "Exe dynamically links libwinpthread-1.dll (may be OK if -Wl,-Bstatic -lpthread wasn't used)"
else
    pass "Exe does NOT reference libwinpthread-1.dll (statically linked)"
fi

# ──────────────────────────────────────────────
section "4. Check sysroot for stale libacidcam shared artifacts"
# ──────────────────────────────────────────────

STALE_DLL=$(find "${MINGW_SYSROOT}/bin" -maxdepth 1 \( -name "libacidcam*.dll" -o -name "acidcam*.dll" \) 2>/dev/null)
STALE_IMPLIB=$(find "${MINGW_SYSROOT}/lib" -maxdepth 1 \( -name "libacidcam*.dll.a" -o -name "acidcam*.dll.a" \) 2>/dev/null)

if [ -n "$STALE_DLL" ]; then
    fail "Stale libacidcam DLL in sysroot (will confuse deploy script):"
    echo "       $STALE_DLL"
    echo "       Fix: sudo rm -f ${MINGW_SYSROOT}/bin/libacidcam*.dll"
else
    pass "No stale libacidcam DLL in sysroot bin/"
fi

if [ -n "$STALE_IMPLIB" ]; then
    fail "Stale libacidcam import library in sysroot (linker may prefer this over static .a):"
    echo "       $STALE_IMPLIB"
    echo "       Fix: sudo rm -f ${MINGW_SYSROOT}/lib/libacidcam*.dll.a"
else
    pass "No stale libacidcam import library (.dll.a) in sysroot lib/"
fi

# Check the static lib IS installed
INSTALLED_STATIC=$(find "${MINGW_SYSROOT}/lib" -maxdepth 1 -name "libacidcam.a" 2>/dev/null | head -1)
if [ -n "$INSTALLED_STATIC" ]; then
    pass "libacidcam.a installed in sysroot: ${INSTALLED_STATIC}"
else
    warn "libacidcam.a not found in sysroot lib/ — may not be installed yet"
fi

# ──────────────────────────────────────────────
section "5. Check OpenCV imgcodecs DLL dependencies"
# ──────────────────────────────────────────────

IMGCODECS_DLL=$(find "${MINGW_SYSROOT}/bin" -maxdepth 1 -iname "libopencv_imgcodecs*.dll" 2>/dev/null | head -1)

if [ -z "$IMGCODECS_DLL" ]; then
    warn "opencv_imgcodecs DLL not found in sysroot — checking deploy dir"
    IMGCODECS_DLL=$(find "${DEPLOY_DIR}" -maxdepth 1 -iname "libopencv_imgcodecs*.dll" 2>/dev/null | head -1)
fi

if [ -n "$IMGCODECS_DLL" ]; then
    info "Checking: $(basename "$IMGCODECS_DLL")"
    IMGCODECS_DEPS=$("${OBJDUMP}" -p "$IMGCODECS_DLL" 2>/dev/null | grep "DLL Name:" | awk '{print $3}')

    # Check it links to libstdc++
    if echo "$IMGCODECS_DEPS" | grep -qi "libstdc++"; then
        info "imgcodecs depends on libstdc++-6.dll (expected for pre-built OpenCV)"
    fi

    # Show GCC version embedded in imgcodecs
    IMGCODECS_GCC=$("${STRINGS}" "$IMGCODECS_DLL" 2>/dev/null | grep "GCC:" | head -1)
    if [ -n "$IMGCODECS_GCC" ]; then
        info "imgcodecs built with: $IMGCODECS_GCC"
    fi

    # Show GCC version of the sysroot's libstdc++
    SYSROOT_STDCXX="${MINGW_SYSROOT}/bin/libstdc++-6.dll"
    if [ -f "$SYSROOT_STDCXX" ]; then
        STDCXX_GCC=$("${STRINGS}" "$SYSROOT_STDCXX" 2>/dev/null | grep "GCC:" | head -1)
        if [ -n "$STDCXX_GCC" ]; then
            info "sysroot libstdc++ built with: $STDCXX_GCC"
        fi

        # Compare GCC major versions
        IMGCODECS_VER=$(echo "$IMGCODECS_GCC" | grep -oP '\d+\.\d+\.\d+' | head -1)
        STDCXX_VER=$(echo "$STDCXX_GCC" | grep -oP '\d+\.\d+\.\d+' | head -1)
        if [ -n "$IMGCODECS_VER" ] && [ -n "$STDCXX_VER" ]; then
            IMG_MAJOR=$(echo "$IMGCODECS_VER" | cut -d. -f1)
            STD_MAJOR=$(echo "$STDCXX_VER" | cut -d. -f1)
            if [ "$IMG_MAJOR" = "$STD_MAJOR" ]; then
                pass "GCC major version matches: imgcodecs=$IMGCODECS_VER, libstdc++=$STDCXX_VER"
            else
                fail "GCC major version MISMATCH: imgcodecs=$IMGCODECS_VER, libstdc++=$STDCXX_VER"
                echo "       This WILL cause 'entry point not found' errors!"
            fi
        fi
    fi

    # Show all imgcodecs dependencies
    echo ""
    info "Full imgcodecs DLL dependencies:"
    for dep in $IMGCODECS_DEPS; do
        echo "       - $dep"
    done
else
    warn "opencv_imgcodecs DLL not found anywhere"
fi

# ──────────────────────────────────────────────
section "6. Check current toolchain GCC version"
# ──────────────────────────────────────────────

TOOLCHAIN_VER=$(x86_64-w64-mingw32-g++ --version 2>/dev/null | head -1)
if [ -n "$TOOLCHAIN_VER" ]; then
    info "Toolchain: $TOOLCHAIN_VER"
fi

# Check RPM versions if on Fedora
if command -v rpm &>/dev/null; then
    OPENCV_RPM=$(rpm -q mingw64-opencv 2>/dev/null || echo "not installed")
    GCC_RPM=$(rpm -q mingw64-gcc-c++ 2>/dev/null || echo "not installed")
    info "mingw64-opencv:  $OPENCV_RPM"
    info "mingw64-gcc-c++: $GCC_RPM"
fi

# ──────────────────────────────────────────────
section "7. Verify deploy directory (if exists)"
# ──────────────────────────────────────────────

if [ ! -d "$DEPLOY_DIR" ]; then
    warn "Deploy directory does not exist yet — run ./deploy-mingw64.sh"
else
    DEPLOY_EXE="${DEPLOY_DIR}/${EXE_NAME}"
    if [ ! -f "$DEPLOY_EXE" ]; then
        warn "Exe not found in deploy dir"
    else
        pass "Exe found in deploy dir"

        # Check libacidcam DLL is NOT in deploy dir (should be static)
        DEPLOY_ACIDCAM=$(find "${DEPLOY_DIR}" -maxdepth 1 \( -name "libacidcam*.dll" -o -name "acidcam*.dll" \) 2>/dev/null)
        if [ -n "$DEPLOY_ACIDCAM" ]; then
            fail "Stale libacidcam DLL in deploy dir (should not be there for static build):"
            echo "       $DEPLOY_ACIDCAM"
            echo "       Fix: rm -f ${DEPLOY_DIR}/libacidcam*.dll"
        else
            pass "No libacidcam DLL in deploy dir (correct for static build)"
        fi

        # Check libstdc++ is present (needed by OpenCV DLLs)
        if [ -f "${DEPLOY_DIR}/libstdc++-6.dll" ]; then
            pass "libstdc++-6.dll present in deploy dir (needed by OpenCV)"

            # Verify it's the sysroot copy, not some other version
            if [ -f "$SYSROOT_STDCXX" ]; then
                DEPLOY_MD5=$(md5sum "${DEPLOY_DIR}/libstdc++-6.dll" 2>/dev/null | awk '{print $1}')
                SYSROOT_MD5=$(md5sum "$SYSROOT_STDCXX" 2>/dev/null | awk '{print $1}')
                if [ "$DEPLOY_MD5" = "$SYSROOT_MD5" ]; then
                    pass "libstdc++-6.dll matches sysroot copy (correct version)"
                else
                    fail "libstdc++-6.dll does NOT match sysroot copy!"
                    echo "       The deployed copy may be from a different GCC version."
                    echo "       Fix: cp ${SYSROOT_STDCXX} ${DEPLOY_DIR}/"
                fi
            fi
        else
            fail "libstdc++-6.dll MISSING from deploy dir (OpenCV DLLs need it!)"
        fi

        if [ -f "${DEPLOY_DIR}/libgcc_s_seh-1.dll" ]; then
            pass "libgcc_s_seh-1.dll present in deploy dir"
        else
            fail "libgcc_s_seh-1.dll MISSING from deploy dir"
        fi

        if [ -f "${DEPLOY_DIR}/libwinpthread-1.dll" ]; then
            pass "libwinpthread-1.dll present in deploy dir"
        else
            warn "libwinpthread-1.dll not in deploy dir (may be statically linked)"
        fi

        # Check ALL DLL dependencies are satisfied
        echo ""
        info "Scanning all deployed binaries for missing dependencies..."

        # Windows system DLLs to ignore
        SYSTEM_DLLS_PATTERN="^(kernel32|user32|gdi32|msvcrt|advapi32|shell32|ole32|oleaut32|ws2_32|mswsock|ntdll|winmm|comdlg32|imm32|version|opengl32|glu32|setupapi|cfgmgr32|crypt32|dwmapi|uxtheme|userenv|netapi32|iphlpapi|dnsapi|secur32|bcrypt|ncrypt|shlwapi|winspool|wldap32|odbc32|odbccp32|authz|mpr|d3d9|d3d11|d3d12|d2d1|dwrite|dxgi|winhttp|shcore|wtsapi32)\.dll$"

        MISSING_TOTAL=0
        for binary in "${DEPLOY_DIR}/${EXE_NAME}" "${DEPLOY_DIR}/"*.dll; do
            [ -f "$binary" ] || continue
            DEPS=$("${OBJDUMP}" -p "$binary" 2>/dev/null | grep "DLL Name:" | awk '{print $3}')
            for dep in $DEPS; do
                dep_lower=$(echo "$dep" | tr '[:upper:]' '[:lower:]')
                # Skip system DLLs
                if echo "$dep_lower" | grep -qPi "$SYSTEM_DLLS_PATTERN"; then continue; fi
                if [[ "$dep_lower" == api-ms-win-* ]] || [[ "$dep_lower" == ext-ms-* ]]; then continue; fi

                if [ ! -f "${DEPLOY_DIR}/${dep}" ]; then
                    # Case-insensitive search in deploy dir
                    FOUND=$(find "${DEPLOY_DIR}" -maxdepth 1 -iname "$dep" 2>/dev/null | head -1)
                    if [ -z "$FOUND" ]; then
                        echo "       MISSING: ${dep}  (needed by $(basename "$binary"))"
                        MISSING_TOTAL=$((MISSING_TOTAL + 1))
                    fi
                fi
            done
        done

        # Also check plugin subdirectories
        for subdir in platforms styles imageformats tls networkinformation; do
            for plugin_dll in "${DEPLOY_DIR}/${subdir}/"*.dll; do
                [ -f "$plugin_dll" ] || continue
                DEPS=$("${OBJDUMP}" -p "$plugin_dll" 2>/dev/null | grep "DLL Name:" | awk '{print $3}')
                for dep in $DEPS; do
                    dep_lower=$(echo "$dep" | tr '[:upper:]' '[:lower:]')
                    if echo "$dep_lower" | grep -qPi "$SYSTEM_DLLS_PATTERN"; then continue; fi
                    if [[ "$dep_lower" == api-ms-win-* ]] || [[ "$dep_lower" == ext-ms-* ]]; then continue; fi

                    if [ ! -f "${DEPLOY_DIR}/${dep}" ]; then
                        FOUND=$(find "${DEPLOY_DIR}" -maxdepth 1 -iname "$dep" 2>/dev/null | head -1)
                        if [ -z "$FOUND" ]; then
                            echo "       MISSING: ${dep}  (needed by ${subdir}/$(basename "$plugin_dll"))"
                            MISSING_TOTAL=$((MISSING_TOTAL + 1))
                        fi
                    fi
                done
            done
        done

        if [ "$MISSING_TOTAL" -eq 0 ]; then
            pass "All DLL dependencies satisfied in deploy dir"
        else
            fail "${MISSING_TOTAL} DLL(s) missing from deploy dir"
        fi
    fi
fi

# ──────────────────────────────────────────────
section "Summary"
# ──────────────────────────────────────────────

echo ""
echo "  Results:  ${PASS} passed, ${FAIL} failed, ${WARN} warnings"
echo ""

if [ "$FAIL" -gt 0 ]; then
    echo "  ❌ Some checks FAILED — fix issues above before deploying."
    exit 1
else
    echo "  ✅ All checks passed!"
    exit 0
fi
