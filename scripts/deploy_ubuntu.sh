#!/usr/bin/env bash
set -euo pipefail

APP_NAME="${APP_NAME:-RhenoCalc}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
SOURCE_DIR="${SOURCE_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
BUILD_DIR="${BUILD_DIR:-${SOURCE_DIR}/cmake-build-${BUILD_TYPE,,}}"
DIST_ROOT="${DIST_ROOT:-${SOURCE_DIR}/dist/linux}"
DIST_APP_DIR="${DIST_ROOT}/${APP_NAME}"

# Extract version from info.h (e.g. "V0.1.2" → "0.1.2")
VERSION=$(grep -oP 'APP_VERSION_STRING\s+"V?\K[^"]+' "${SOURCE_DIR}/src/info.h" || echo "0.0.0")
APPIMAGE_NAME="rhenocalc-linux-x86_64_v${VERSION}.AppImage"

# Paths to icon and desktop file
ICON_PATH="${SOURCE_DIR}/src/resources/icons/calculator.svg"
DESKTOP_PATH="${SOURCE_DIR}/scripts/rhenocalc.desktop"

# Qt prefix (auto-detect from CMakeCache or use env)
if [[ -z "${CMAKE_PREFIX_PATH:-}" && -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  CMAKE_PREFIX_PATH=$(grep -oP 'CMAKE_PREFIX_PATH:PATH=\K.*' "${BUILD_DIR}/CMakeCache.txt" || true)
fi
export QMAKE="${CMAKE_PREFIX_PATH:-/opt/qt/6.9.3/gcc_64}/bin/qmake"

run() {
  echo "> $*"
  "$@"
}

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Missing required command: $1" >&2
    exit 1
  fi
}

require_cmd cmake
require_cmd linuxdeploy

mkdir -p "$BUILD_DIR"
mkdir -p "$DIST_ROOT"

run cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
run cmake --build "$BUILD_DIR" --target "$APP_NAME"

BIN_PATH="$BUILD_DIR/$APP_NAME"
if [[ ! -f "$BIN_PATH" ]]; then
  echo "Binary not found at $BIN_PATH" >&2
  exit 1
fi

# Clean previous AppDir
rm -rf "$DIST_APP_DIR"

# Run linuxdeploy with qt plugin to create AppImage
export OUTPUT="${DIST_ROOT}/${APPIMAGE_NAME}"
run linuxdeploy \
  --appdir "$DIST_APP_DIR" \
  --executable "$BIN_PATH" \
  --desktop-file "$DESKTOP_PATH" \
  --icon-file "$ICON_PATH" \
  --plugin qt \
  --output appimage

# Clean up AppDir (only the AppImage is needed)
rm -rf "$DIST_APP_DIR"

echo ""
echo "Deployment completed."
echo "AppImage: ${DIST_ROOT}/${APPIMAGE_NAME}"
