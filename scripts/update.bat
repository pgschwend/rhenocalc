@echo off
setlocal enabledelayedexpansion
title RhenoCalc Updater

:: ══════════════════════════════════════════════════════════════════════════
:: SELF-RELOCATION: Copy script to TEMP and run from there
:: This prevents the script from being overwritten while running
:: ══════════════════════════════════════════════════════════════════════════
set "TEMP_SCRIPT=%TEMP%\rhenocalc_updater_%RANDOM%.bat"
set "RUNNING_FROM_TEMP=0"

:: Check if we're already running from TEMP
echo %~f0 | find /i "%TEMP%" >nul
if not errorlevel 1 set "RUNNING_FROM_TEMP=1"

if "%RUNNING_FROM_TEMP%"=="0" (
    echo Relocating updater to temp directory...
    copy /y "%~f0" "%TEMP_SCRIPT%" >nul
    if exist "%TEMP_SCRIPT%" (
        echo Starting relocated updater...
        start "" /wait cmd /c "%TEMP_SCRIPT%" %*
        del /f /q "%TEMP_SCRIPT%" 2>nul
        exit
    ) else (
        echo WARNING: Could not copy to temp, continuing anyway...
    )
)

:: ══════════════════════════════════════════════════════════════════════════
:: MAIN SCRIPT (runs from TEMP)
:: ══════════════════════════════════════════════════════════════════════════

:: Arguments: %1 = ZIP file path, %2 = app directory, %3 = app executable path
set "ZIP_FILE=%~1"
set "APP_DIR=%~2"
set "APP_EXE=%~3"
set "TEMP_DIR=%APP_DIR%\rhenocalc_update_temp"
set "LOG_FILE=%APP_DIR%\update.log"
set "UPDATE_SUCCESS=0"

:: Start logging - overwrite previous log
echo ============================================ > "%LOG_FILE%"
echo  RhenoCalc Update Log >> "%LOG_FILE%"
echo  %date% %time% >> "%LOG_FILE%"
echo ============================================ >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

call :log "Running from TEMP: %RUNNING_FROM_TEMP%"
call :log "Script location: %~f0"
call :log ""

:: Also show on screen
echo ============================================
echo  RhenoCalc Update Script
echo ============================================
echo.
echo Log file: %LOG_FILE%
echo.

call :log "ZIP file:   %ZIP_FILE%"
call :log "Target:     %APP_DIR%"
call :log "Executable: %APP_EXE%"
call :log "Temp dir:   %TEMP_DIR%"
call :log ""

:: Validate ZIP file exists
if not exist "%ZIP_FILE%" (
    call :log "ERROR: ZIP file not found: %ZIP_FILE%"
    goto :cleanup
)
call :log "ZIP file exists: OK"

:: ── Check if we can write to the app directory ───────────────────────────
set "TEST_FILE=%APP_DIR%\_write_test.tmp"
echo test > "%TEST_FILE%" 2>nul
if exist "%TEST_FILE%" (
    del "%TEST_FILE%" 2>nul
    call :log "Write access: OK"
) else (
    call :log "No write access - requesting admin rights..."
    powershell -NoProfile -Command "Start-Process cmd.exe -ArgumentList '/c \"\"%~f0\" \"%ZIP_FILE%\" \"%APP_DIR%\" \"%APP_EXE%\"\"' -Verb RunAs"
    exit
)

:: Wait for the application to fully close
call :log "Waiting for RhenoCalc to close..."
:wait_loop
tasklist /FI "IMAGENAME eq RhenoCalc.exe" 2>nul | find "RhenoCalc.exe" >nul
if not errorlevel 1 (
    timeout /t 1 /nobreak >nul
    goto wait_loop
)
call :log "Application closed: OK"

:: ── Extract ZIP file ─────────────────────────────────────────────────────
call :log ""
call :log "Extracting update..."

if exist "%TEMP_DIR%" (
    call :log "Removing old temp dir..."
    rmdir /s /q "%TEMP_DIR%" 2>>"%LOG_FILE%"
)

