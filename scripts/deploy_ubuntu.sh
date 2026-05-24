#!/usr/bin/env bash
set -euo pipefail

APP_NAME="${APP_NAME:-RhenoCalc}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
SOURCE_DIR="${SOURCE_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
BUILD_DIR="${BUILD_DIR:-${SOURCE_DIR}/cmake-build-${BUILD_TYPE,,}}"
DIST_ROOT="${DIST_ROOT:-${SOURCE_DIR}/dist/linux}"
DIST_APP_DIR="${DIST_ROOT}/${APP_NAME}"

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

mkdir -p "$BUILD_DIR"
mkdir -p "$DIST_ROOT"

run cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
run cmake --build "$BUILD_DIR" --target "$APP_NAME"

BIN_PATH="$BUILD_DIR/$APP_NAME"
if [[ ! -f "$BIN_PATH" ]]; then
  echo "Binary not found at $BIN_PATH" >&2
  exit 1
fi

rm -rf "$DIST_APP_DIR"
mkdir -p "$DIST_APP_DIR"
cp "$BIN_PATH" "$DIST_APP_DIR/$APP_NAME"

if command -v linuxdeployqt >/dev/null 2>&1; then
  run linuxdeployqt "$DIST_APP_DIR/$APP_NAME" -bundle-non-qt-libs
  echo "linuxdeployqt finished."
else
  echo "linuxdeployqt not found. Created staging folder only: $DIST_APP_DIR"
  echo "Install linuxdeployqt to bundle Qt libs, then run:"
  echo "  linuxdeployqt \"$DIST_APP_DIR/$APP_NAME\" -bundle-non-qt-libs"
fi

echo ""
echo "Deployment completed."
echo "App folder: $DIST_APP_DIR"

