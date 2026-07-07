#!/usr/bin/env bash
set -euo pipefail

APP_NAME="${APP_NAME:-RhenoCalc}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
SOURCE_DIR="${SOURCE_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
BUILD_DIR="${BUILD_DIR:-${SOURCE_DIR}/cmake-build-${BUILD_TYPE,,}}"
DIST_ROOT="${DIST_ROOT:-${SOURCE_DIR}/dist/macos}"

# Extract version from info.h (e.g. "V0.1.2" → "0.1.2")
VERSION=$(grep -oE 'APP_VERSION_STRING\s+"V?[^"]+' "${SOURCE_DIR}/src/info.h" | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' || echo "0.0.0")
DMG_NAME="rhenocalc-macos-${VERSION}.dmg"

# Icon path
ICON_PATH="${SOURCE_DIR}/src/resources/icons/calculator.svg"

# Qt prefix (auto-detect from CMakeCache or use env)
if [[ -z "${CMAKE_PREFIX_PATH:-}" && -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  CMAKE_PREFIX_PATH=$(grep -oE 'CMAKE_PREFIX_PATH:PATH=.*' "${BUILD_DIR}/CMakeCache.txt" | cut -d= -f2 || true)
fi
QT_DIR="${CMAKE_PREFIX_PATH:-/usr/local/opt/qt}"
MACDEPLOYQT="${QT_DIR}/bin/macdeployqt"

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

if [[ ! -x "$MACDEPLOYQT" ]]; then
  # Try to find macdeployqt in PATH
  if command -v macdeployqt >/dev/null 2>&1; then
    MACDEPLOYQT=$(command -v macdeployqt)
  else
    echo "macdeployqt not found. Install Qt and ensure it's in PATH or set CMAKE_PREFIX_PATH." >&2
    exit 1
  fi
fi

mkdir -p "$BUILD_DIR"
mkdir -p "$DIST_ROOT"

run cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
run cmake --build "$BUILD_DIR" --target "$APP_NAME"

APP_BUNDLE="$BUILD_DIR/${APP_NAME}.app"
if [[ ! -d "$APP_BUNDLE" ]]; then
  echo "App bundle not found at $APP_BUNDLE" >&2
  exit 1
fi

# Clean previous distribution
DIST_APP="$DIST_ROOT/${APP_NAME}.app"
rm -rf "$DIST_APP"
rm -f "$DIST_ROOT/$DMG_NAME"

# Copy app bundle to distribution folder
cp -R "$APP_BUNDLE" "$DIST_APP"

# Run macdeployqt to bundle Qt frameworks
run "$MACDEPLOYQT" "$DIST_APP" -verbose=1

# Create DMG (optional, requires create-dmg or hdiutil)
if command -v create-dmg >/dev/null 2>&1; then
  echo "Creating DMG with create-dmg..."
  run create-dmg \
    --volname "$APP_NAME" \
    --window-pos 200 120 \
    --window-size 600 400 \
    --icon-size 100 \
    --icon "${APP_NAME}.app" 150 185 \
    --app-drop-link 450 185 \
    "$DIST_ROOT/$DMG_NAME" \
    "$DIST_APP"
elif command -v hdiutil >/dev/null 2>&1; then
  echo "Creating DMG with hdiutil..."
  run hdiutil create -volname "$APP_NAME" -srcfolder "$DIST_APP" -ov -format UDZO "$DIST_ROOT/$DMG_NAME"
else
  echo "Note: Neither create-dmg nor hdiutil found. Skipping DMG creation."
  echo "The app bundle is ready at: $DIST_APP"
fi

echo ""
echo "Deployment completed."
if [[ -f "$DIST_ROOT/$DMG_NAME" ]]; then
  echo "DMG: $DIST_ROOT/$DMG_NAME"
else
  echo "App bundle: $DIST_APP"
fi

