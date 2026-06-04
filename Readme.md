# RhenoCalc

RhenoCalc is a Qt desktop toolbox focused on practical engineering utilities.
It combines a calculator, converters, network tools, and CRC/hash functions in one app.


## Features

- Scientific calculator with arbitrary precision (Boost.Multiprecision)
- Multiple number bases (binary, octal, decimal, hexadecimal) and bit-width modes
- Trigonometric functions, logarithms, and constants (π, e)
- Base converter with live bit visualization
- Unit converter (length, weight, temperature, and more)
- IEEE 754 float inspector (32-bit and 64-bit)
- Network tools: IPv4/CIDR calculator, traceroute
- CRC and hash tools (CRC-8/16/32, MD5, SHA-1/256/512)
- Color picker with hex/RGB/HSL conversion and screen pipette
- Finance calculator (interest, loan payments)
- Electronics helpers (LED resistor, Ohm's law)
- Dark and light theme with keyboard shortcuts
- Auto-update mechanism

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

- Windows script: `scripts/deploy_windows.ps1`
- Linux script: `scripts/deploy_ubuntu.sh`

These scripts help package the app with Qt runtime dependencies.
On Linux, the script can create an AppImage when `linuxdeploy` is installed.

## Installation

### Windows

Place the application folder in a user-writable location such as
`%LOCALAPPDATA%\RhenoCalc` or any directory inside your user profile.
Avoid installing it under `C:\Program Files` or other system-protected paths,
because the built-in auto-update mechanism needs write access to replace files
and will fail without administrator privileges.

### Linux

Place the application folder in a user-writable location such as
`~/.local/share/RhenoCalc` or `~/RhenoCalc`.
Avoid installing it under `/usr/local` or `/opt` unless you want to run
updates with `sudo`. The auto-update mechanism requires write access to the
application directory.

For traceroute functionality, install the `traceroute` package:
```bash
sudo apt install traceroute
```

## Project Layout

- `src/pages/`: Qt UI widgets, layouts, and signal/slot wiring
- `src/core/`: calculation logic and reusable backend modules
- `src/ui/`: main window and shared UI theme handling
- `src/resources/`: icons, stylesheets, and Qt resource files
- `scripts/`: deployment and packaging scripts
