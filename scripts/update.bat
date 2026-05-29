@echo off
setlocal enabledelayedexpansion

:: Arguments: %1 = download URL, %2 = app directory, %3 = app executable path
set "DOWNLOAD_URL=%~1"
set "APP_DIR=%~2"
set "APP_EXE=%~3"

:: ── Check if we can write to the app directory ───────────────────────────
set "TEST_FILE=%APP_DIR%\_write_test.tmp"
echo test > "%TEST_FILE%" 2>nul
if exist "%TEST_FILE%" (
    del "%TEST_FILE%" 2>nul
) else (
    :: No write access – re-launch with admin rights
    powershell -NoProfile -Command "Start-Process cmd.exe -ArgumentList '/c \"\"%~f0\" \"%DOWNLOAD_URL%\" \"%APP_DIR%\" \"%APP_EXE%\"\"' -Verb RunAs"
    exit /b 0
)

:: Wait for the application to fully close
timeout /t 2 /nobreak >nul

:: Download to user TEMP (always writable)
set "ZIP_FILE=%TEMP%\rhenocalc_update.zip"
set "TEMP_DIR=%TEMP%\rhenocalc_update_temp"

powershell -NoProfile -Command "& { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri '%DOWNLOAD_URL%' -OutFile '%ZIP_FILE%' }"

if not exist "%ZIP_FILE%" (
    echo ERROR: Download failed.
    pause
    exit /b 1
)

:: Extract to temp folder
if exist "%TEMP_DIR%" rmdir /s /q "%TEMP_DIR%"
powershell -NoProfile -Command "Expand-Archive -Path '%ZIP_FILE%' -DestinationPath '%TEMP_DIR%' -Force"

:: GitHub release zips may have a single root folder or be flat
set "INNER="
for /d %%D in ("%TEMP_DIR%\*") do (
    set "INNER=%%D"
)

if defined INNER (
    xcopy "!INNER!\*" "%APP_DIR%\" /s /y /q >nul 2>&1
) else (
    xcopy "%TEMP_DIR%\*" "%APP_DIR%\" /s /y /q >nul 2>&1
)

:: Cleanup
del "%ZIP_FILE%" 2>nul
rmdir /s /q "%TEMP_DIR%" 2>nul

:: Restart the application
start "" "%APP_EXE%"
exit /b 0
