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
    copy /y "%~f0" "%TEMP_SCRIPT%" >nul
    if exist "%TEMP_SCRIPT%" (
        start "" /wait cmd /c "%TEMP_SCRIPT%" %*
        del /f /q "%TEMP_SCRIPT%" 2>nul
        exit
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

:: Start logging
echo ============================================ > "%LOG_FILE%"
echo  RhenoCalc Update Log >> "%LOG_FILE%"
echo  %date% %time% >> "%LOG_FILE%"
echo ============================================ >> "%LOG_FILE%"

:: Validate ZIP file exists
if not exist "%ZIP_FILE%" (
    call :log "ERROR: ZIP file not found"
    goto :cleanup
)

:: ── Check if we can write to the app directory ───────────────────────────
set "TEST_FILE=%APP_DIR%\_write_test.tmp"
echo test > "%TEST_FILE%" 2>nul
if exist "%TEST_FILE%" (
    del "%TEST_FILE%" 2>nul
) else (
    call :log "Requesting admin rights..."
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

:: ── Extract ZIP file ─────────────────────────────────────────────────────
call :log "Extracting update..."

if exist "%TEMP_DIR%" (
    rmdir /s /q "%TEMP_DIR%" 2>>"%LOG_FILE%"
)

powershell -NoProfile -Command "Expand-Archive -Path '%ZIP_FILE%' -DestinationPath '%TEMP_DIR%' -Force" 2>>"%LOG_FILE%"
if errorlevel 1 (
    call :log "ERROR: Extraction failed!"
    goto :cleanup
)

:: ── Detect if ZIP has single root folder ─────────────────────────────────
set "SOURCE_DIR=%TEMP_DIR%"
set "SUBFOLDER_COUNT=0"
for /d %%D in ("%TEMP_DIR%\*") do (
    set "INNER_DIR=%%D"
    set /a SUBFOLDER_COUNT+=1
)
if %SUBFOLDER_COUNT% equ 1 (
    set "SOURCE_DIR=!INNER_DIR!"
)

:: ── Copy files ───────────────────────────────────────────────────────────
call :log "Installing update..."
xcopy "%SOURCE_DIR%\*" "%APP_DIR%\" /s /y >> "%LOG_FILE%" 2>&1
if errorlevel 1 (
    call :log "ERROR: Failed to copy files!"
) else (
    set "UPDATE_SUCCESS=1"
)

:: ── Cleanup (ALWAYS executed) ────────────────────────────────────────────
:cleanup

:: Delete temp extraction folder
if exist "%TEMP_DIR%" (
    rmdir /s /q "%TEMP_DIR%" 2>>"%LOG_FILE%"
    timeout /t 1 /nobreak >nul
    if exist "%TEMP_DIR%" rmdir /s /q "%TEMP_DIR%" 2>>"%LOG_FILE%"
)

:: Delete ZIP file
if exist "%ZIP_FILE%" (
    del /f /q "%ZIP_FILE%" 2>>"%LOG_FILE%"
    timeout /t 1 /nobreak >nul
    if exist "%ZIP_FILE%" del /f /q "%ZIP_FILE%" 2>>"%LOG_FILE%"
)

if "%UPDATE_SUCCESS%"=="1" (
    call :log "Update complete!"
) else (
    call :log "Update finished with errors"
)

:: ── Restart application ──────────────────────────────────────────────────
call :log "Starting RhenoCalc..."

set "TARGET_EXE="
if exist "%APP_EXE%" (
    set "TARGET_EXE=%APP_EXE%"
) else if exist "%APP_DIR%\RhenoCalc.exe" (
    set "TARGET_EXE=%APP_DIR%\RhenoCalc.exe"
)

if not defined TARGET_EXE (
    call :log "ERROR: Could not find RhenoCalc.exe"
    pause
    exit
)

cd /d "%APP_DIR%"
explorer.exe "%TARGET_EXE%"

echo.
timeout /t 3 /nobreak >nul
endlocal
exit

:log
echo %~1
echo %~1 >> "%LOG_FILE%"
goto :eof
