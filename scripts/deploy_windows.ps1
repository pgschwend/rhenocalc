# Execute with following command:
# powershell -ExecutionPolicy Bypass -File D:\32_QT\rhenocalc\scripts\deploy_windows.ps1 -VerboseProbe

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
    [switch]$VerboseProbe,
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

function Resolve-QtMingwCompilers {
    param([string]$WindeployqtExe)

    if (-not $WindeployqtExe) {
        return @($null, $null)
    }

    $qtBinDir = Split-Path -Path $WindeployqtExe -Parent
    $qtKitDir = Split-Path -Path $qtBinDir -Parent

    # Some kits include gcc/g++ directly in the kit bin folder.
    $kitGcc = Join-Path $qtBinDir "gcc.exe"
    $kitGxx = Join-Path $qtBinDir "g++.exe"
    if ((Test-Path $kitGcc) -and (Test-Path $kitGxx)) {
        return @($kitGcc, $kitGxx)
    }

    # Typical Qt layout: C:\Qt\<ver>\mingw_64\bin\windeployqt.exe
    # Matching compilers are under C:\Qt\Tools\mingw*_64\bin.
    $qtVersionDir = Split-Path -Path $qtKitDir -Parent
    $qtRoot = Split-Path -Path $qtVersionDir -Parent
    $toolsDir = Join-Path $qtRoot "Tools"
    if (Test-Path $toolsDir) {
        $mingwDirs = Get-ChildItem -Path $toolsDir -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -like "mingw*_64" } |
            Sort-Object Name -Descending

        foreach ($mingwDir in $mingwDirs) {
            $gcc = Join-Path $mingwDir.FullName "bin\gcc.exe"
            $gxx = Join-Path $mingwDir.FullName "bin\g++.exe"
            if ((Test-Path $gcc) -and (Test-Path $gxx)) {
                return @($gcc, $gxx)
            }
        }
    }

    return @($null, $null)
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
    $fallbackGcc = "C:\Program Files\JetBrains\CLion 2026.1\bin\mingw\bin\gcc.exe"
    $fallbackGxx = "C:\Program Files\JetBrains\CLion 2026.1\bin\mingw\bin\g++.exe"
    if ((Test-Path $fallbackGcc) -and (Test-Path $fallbackGxx)) {
        return @($fallbackGcc, $fallbackGxx)
    }

    $gccFromPath = Get-Command gcc -ErrorAction SilentlyContinue
    $gxxFromPath = Get-Command g++ -ErrorAction SilentlyContinue
    if ($gccFromPath -and $gxxFromPath) {
        return @($gccFromPath.Source, $gxxFromPath.Source)
    }

    return @($null, $null)
}

function Resolve-PathMingwCompilers {
    $gccFromPath = Get-Command gcc -ErrorAction SilentlyContinue
    $gxxFromPath = Get-Command g++ -ErrorAction SilentlyContinue
    if ($gccFromPath -and $gxxFromPath) {
        return @($gccFromPath.Source, $gxxFromPath.Source)
    }

    return @($null, $null)
}

function Get-QtExpectedMachine {
    param([string]$WindeployqtExe)

    if (-not $WindeployqtExe) {
        return $null
    }

    $qtBinDir = Split-Path -Path $WindeployqtExe -Parent
    $qtKitDir = Split-Path -Path $qtBinDir -Parent
    $kitName = Split-Path -Path $qtKitDir -Leaf

    if ($kitName -like "mingw*_64") {
        return "x86_64-w64-mingw32"
    }
    if ($kitName -like "mingw*_32") {
        return "mingw32"
    }

    return $null
}

function Get-CompilerMachine {
    param([string]$CompilerPath)

    if (-not $CompilerPath -or -not (Test-Path $CompilerPath)) {
        return $null
    }

    try {
        $machine = (& $CompilerPath "-dumpmachine" 2>$null | Select-Object -First 1)
        if ($LASTEXITCODE -ne 0) {
            return $null
        }
        if ($machine) {
            return $machine.Trim()
        }
    }
    catch {
        return $null
    }

    return $null
}

function Test-CompilerQtCompatibility {
    param(
        [string]$CCompilerPath,
        [string]$WindeployqtExe
    )

    $expectedMachine = Get-QtExpectedMachine -WindeployqtExe $WindeployqtExe
    if (-not $expectedMachine) {
        return $true
    }

    $actualMachine = Get-CompilerMachine -CompilerPath $CCompilerPath
    if (-not $actualMachine) {
        return $false
    }

    if ($expectedMachine -eq "x86_64-w64-mingw32") {
        return $actualMachine -like "x86_64*"
    }
    if ($expectedMachine -eq "mingw32") {
        return ($actualMachine -eq "mingw32") -or ($actualMachine -like "i686*")
    }

    return $actualMachine -eq $expectedMachine
}

