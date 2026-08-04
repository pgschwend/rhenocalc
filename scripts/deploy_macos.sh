#!/usr/bin/env bash
set -euo pipefail

# ==============================================================================
# Configuration & Setup
# ==============================================================================
APP_NAME="${APP_NAME:-RhenoCalc}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
SOURCE_DIR="${SOURCE_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
BUILD_TYPE_LOWER="$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')"
BUILD_DIR="${BUILD_DIR:-${SOURCE_DIR}/cmake-build-${BUILD_TYPE_LOWER}}"
DIST_ROOT="${DIST_ROOT:-${SOURCE_DIR}/dist/macos}"

# Extract marketing version (CFBundleShortVersionString)
VERSION="$(
  grep -oE 'project\([^)]*VERSION[[:space:]]+[0-9]+\.[0-9]+\.[0-9]+' "${SOURCE_DIR}/CMakeLists.txt" \
    | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' \
    || echo "0.0.0"
)"

# Extract build number (CFBundleVersion) from CMakeLists
BUNDLE_VERSION="$(
  sed -nE 's/^[[:space:]]*set\(APP_BUNDLE_VERSION[[:space:]]+"?([0-9]+)"?\).*/\1/p' "${SOURCE_DIR}/CMakeLists.txt" \
    | head -n1
)"
BUNDLE_VERSION="${BUNDLE_VERSION:-1}"

# Certificates & Entitlements
# For Mac App Store:
# - App signing identity: "Apple Distribution: <Company> (<TeamID>)"
# - Installer identity:   "3rd Party Mac Developer Installer: <Company> (<TeamID>)"
SIGN_APP_KEY="${SIGN_APP_KEY:-Apple Distribution: Rhenosys GmbH (JJNH23H5Y5)}"
INSTALLER_KEY="${INSTALLER_KEY:-3rd Party Mac Developer Installer: Rhenosys GmbH (JJNH23H5Y5)}"

# Entitlements:
# - APP_ENTITLEMENTS: for .app bundle
# - INHERITED_ENTITLEMENTS: for nested code (frameworks/plugins helpers), optional but recommended
APP_ENTITLEMENTS="${APP_ENTITLEMENTS:-${SOURCE_DIR}/packaging/apple/Entitlements.plist}"
INHERITED_ENTITLEMENTS="${INHERITED_ENTITLEMENTS:-${SOURCE_DIR}/packaging/apple/Entitlements.Inherit.plist}"

# Minimum macOS version for App Store build
MIN_MACOS_VERSION="${MIN_MACOS_VERSION:-12.0}"

# App Store category UTI
APP_CATEGORY="${APP_CATEGORY:-public.app-category.utilities}"

# ==============================================================================
# Helpers
# ==============================================================================
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

sign_path() {
  local target="$1"
  local entitlements="${2:-}"

  if [[ ! -e "$target" ]]; then
    return 0
  fi

  if [[ -n "$entitlements" && -f "$entitlements" ]]; then
    run codesign --force --timestamp --sign "$SIGN_APP_KEY" --entitlements "$entitlements" "$target"
  else
    run codesign --force --timestamp --sign "$SIGN_APP_KEY" "$target"
  fi
}

# ==============================================================================
# Prerequisites
# ==============================================================================
require_cmd cmake
require_cmd codesign
require_cmd productbuild
require_cmd plutil

# Resolve macdeployqt
if command -v macdeployqt >/dev/null 2>&1; then
  MACDEPLOYQT="$(command -v macdeployqt)"
else
  echo "Error: macdeployqt not found in PATH." >&2
  echo "Hint: export PATH=\"/path/to/Qt/<version>/macos/bin:\$PATH\"" >&2
  exit 1
fi

# Check signing identities exist
if ! security find-identity -v -p codesigning | grep -F "$SIGN_APP_KEY" >/dev/null 2>&1; then
  echo "Error: App signing identity not found in keychain:" >&2
  echo "  $SIGN_APP_KEY" >&2
  exit 1
fi

if ! security find-identity -v -p basic | grep -F "$INSTALLER_KEY" >/dev/null 2>&1; then
  echo "Error: Installer signing identity not found in keychain:" >&2
  echo "  $INSTALLER_KEY" >&2
  exit 1
fi

mkdir -p "$BUILD_DIR" "$DIST_ROOT"

# ==============================================================================
# Build Project
# ==============================================================================
run cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
run cmake --build "$BUILD_DIR" --target "$APP_NAME"

APP_BUNDLE="$BUILD_DIR/${APP_NAME}.app"
if [[ ! -d "$APP_BUNDLE" ]]; then
  echo "Error: App bundle not found at $APP_BUNDLE" >&2
  exit 1
fi

# Clean previous distribution
DIST_APP="$DIST_ROOT/${APP_NAME}.app"
rm -rf "$DIST_APP"

# Copy app bundle to distribution folder
run cp -R "$APP_BUNDLE" "$DIST_APP"

# Deploy Qt dependencies
run "$MACDEPLOYQT" "$DIST_APP" -verbose=1

# ==============================================================================
# Validate Bundle
# ==============================================================================
PLIST="$DIST_APP/Contents/Info.plist"
if [[ ! -d "$DIST_APP" ]]; then
  echo "Error: Deployed app bundle missing: $DIST_APP" >&2
  exit 1
