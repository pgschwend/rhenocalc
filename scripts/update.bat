@echo off
setlocal enabledelayedexpansion
title RhenoCalc Updater

:: Arguments: %1 = extracted update folder, %2 = app directory, %3 = app executable path
set "UPDATE_DIR=%~1"
set "APP_DIR=%~2"
set "APP_EXE=%~3"

echo ============================================
echo  RhenoCalc Update Script
echo ============================================
echo.
echo Update source: %UPDATE_DIR%
echo Target:        %APP_DIR%
echo Executable:    %APP_EXE%
echo.

:: Validate arguments
if not exist "%UPDATE_DIR%" (
    echo ERROR: Update directory not found: %UPDATE_DIR%
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
    powershell -NoProfile -Command "Start-Process cmd.exe -ArgumentList '/c \"\"%~f0\" \"%UPDATE_DIR%\" \"%APP_DIR%\" \"%APP_EXE%\"\"' -Verb RunAs"
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

:: Show what we're copying
echo Files in update package:
dir /b "%UPDATE_DIR%"
echo.

:: Copy files
echo Installing update...
xcopy "%UPDATE_DIR%\*" "%APP_DIR%\" /s /y /q
if errorlevel 1 (
    echo ERROR: Failed to copy files!
    pause
    exit
)
echo Files copied successfully.
echo.

:: ── Cleanup: Delete downloaded ZIP and extracted folder ──────────────────
echo Cleaning up update files...

:: Delete the extracted update folder
if exist "%UPDATE_DIR%" (
    rmdir /s /q "%UPDATE_DIR%"
    echo Deleted: %UPDATE_DIR%
)

:: Delete the ZIP file (should be in same directory as extracted folder)
set "ZIP_FILE=%APP_DIR%\rhenocalc_update.zip"
if exist "%ZIP_FILE%" (
    del /f /q "%ZIP_FILE%"
    echo Deleted: %ZIP_FILE%
)

echo.
echo ============================================
echo  Update complete!
echo ============================================
echo.

:: Restart the application
echo Starting RhenoCalc...
if exist "%APP_EXE%" (
    start "" "%APP_EXE%"
) else (
    start "" "%APP_DIR%\RhenoCalc.exe"
)

:: Explicitly exit - no ghost processes
echo.
echo Updater finished. Closing in 3 seconds...
timeout /t 3 /nobreak >nul
endlocal
exit
