# RhenoCalc

RhenoCalc is a lightweight native engineering toolbox for developers, embedded engineers, electronics engineers and IT professionals. It combines a scientific calculator, networking tools, electronics utilities, cryptographic functions and various engineering helpers into a single fast desktop application.

The app is optimized for quick utility workflows: it opens directly at the current mouse cursor position, operates via keyboard shortcuts, and closes instantly by pressing `ESC`.

[![GitHub Releases](https://img.shields.io/github/v/release/pgschwend/rhenocalc?label=Latest%20Release)](https://github.com/pgschwend/rhenocalc/releases)
[![License: GPLv3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)

---

## Key Characteristics

- **Performance:** Written in native C++20 and Qt6. It uses less than 50 MB of RAM and starts instantly.
- **Offline & Privacy:** Developed by a Swiss company. No cloud dependencies, no tracking, and no telemetry. The only network activity is a direct check for updates against the official GitHub Release URL.

---

## Availability

RhenoCalc is open-source. You can choose how to use it:

### 1. Pre-compiled Binaries
You can download the compiled binaries directly from the GitHub Releases page:
 **[Latest Release](https://github.com/pgschwend/rhenocalc/releases)**

### 2. App Stores (Optional Support)
For automated background updates and seamless OS integration, the app can be purchased for a small one-time fee in the official stores. This supports further independent development — or simply buys me a coffee ☕
- **Windows:** *[Coming soon to the Microsoft Store]*
- **macOS:** *[Coming soon to the Apple App Store]*

---

## Features

- **Scientific Calculator:** Arbitrary precision math powered by `Boost.Multiprecision`.
- **Programmer Mode:** Multiple number bases (Binary, Octal, Decimal, Hexadecimal) with live bit visualization and bit-width modes.
- **Math & Science:** Trigonometric functions, logarithms, and built-in constants ($\pi$, $e$).
- **IEEE 754 Inspector:** Visualizer for 32-bit (float) and 64-bit (double) binary representations.
- **Network Helper:** Local IPv4/CIDR subnet calculator.
- **Cryptography & Integrity:** Fast CRC (CRC-8/16/32) and cryptographic hash tools (MD5, SHA-1, SHA-256, SHA-512).
- **Electronics Helpers:** Ohm's law calculators and LED series resistor dimensioning.
- **Color Converter:** Hex/RGB/HSL conversion utility with a screen pipette.
- **Finance Utilities:** Calculators for compound interest and loan payments.
- **UI & Shortcuts:** Dark and Light themes with keyboard shortcuts.

## Requirements

- CMake 3.21+
- C++20 compiler (MinGW, GCC, or Clang)
- Qt 6 modules: Core, Gui, Widgets, Network
- Boost (fetched automatically via CMake FetchContent)

Important: your Qt kit and compiler must match the same architecture (for example both x64).

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Run

```bash
./build/RhenoCalc
```

On Windows, the output is `RhenoCalc.exe`. If Qt is detected correctly,
`windeployqt` is executed in a post-build step.

## Deployment Helpers

- Windows: `scripts/deploy_windows.ps1`
- Linux: `scripts/deploy_ubuntu.sh`
- macOS: `scripts/deploy_macos.sh`

These scripts help package the app with Qt runtime dependencies.
On Linux, the script can create an AppImage when `linuxdeploy` is installed.
On macOS, the script creates a `.app` bundle and optionally a DMG.

## Updates

RhenoCalc checks for updates on startup. When a new version is available,
a notification appears in the status bar with a link to the GitHub releases page.
Download the latest release manually from:
https://github.com/pgschwend/rhenocalc/releases

## Project Layout

- `src/pages/`: Qt UI widgets, layouts, and signal/slot wiring
- `src/core/`: calculation logic and reusable backend modules
- `src/ui/`: main window and shared UI theme handling
- `src/resources/`: icons, stylesheets, and Qt resource files
- `scripts/`: deployment and packaging scripts
