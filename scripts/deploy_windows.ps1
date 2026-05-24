param(
    [string]$BuildType = "Release",
    [string]$SourceDir = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$BuildDir,
    [string]$DistDir,
    [string]$AppName = "RhenoCalc",
    [string]$QtBinDir,
    [string]$CMakeExe,
    [string]$Generator,
    [string]$MakeProgram,
    [string]$CCompiler,
    [string]$CxxCompiler,
    [ValidateSet("auto", "release", "debug")]
    [string]$DeployMode = "auto",
    [switch]$SkipConfigure,
    [switch]$SkipBuild,
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

function Resolve-CMakeExe {
    param([string]$UserProvided)

    if ($UserProvided) {
        return (Resolve-Path $UserProvided).Path
    }

    $cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmakeCmd) {
        return $cmakeCmd.Source
    }

    $fallback = "C:\Program Files\JetBrains\CLion 2026.1\bin\cmake\win\x64\bin\cmake.exe"
    if (Test-Path $fallback) {
        return $fallback
    }

    throw "cmake was not found. Install cmake or pass -CMakeExe <path>."
}

function Resolve-WindeployqtExe {
    param([string]$UserQtBinDir)

    if ($UserQtBinDir) {
        $candidate = Join-Path $UserQtBinDir "windeployqt.exe"
        if (Test-Path $candidate) {
            return $candidate
        }
        throw "windeployqt.exe not found in -QtBinDir '$UserQtBinDir'."
    }

    $fromPath = Get-Command windeployqt.exe -ErrorAction SilentlyContinue
    if ($fromPath) {
        return $fromPath.Source
    }

    # Fast fallback lookup under standard Qt installation layout.
    $fallbackCandidates = @(
        "C:\Qt\6.9.3\mingw_64\bin\windeployqt.exe",
        "C:\Qt\6.8.0\mingw_64\bin\windeployqt.exe"
    )

    foreach ($candidate in $fallbackCandidates) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    $qtRoot = "C:\Qt"
    if (Test-Path $qtRoot) {
        $qtVersions = Get-ChildItem -Path $qtRoot -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending
        foreach ($ver in $qtVersions) {
            $mingwDirs = Get-ChildItem -Path $ver.FullName -Directory -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -like "mingw*" }
            foreach ($mingw in $mingwDirs) {
                $candidate = Join-Path $mingw.FullName "bin\windeployqt.exe"
                if (Test-Path $candidate) {
                    return $candidate
                }
            }
        }
    }

    throw "windeployqt.exe was not found. Install Qt and pass -QtBinDir <qt_bin_path>."
}

function Resolve-NinjaExe {
    $fromPath = Get-Command ninja -ErrorAction SilentlyContinue
    if ($fromPath) {
        return $fromPath.Source
    }

    $fallback = "C:\Program Files\JetBrains\CLion 2026.1\bin\ninja\win\x64\ninja.exe"
    if (Test-Path $fallback) {
        return $fallback
    }

    return $null
}

function Resolve-MingwCompilers {
    $gccFromPath = Get-Command gcc -ErrorAction SilentlyContinue
    $gxxFromPath = Get-Command g++ -ErrorAction SilentlyContinue
    if ($gccFromPath -and $gxxFromPath) {
        return @($gccFromPath.Source, $gxxFromPath.Source)
    }

    $fallbackGcc = "C:\Program Files\JetBrains\CLion 2026.1\bin\mingw\bin\gcc.exe"
    $fallbackGxx = "C:\Program Files\JetBrains\CLion 2026.1\bin\mingw\bin\g++.exe"
    if ((Test-Path $fallbackGcc) -and (Test-Path $fallbackGxx)) {
        return @($fallbackGcc, $fallbackGxx)
    }

    return @($null, $null)
}

function Get-WindeployqtModeArg {
    param([string]$Mode)

    switch ($Mode.ToLowerInvariant()) {
        "release" { return "--release" }
        "debug" { return "--debug" }
        default { return $null }
    }
}

function Invoke-Step {
    param(
        [string]$Command,
        [string[]]$Arguments
    )

    $quoted = @($Command) + ($Arguments | ForEach-Object { if ($_ -match "\s") { '"' + $_ + '"' } else { $_ } })
    Write-Output ("> " + ($quoted -join " "))

    if ($DryRun) {
        return
    }

    & $Command @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE"
    }
}

