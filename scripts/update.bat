@echo off
setlocal enabledelayedexpansion
title RhenoCalc Updater

:: Arguments: %1 = ZIP file path, %2 = app directory, %3 = app executable path
set "ZIP_FILE=%~1"
set "APP_DIR=%~2"
set "APP_EXE=%~3"
set "TEMP_DIR=%APP_DIR%\rhenocalc_update_temp"

echo ============================================
echo  RhenoCalc Update Script
echo ============================================
echo.
echo ZIP file:   %ZIP_FILE%
echo Target:     %APP_DIR%
echo Executable: %APP_EXE%
echo Temp dir:   %TEMP_DIR%
echo.

:: Validate ZIP file exists
if not exist "%ZIP_FILE%" (
    echo ERROR: ZIP file not found: %ZIP_FILE%
    pause
    exit
)

:: ── Check if we can write to the app directory ───────────────────────────
set "TEST_FILE=%APP_DIR%\_write_test.tmp"
echo test > "%TEST_FILE%" 2>nul
if exist "%TEST_FILE%" (
    del "%TEST_FILE%" 2>nul
) else (
    echo Requesting administrator rights...
    powershell -NoProfile -Command "Start-Process cmd.exe -ArgumentList '/c \"\"%~f0\" \"%ZIP_FILE%\" \"%APP_DIR%\" \"%APP_EXE%\"\"' -Verb RunAs"
    exit
)

:: Wait for the application to fully close
echo Waiting for RhenoCalc to close...
:wait_loop
tasklist /FI "IMAGENAME eq RhenoCalc.exe" 2>nul | find "RhenoCalc.exe" >nul
if not errorlevel 1 (
    timeout /t 1 /nobreak >nul
    goto wait_loop
)
echo Application closed.
echo.

:: ── Extract ZIP file ─────────────────────────────────────────────────────
echo Extracting update...
if exist "%TEMP_DIR%" rmdir /s /q "%TEMP_DIR%"
powershell -NoProfile -Command "Expand-Archive -Path '%ZIP_FILE%' -DestinationPath '%TEMP_DIR%' -Force"
if errorlevel 1 (
    echo ERROR: Extraction failed!
    pause
    exit
)
echo Extraction complete.
echo.

:: ── Detect if ZIP has single root folder ─────────────────────────────────
set "SOURCE_DIR=%TEMP_DIR%"
set "SUBFOLDER_COUNT=0"
for /d %%D in ("%TEMP_DIR%\*") do (
    set "INNER_DIR=%%D"
    set /a SUBFOLDER_COUNT+=1
)
if %SUBFOLDER_COUNT% equ 1 (
    set "SOURCE_DIR=!INNER_DIR!"
    echo Detected single root folder: !SOURCE_DIR!
)

:: Show what we're copying
echo.
echo Files in update package:
dir /b "%SOURCE_DIR%"
echo.

:: ── Copy files ───────────────────────────────────────────────────────────
echo Installing update...
xcopy "%SOURCE_DIR%\*" "%APP_DIR%\" /s /y /q
if errorlevel 1 (
    echo ERROR: Failed to copy files!
    pause
    exit
)
echo Files copied successfully.
echo.

:: ── Cleanup ──────────────────────────────────────────────────────────────
echo Cleaning up...

:: Delete temp extraction folder
if exist "%TEMP_DIR%" (
    rmdir /s /q "%TEMP_DIR%" 2>nul
    if exist "%TEMP_DIR%" (
        echo WARNING: Could not delete temp folder
    ) else (
        echo Deleted: %TEMP_DIR%
    )
)

:: Delete ZIP file
if exist "%ZIP_FILE%" (
    del /f /q "%ZIP_FILE%" 2>nul
    if exist "%ZIP_FILE%" (
        echo WARNING: Could not delete ZIP file
    ) else (
        echo Deleted: %ZIP_FILE%
    )
)

echo.
echo ============================================
echo  Update complete!
echo ============================================
echo.

:: ── Restart application ──────────────────────────────────────────────────
echo Starting RhenoCalc...

set "TARGET_EXE="
if exist "%APP_EXE%" (
    set "TARGET_EXE=%APP_EXE%"
) else if exist "%APP_DIR%\RhenoCalc.exe" (
    set "TARGET_EXE=%APP_DIR%\RhenoCalc.exe"
)

if defined TARGET_EXE (
    echo Launching: %TARGET_EXE%
    cd /d "%APP_DIR%"
    start "" "%TARGET_EXE%"
) else (
    echo ERROR: Could not find RhenoCalc.exe
    pause
    exit
)

:: ── Exit cleanly ─────────────────────────────────────────────────────────
echo.
echo Updater finished.
timeout /t 2 /nobreak >nul
endlocal
exit