function Get-WindeployqtModeArg {
    param([string]$Mode)

    switch ($Mode.ToLowerInvariant()) {
        "release" { return "--release" }
        "debug" { return "--debug" }
        default { return $null }
    }
}

function Test-CompilerPair {
    param(
        [string]$CCompilerPath,
        [string]$CxxCompilerPath
    )

    if ((-not $CCompilerPath) -or (-not $CxxCompilerPath)) {
        return $false
    }

    if ((-not (Test-Path $CCompilerPath)) -or (-not (Test-Path $CxxCompilerPath))) {
        return $false
    }

    $probeDir = Join-Path ([System.IO.Path]::GetTempPath()) ("rhenocalc-compiler-probe-" + [System.Guid]::NewGuid().ToString("N"))
    New-Item -ItemType Directory -Path $probeDir -Force | Out-Null

    $cFile = Join-Path $probeDir "probe.c"
    $cppFile = Join-Path $probeDir "probe.cpp"
    $cObj = Join-Path $probeDir "probe_c.o"
    $cppObj = Join-Path $probeDir "probe_cpp.o"
    $cErr = Join-Path $probeDir "probe_c.err.txt"
    $cppErr = Join-Path $probeDir "probe_cpp.err.txt"

    Set-Content -Path $cFile -Value "int main(void){return 0;}" -Encoding Ascii
    Set-Content -Path $cppFile -Value "int main(){return 0;}" -Encoding Ascii

    try {
        & $CCompilerPath "-c" $cFile "-o" $cObj 1>$null 2> $cErr
        if (($LASTEXITCODE -ne 0) -or (-not (Test-Path $cObj))) {
            if ($VerboseProbe) {
                Write-Output "[probe] C compiler failed: $CCompilerPath"
                if (Test-Path $cErr) {
                    $err = Get-Content -Path $cErr -Raw -ErrorAction SilentlyContinue
                    if ($err) { Write-Output "[probe] C stderr:`n$err" }
                }
            }
            return $false
        }

        & $CxxCompilerPath "-c" $cppFile "-o" $cppObj 1>$null 2> $cppErr
        if (($LASTEXITCODE -ne 0) -or (-not (Test-Path $cppObj))) {
            if ($VerboseProbe) {
                Write-Output "[probe] CXX compiler failed: $CxxCompilerPath"
                if (Test-Path $cppErr) {
                    $err = Get-Content -Path $cppErr -Raw -ErrorAction SilentlyContinue
                    if ($err) { Write-Output "[probe] CXX stderr:`n$err" }
                }
            }
            return $false
        }

        return $true
    }
    finally {
        if (Test-Path $probeDir) {
            Remove-Item -Path $probeDir -Recurse -Force -ErrorAction SilentlyContinue
        }
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

function Add-DirsToPath {
    param([string[]]$Dirs)

    foreach ($dir in $Dirs) {
        if (-not $dir) { continue }
        if (-not (Test-Path $dir)) { continue }

        $parts = $env:Path -split ';'
        $exists = $false
        foreach ($part in $parts) {
            if ($part -and ($part.TrimEnd('\\') -ieq $dir.TrimEnd('\\'))) {
                $exists = $true
                break
            }
        }

        if (-not $exists) {
            $env:Path = "$dir;$($env:Path)"
            Write-Output "Added to PATH for this run: $dir"
        }
    }
}

function Reset-BuildDirIfGeneratorChanged {
    param(
        [string]$BuildDirPath,
        [string]$RequestedGenerator,
        [string]$RequestedCCompiler,
        [string]$RequestedCxxCompiler,
        [switch]$WhatIfOnly
    )

    $cacheFile = Join-Path $BuildDirPath "CMakeCache.txt"
    if (-not (Test-Path $cacheFile)) {
        return
    }

    $mustReset = $false

    if ($RequestedGenerator) {
        $line = Select-String -Path $cacheFile -Pattern "^CMAKE_GENERATOR:INTERNAL=" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($line) {
            $currentGenerator = ($line.Line -replace "^CMAKE_GENERATOR:INTERNAL=", "").Trim()
            if ($currentGenerator -ne $RequestedGenerator) {
                Write-Output "Generator change detected: '$currentGenerator' -> '$RequestedGenerator'."
                $mustReset = $true
            }
        }
    }

    if ($RequestedCCompiler) {
        $line = Select-String -Path $cacheFile -Pattern "^CMAKE_C_COMPILER:.*=" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($line) {
            $currentCCompiler = ($line.Line -replace "^CMAKE_C_COMPILER:.*=", "").Trim()
            if ($currentCCompiler -and ($currentCCompiler -ine $RequestedCCompiler)) {
                Write-Output "C compiler change detected: '$currentCCompiler' -> '$RequestedCCompiler'."
                $mustReset = $true
            }
        }
    }

    if ($RequestedCxxCompiler) {
        $line = Select-String -Path $cacheFile -Pattern "^CMAKE_CXX_COMPILER:.*=" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($line) {
            $currentCxxCompiler = ($line.Line -replace "^CMAKE_CXX_COMPILER:.*=", "").Trim()
            if ($currentCxxCompiler -and ($currentCxxCompiler -ine $RequestedCxxCompiler)) {
                Write-Output "CXX compiler change detected: '$currentCxxCompiler' -> '$RequestedCxxCompiler'."
                $mustReset = $true
            }
        }
    }

    if (-not $mustReset) {
        return
    }

    if ($WhatIfOnly) {
        Write-Output "Dry-run: would remove stale CMake cache/toolchain files in '$BuildDirPath'."
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

# Make sure Qt tool binaries are discoverable for child processes.
Add-DirsToPath -Dirs @((Split-Path -Path $windeployqtExe -Parent))

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
    $candidatePairs = @()

    $qtCompilers = Resolve-QtMingwCompilers -WindeployqtExe $windeployqtExe
    if ($qtCompilers[0] -and $qtCompilers[1]) {
        $candidatePairs += ,@($qtCompilers[0], $qtCompilers[1], "Qt MinGW")
    }

    $fallbackCompilers = Resolve-MingwCompilers
    if ($fallbackCompilers[0] -and $fallbackCompilers[1]) {
        $candidatePairs += ,@($fallbackCompilers[0], $fallbackCompilers[1], "Fallback MinGW")
    }

    $pathCompilers = Resolve-PathMingwCompilers
    if ($pathCompilers[0] -and $pathCompilers[1]) {
        $candidatePairs += ,@($pathCompilers[0], $pathCompilers[1], "PATH MinGW")
    }

    $resolved = $false
    foreach ($pair in $candidatePairs) {
        $candidateC = if ($CCompiler) { $CCompiler } else { $pair[0] }
        $candidateCxx = if ($CxxCompiler) { $CxxCompiler } else { $pair[1] }

        if ((-not $candidateC) -or (-not $candidateCxx)) {
            continue
        }

        if (-not (Test-CompilerQtCompatibility -CCompilerPath $candidateC -WindeployqtExe $windeployqtExe)) {
            $expectedMachine = Get-QtExpectedMachine -WindeployqtExe $windeployqtExe
            $actualMachine = Get-CompilerMachine -CompilerPath $candidateC
            Write-Output "Skipping incompatible compiler pair from $($pair[2]): expected '$expectedMachine', got '$actualMachine'."
            continue
        }

        if (Test-CompilerPair -CCompilerPath $candidateC -CxxCompilerPath $candidateCxx) {
            $CCompiler = $candidateC
            $CxxCompiler = $candidateCxx
            Write-Output "Using validated compiler pair from $($pair[2])."
            $resolved = $true
            break
        }

        Write-Output "Compiler probe failed for $($pair[2]): C='$candidateC', CXX='$candidateCxx'."
    }

    if (-not $resolved) {
        throw "No working Qt-compatible C/C++ compiler pair found for Ninja generator. Pass -CCompiler/-CxxCompiler for the same architecture as your Qt kit."
    }
}

if (($Generator -eq "Ninja") -and $CCompiler -and $CxxCompiler) {
    if (-not (Test-CompilerPair -CCompilerPath $CCompiler -CxxCompilerPath $CxxCompiler)) {
        throw "Configured compiler pair failed probe: C='$CCompiler', CXX='$CxxCompiler'."
    }
}

# Ensure selected compiler runtime DLLs are resolvable when CMake/Ninja invokes gcc/cc1.
if ($CCompiler -or $CxxCompiler) {
    Add-DirsToPath -Dirs @(
        $(if ($CCompiler) { Split-Path -Path $CCompiler -Parent } else { $null }),
        $(if ($CxxCompiler) { Split-Path -Path $CxxCompiler -Parent } else { $null })
    )
}

if ($CCompiler) {
    Write-Output "Using C compiler: $CCompiler"
}
if ($CxxCompiler) {
    Write-Output "Using CXX compiler: $CxxCompiler"
}

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null

if (-not $SkipConfigure) {
    Reset-BuildDirIfGeneratorChanged -BuildDirPath $BuildDir -RequestedGenerator $Generator -RequestedCCompiler $CCompiler -RequestedCxxCompiler $CxxCompiler -WhatIfOnly:$DryRun

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