function Reset-BuildDirIfGeneratorChanged {
    param(
        [string]$BuildDirPath,
        [string]$RequestedGenerator,
        [switch]$WhatIfOnly
    )

    if (-not $RequestedGenerator) {
        return
    }

    $cacheFile = Join-Path $BuildDirPath "CMakeCache.txt"
    if (-not (Test-Path $cacheFile)) {
        return
    }

    $line = Select-String -Path $cacheFile -Pattern "^CMAKE_GENERATOR:INTERNAL=" -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $line) {
        return
    }

    $currentGenerator = ($line.Line -replace "^CMAKE_GENERATOR:INTERNAL=", "").Trim()
    if ($currentGenerator -eq $RequestedGenerator) {
        return
    }

    Write-Output "Generator change detected: '$currentGenerator' -> '$RequestedGenerator'."
    if ($WhatIfOnly) {
        Write-Output "Dry-run: would remove stale CMake cache in '$BuildDirPath'."
        return
    }

    $cache = Join-Path $BuildDirPath "CMakeCache.txt"
    $cmakeFilesDir = Join-Path $BuildDirPath "CMakeFiles"
    if (Test-Path $cache) {
        Remove-Item -Path $cache -Force
    }
    if (Test-Path $cmakeFilesDir) {
        Remove-Item -Path $cmakeFilesDir -Recurse -Force
    }
}

$resolvedBuildType = $BuildType.ToLowerInvariant()
if ($resolvedBuildType -notin @("release", "debug", "relwithdebinfo", "minsizerel")) {
    throw "Unsupported BuildType '$BuildType'."
}

if (-not $BuildDir) {
    $BuildDir = Join-Path $SourceDir ("cmake-build-" + $resolvedBuildType)
}
if (-not $DistDir) {
    $DistDir = Join-Path $SourceDir (Join-Path "dist" "windows")
}

$CMakeExe = Resolve-CMakeExe -UserProvided $CMakeExe
$windeployqtExe = Resolve-WindeployqtExe -UserQtBinDir $QtBinDir

if (-not $Generator) {
    $ninjaExe = Resolve-NinjaExe
    if ($ninjaExe) {
        $Generator = "Ninja"
        if (-not $MakeProgram) {
            $MakeProgram = $ninjaExe
        }
    }
}

if (($Generator -eq "Ninja") -and ((-not $CCompiler) -or (-not $CxxCompiler))) {
    $resolvedCompilers = Resolve-MingwCompilers
    if (-not $CCompiler) {
        $CCompiler = $resolvedCompilers[0]
    }
    if (-not $CxxCompiler) {
        $CxxCompiler = $resolvedCompilers[1]
    }
}

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null

if (-not $SkipConfigure) {
    Reset-BuildDirIfGeneratorChanged -BuildDirPath $BuildDir -RequestedGenerator $Generator -WhatIfOnly:$DryRun

    $configArgs = @(
        "-S", $SourceDir,
        "-B", $BuildDir,
        "-DCMAKE_BUILD_TYPE=$BuildType"
    )
    if ($Generator) {
        $configArgs += @("-G", $Generator)
    }
    if ($MakeProgram) {
        $configArgs += "-DCMAKE_MAKE_PROGRAM=$MakeProgram"
    }
    if ($CCompiler) {
        $configArgs += "-DCMAKE_C_COMPILER=$CCompiler"
    }
    if ($CxxCompiler) {
        $configArgs += "-DCMAKE_CXX_COMPILER=$CxxCompiler"
    }

    Invoke-Step -Command $CMakeExe -Arguments $configArgs
}

if (-not $SkipBuild) {
    Invoke-Step -Command $CMakeExe -Arguments @(
        "--build", $BuildDir,
        "--target", $AppName
    )
}

if ($DryRun) {
    $dryModeArg = Get-WindeployqtModeArg -Mode $DeployMode
    $dryDeployArgs = @()
    if ($dryModeArg) {
        $dryDeployArgs += $dryModeArg
    }
    $dryDeployArgs += @(
        "--no-translations",
        (Join-Path (Join-Path $DistDir $AppName) "$AppName.exe")
    )

    Invoke-Step -Command $windeployqtExe -Arguments $dryDeployArgs
    Write-Output ""
    Write-Output "Dry-run completed. No files were changed."
    return
}

$builtExe = Join-Path $BuildDir ("$AppName.exe")
if (-not (Test-Path $builtExe)) {
    throw "Executable not found at '$builtExe'."
}

$distAppDir = Join-Path $DistDir $AppName
if (Test-Path $distAppDir) {
    Remove-Item -Path $distAppDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $distAppDir | Out-Null

Copy-Item -Path $builtExe -Destination (Join-Path $distAppDir "$AppName.exe") -Force

$modeArg = Get-WindeployqtModeArg -Mode $DeployMode
$deployArgs = @()
if ($modeArg) {
    $deployArgs += $modeArg
}
$deployArgs += @(
    "--no-translations",
    (Join-Path $distAppDir "$AppName.exe")
)

Invoke-Step -Command $windeployqtExe -Arguments $deployArgs

Write-Output ""
Write-Output "Deployment completed."
Write-Output "App folder: $distAppDir"