fi
if [[ ! -f "$PLIST" ]]; then
  echo "Error: Info.plist missing: $PLIST" >&2
  echo "Bundle contents (debug):"
  find "$DIST_APP" -maxdepth 3 -print
  exit 1
fi

# ==============================================================================
# Info.plist Metadata Fixes for App Store
# ==============================================================================
echo "=== Patching Info.plist for App Store compliance ==="
run plutil -replace CFBundleSupportedPlatforms -json '["MacOSX"]' "$PLIST"
run plutil -replace LSMinimumSystemVersion -string "$MIN_MACOS_VERSION" "$PLIST"
run plutil -replace LSApplicationCategoryType -string "$APP_CATEGORY" "$PLIST"
run plutil -replace CFBundleVersion -string "$BUNDLE_VERSION" "$PLIST"

# Optional sanity print
echo "=== Info.plist Summary ==="
/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "$PLIST" 2>/dev/null || true
/usr/libexec/PlistBuddy -c "Print :CFBundleVersion" "$PLIST" 2>/dev/null || true
/usr/libexec/PlistBuddy -c "Print :LSMinimumSystemVersion" "$PLIST" 2>/dev/null || true
/usr/libexec/PlistBuddy -c "Print :LSApplicationCategoryType" "$PLIST" 2>/dev/null || true

# ==============================================================================
# Code Signing (Inside-Out)
# NOTE (MAS): no '--options runtime'
# ==============================================================================
echo "=== Signing nested code (inside-out) ==="

# 1) Sign dylibs/so/frameworks under Frameworks
if [[ -d "$DIST_APP/Contents/Frameworks" ]]; then
  while IFS= read -r -d '' f; do
    sign_path "$f" "$INHERITED_ENTITLEMENTS"
  done < <(find "$DIST_APP/Contents/Frameworks" -type f \( -name "*.dylib" -o -name "*.so" \) -print0)

  while IFS= read -r -d '' f; do
    sign_path "$f" "$INHERITED_ENTITLEMENTS"
  done < <(find "$DIST_APP/Contents/Frameworks" -type d -name "*.framework" -print0)
fi

# 2) Sign plugins and nested plugin bundles
if [[ -d "$DIST_APP/Contents/PlugIns" ]]; then
  while IFS= read -r -d '' f; do
    sign_path "$f" "$INHERITED_ENTITLEMENTS"
  done < <(find "$DIST_APP/Contents/PlugIns" -type f \( -name "*.dylib" -o -name "*.so" \) -print0)

  # Sign nested bundles (e.g. *.bundle, *.plugin if present)
  while IFS= read -r -d '' d; do
    sign_path "$d" "$INHERITED_ENTITLEMENTS"
  done < <(find "$DIST_APP/Contents/PlugIns" -type d \( -name "*.bundle" -o -name "*.plugin" \) -print0)
fi

# 3) Sign helper tools/executables (if any)
if [[ -d "$DIST_APP/Contents/MacOS" ]]; then
  while IFS= read -r -d '' exe; do
    # skip main executable, will be covered by signing app bundle
    if [[ "$exe" != "$DIST_APP/Contents/MacOS/$APP_NAME" ]]; then
      sign_path "$exe" "$INHERITED_ENTITLEMENTS"
    fi
  done < <(find "$DIST_APP/Contents/MacOS" -type f -perm -111 -print0)
fi

# 4) Finally sign the app bundle itself with app entitlements
echo "=== Signing app bundle ==="
if [[ -f "$APP_ENTITLEMENTS" ]]; then
  sign_path "$DIST_APP" "$APP_ENTITLEMENTS"
else
  echo "Warning: APP_ENTITLEMENTS not found at '$APP_ENTITLEMENTS' - signing app without explicit entitlements."
  sign_path "$DIST_APP"
fi

# ==============================================================================
# Verification
# ==============================================================================
echo "=== Verifying bundle signature ==="
run codesign --verify --deep --strict --verbose=2 "$DIST_APP"
run codesign -dv --verbose=4 "$DIST_APP"

echo "=== Gatekeeper assessment (informational) ==="
# For MAS builds this is informational; final validation is via App Store Connect / Transporter.
spctl --assess --type execute --verbose=4 "$DIST_APP" || true

# ==============================================================================
# App Store PKG Creation
# ==============================================================================
APP_NAME_LOWER="$(printf '%s' "$APP_NAME" | tr '[:upper:]' '[:lower:]')"
PKG_NAME="${APP_NAME_LOWER}-macos-${VERSION}.pkg"

echo "=== Building installer package (.pkg) ==="
run productbuild \
  --component "$DIST_APP" /Applications \
  --sign "$INSTALLER_KEY" \
  "$DIST_ROOT/$PKG_NAME"

echo "=== Verifying installer signature ==="
run pkgutil --check-signature "$DIST_ROOT/$PKG_NAME"

echo ""
echo "=== DONE ==="
echo "App Bundle:      $DIST_APP"
echo "App Store PKG:   $DIST_ROOT/$PKG_NAME"
echo "Version:         $VERSION"
echo "Build Number:    $BUNDLE_VERSION"