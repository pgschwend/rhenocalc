param(
    [string]$AppName = "RhenoCalc",
    [string]$SourceDir = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$DistRoot,
    [string]$ManifestPath,
    [string]$MsixVersion,   # optional override, e.g. 0.3.0.0
    [string]$PfxPath,       # optional for local signing
    [string]$PfxPassword,   # optional for local signing
    [switch]$SkipSign
)

$ErrorActionPreference = "Stop"

function Resolve-ToolPath {
    param(
        [string]$ExeName,
        [string[]]$Fallbacks = @()
    )

    $cmd = Get-Command $ExeName -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    foreach ($f in $Fallbacks) {
        if (Test-Path $f) { return $f }
    }

    return $null
}

function Resolve-SdkToolPath {
    param(
        [string]$ExeName
    )

    $fromPath = Get-Command $ExeName -ErrorAction SilentlyContinue
    if ($fromPath) { return $fromPath.Source }

    $kitsRoot = "C:\Program Files (x86)\Windows Kits\10\bin"
    if (-not (Test-Path $kitsRoot)) {
        return $null
    }

    # 1) Try versioned folders first (e.g. 10.0.22621.0\x64\makeappx.exe)
    $versionDirs = Get-ChildItem -Path $kitsRoot -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '^\d+\.\d+\.\d+\.\d+$' } |
        Sort-Object Name -Descending

    foreach ($ver in $versionDirs) {
        $candidate = Join-Path $ver.FullName ("x64\" + $ExeName)
        if (Test-Path $candidate) { return $candidate }
    }

    # 2) Fallback non-versioned path (x64\makeappx.exe)
    $flatCandidate = Join-Path $kitsRoot ("x64\" + $ExeName)
    if (Test-Path $flatCandidate) { return $flatCandidate }

    return $null
}

function Get-CMakeMarketingVersion {
    param([string]$CMakeText)

    $m = [regex]::Match($CMakeText, 'project\([^\)]*VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)', 'IgnoreCase')
    if (-not $m.Success) {
        throw "Could not parse project VERSION from CMakeLists.txt"
    }
    return $m.Groups[1].Value
}

function Get-MsixVersionFromCMake {
    param([string]$CMakeText)

    $marketingVersion = Get-CMakeMarketingVersion -CMakeText $CMakeText  # e.g. 0.3.0
    return "$marketingVersion.0"                                          # e.g. 0.3.0.0
}

# ------------------------------------------------------------------------------
# Defaults
# ------------------------------------------------------------------------------
if (-not $DistRoot -or $DistRoot.Trim() -eq "") {
    $DistRoot = Join-Path $SourceDir "dist\windows"
}
if (-not $ManifestPath -or $ManifestPath.Trim() -eq "") {
    $ManifestPath = Join-Path $SourceDir "packaging\windows\AppxManifest.xml"
}

$AppDir = Join-Path $DistRoot $AppName
$CMakeLists = Join-Path $SourceDir "CMakeLists.txt"

# ------------------------------------------------------------------------------
# Validate inputs/files
# ------------------------------------------------------------------------------
if (-not (Test-Path $AppDir)) {
    throw "App folder not found: $AppDir`nRun deploy_windows.ps1 first."
}
if (-not (Test-Path $ManifestPath)) {
    throw "Manifest not found: $ManifestPath"
}
if (-not (Test-Path $CMakeLists)) {
    throw "CMakeLists.txt not found: $CMakeLists"
}

# ------------------------------------------------------------------------------
# Resolve version: param override > CMake-derived (x.y.z.0)
# ------------------------------------------------------------------------------
if (-not $MsixVersion -or $MsixVersion.Trim() -eq "") {
    $cmakeText = Get-Content -Path $CMakeLists -Raw
    $MsixVersion = Get-MsixVersionFromCMake -CMakeText $cmakeText
}

if ($MsixVersion -notmatch '^\d+\.\d+\.\d+\.0$') {
    throw "Invalid MSIX version '$MsixVersion'. Expected format x.y.z.0 (revision must be 0 for Store submission)."
}

Write-Output "MSIX version: $MsixVersion"

# ------------------------------------------------------------------------------
# Locate tools
# ------------------------------------------------------------------------------
$makeappx = Resolve-SdkToolPath -ExeName "makeappx.exe"
if (-not $makeappx) {
    # extra static fallbacks
    $makeappx = Resolve-ToolPath -ExeName "makeappx.exe" -Fallbacks @(
        "C:\Program Files (x86)\Windows Kits\10\bin\x64\makeappx.exe",
        "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\makeappx.exe",
        "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\makeappx.exe"
    )
}
if (-not $makeappx) {
    throw "makeappx.exe not found. Install Windows 10/11 SDK."
}

$signtool = Resolve-SdkToolPath -ExeName "signtool.exe"
if (-not $signtool) {
    $signtool = Resolve-ToolPath -ExeName "signtool.exe" -Fallbacks @(
        "C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe",
        "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe",
        "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe"
    )
}
if (-not $SkipSign -and -not $signtool) {
    throw "signtool.exe not found. Install Windows 10/11 SDK."
}

Write-Output "makeappx: $makeappx"
if ($signtool) { Write-Output "signtool: $signtool" }

# ------------------------------------------------------------------------------
# Stage files
# ------------------------------------------------------------------------------
$StageDir = Join-Path $DistRoot "_msix_stage"
if (Test-Path $StageDir) {
    Remove-Item -Path $StageDir -Recurse -Force
}
New-Item -ItemType Directory -Path $StageDir | Out-Null

Copy-Item -Path (Join-Path $AppDir "*") -Destination $StageDir -Recurse -Force
Copy-Item -Path $ManifestPath -Destination (Join-Path $StageDir "AppxManifest.xml") -Force

# Optional assets folder next to manifest
$manifestDir = Split-Path -Path $ManifestPath -Parent
$assetsDir = Join-Path $manifestDir "assets"
if (Test-Path $assetsDir) {
    Copy-Item -Path $assetsDir -Destination (Join-Path $StageDir "assets") -Recurse -Force
}

# ------------------------------------------------------------------------------
# Patch manifest version in stage copy
# ------------------------------------------------------------------------------
$stageManifest = Join-Path $StageDir "AppxManifest.xml"
[xml]$xml = Get-Content -Path $stageManifest

if (-not $xml.Package -or -not $xml.Package.Identity) {
    throw "Invalid manifest structure in $stageManifest (Package/Identity missing)."
}

$xml.Package.Identity.Version = $MsixVersion
$xml.Save($stageManifest)

# ------------------------------------------------------------------------------
# Build MSIX
# ------------------------------------------------------------------------------
$safeAppName = $AppName.ToLowerInvariant()
$msixPath = Join-Path $DistRoot "$safeAppName-$MsixVersion.msix"
if (Test-Path $msixPath) {
    Remove-Item -Path $msixPath -Force
}

Write-Output "> `"$makeappx`" pack /d `"$StageDir`" /p `"$msixPath`" /o"
& $makeappx pack /d $StageDir /p $msixPath /o
if ($LASTEXITCODE -ne 0) {
    throw "makeappx failed with exit code $LASTEXITCODE"
}

# ------------------------------------------------------------------------------
# Optional local signing
# ------------------------------------------------------------------------------
if (-not $SkipSign) {
    if (-not $PfxPath -or $PfxPath.Trim() -eq "") {
        throw "Signing requested but -PfxPath is missing. Pass -SkipSign or provide -PfxPath."
    }
    if (-not (Test-Path $PfxPath)) {
        throw "PFX file not found: $PfxPath"
    }

    $signArgs = @(
        "sign",
        "/fd", "SHA256",
        "/f", $PfxPath
    )

    if ($PfxPassword -and $PfxPassword.Trim() -ne "") {
        $signArgs += @("/p", $PfxPassword)
    }

    $signArgs += @(
        "/tr", "http://timestamp.digicert.com",
        "/td", "SHA256",
        $msixPath
    )

    Write-Output "> `"$signtool`" $($signArgs -join ' ')"
    & $signtool @signArgs
    if ($LASTEXITCODE -ne 0) {
        throw "signtool failed with exit code $LASTEXITCODE"
    }
}

Write-Output ""
Write-Output "Done."
Write-Output "App dir:        $AppDir"
Write-Output "Manifest used:  $ManifestPath"
Write-Output "MSIX version:   $MsixVersion"
Write-Output "MSIX package:   $msixPath"
if ($SkipSign) {
    Write-Output "Signing:        skipped"
} else {
    Write-Output "Signing:        done"
}