powershell -NoProfile -Command "Expand-Archive -Path '%ZIP_FILE%' -DestinationPath '%TEMP_DIR%' -Force" 2>>"%LOG_FILE%"
if errorlevel 1 (
    call :log "ERROR: Extraction failed!"
    goto :cleanup
)
call :log "Extraction complete: OK"

:: ── Detect if ZIP has single root folder ─────────────────────────────────
set "SOURCE_DIR=%TEMP_DIR%"
set "SUBFOLDER_COUNT=0"
for /d %%D in ("%TEMP_DIR%\*") do (
    set "INNER_DIR=%%D"
    set /a SUBFOLDER_COUNT+=1
)
call :log "Subfolder count: %SUBFOLDER_COUNT%"
if %SUBFOLDER_COUNT% equ 1 (
    set "SOURCE_DIR=!INNER_DIR!"
    call :log "Using single root folder: !SOURCE_DIR!"
)

:: Log what we're copying
call :log ""
call :log "Files in update package:"
dir /b "%SOURCE_DIR%" >> "%LOG_FILE%" 2>&1
call :log ""

:: ── Copy files ───────────────────────────────────────────────────────────
call :log "Installing update..."
xcopy "%SOURCE_DIR%\*" "%APP_DIR%\" /s /y >> "%LOG_FILE%" 2>&1
if errorlevel 1 (
    call :log "ERROR: xcopy failed!"
) else (
    call :log "Files copied: OK"
    set "UPDATE_SUCCESS=1"
)

:: ── Cleanup (ALWAYS executed) ────────────────────────────────────────────
:cleanup
call :log ""
call :log "======== CLEANUP START ========"

:: Delete temp extraction folder
call :log "Checking temp folder: %TEMP_DIR%"
if exist "%TEMP_DIR%" (
    call :log "Deleting temp folder..."
    rmdir /s /q "%TEMP_DIR%" 2>>"%LOG_FILE%"
    timeout /t 2 /nobreak >nul

    if exist "%TEMP_DIR%" (
        call :log "Retry delete temp folder..."
        rmdir /s /q "%TEMP_DIR%" 2>>"%LOG_FILE%"
    )

    if exist "%TEMP_DIR%" (
        call :log "WARNING: Could not delete temp folder"
    ) else (
        call :log "Temp folder deleted: OK"
    )
) else (
    call :log "Temp folder not found (nothing to delete)"
)

:: Delete ZIP file
call :log "Checking ZIP file: %ZIP_FILE%"
if exist "%ZIP_FILE%" (
    call :log "Deleting ZIP file..."
    del /f /q "%ZIP_FILE%" 2>>"%LOG_FILE%"
    timeout /t 2 /nobreak >nul

    if exist "%ZIP_FILE%" (
        call :log "Retry delete ZIP..."
        del /f /q "%ZIP_FILE%" 2>>"%LOG_FILE%"
    )

    if exist "%ZIP_FILE%" (
        call :log "WARNING: Could not delete ZIP file"
    ) else (
        call :log "ZIP file deleted: OK"
    )
) else (
    call :log "ZIP file not found (nothing to delete)"
)

call :log "======== CLEANUP END ========"
call :log ""

if "%UPDATE_SUCCESS%"=="1" (
    call :log "Update complete!"
) else (
    call :log "Update finished with errors"
)

:: ── Restart application ──────────────────────────────────────────────────
call :log ""
call :log "Starting RhenoCalc..."

set "TARGET_EXE="
if exist "%APP_EXE%" (
    set "TARGET_EXE=%APP_EXE%"
) else if exist "%APP_DIR%\RhenoCalc.exe" (
    set "TARGET_EXE=%APP_DIR%\RhenoCalc.exe"
)

if not defined TARGET_EXE (
    call :log "ERROR: Could not find RhenoCalc.exe"
    echo ERROR: Could not find RhenoCalc.exe
    pause
    exit
)

call :log "Launching: %TARGET_EXE%"
cd /d "%APP_DIR%"
explorer.exe "%TARGET_EXE%"

call :log ""
call :log "Updater finished at %date% %time%"

echo.
echo Update finished. See: %LOG_FILE%
timeout /t 5 /nobreak >nul
endlocal
exit

:log
echo %~1
echo %~1 >> "%LOG_FILE%"
goto :eof

