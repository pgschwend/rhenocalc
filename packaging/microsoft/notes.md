
## Notes for Apple Packaging

### 1. Create target directory (if needed) and create fresh build
```bash
powershell -ExecutionPolicy Bypass -File .\deploy_windows.ps1 -CMakeExe "C:\Program Files\JetBrains\CLion 2026.2.0.1\bin\cmake\win\x64\bin\cmake.exe" -QtBinDir "C:\Qt\6.9.3\mingw_64\bin" -VerboseProb
```

### 2. Add package signature and create msix package
```bash
.\package_msix.ps1 -ManifestPath "C:\Users\patri\Projects\RhenoCalc\scripts\AppxManifest.xml" -SkipSign
```


