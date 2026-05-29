@echo off
setlocal enabledelayedexpansion

:: Arguments: %1 = download URL, %2 = app directory, %3 = app executable path
set "DOWNLOAD_URL=%~1"
set "APP_DIR=%~2"
set "APP_EXE=%~3"

:: Wait for the application to fully close
timeout /t 2 /nobreak >nul

:: Download the zip
set "ZIP_FILE=%APP_DIR%\update_download.zip"
powershell -NoProfile -Command "& { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri '%DOWNLOAD_URL%' -OutFile '%ZIP_FILE%' }"

if not exist "%ZIP_FILE%" (
    echo ERROR: Download failed.
    pause
    exit /b 1
)

:: Extract to temp folder
set "TEMP_DIR=%APP_DIR%\update_temp"
if exist "%TEMP_DIR%" rmdir /s /q "%TEMP_DIR%"
powershell -NoProfile -Command "Expand-Archive -Path '%ZIP_FILE%' -DestinationPath '%TEMP_DIR%' -Force"

:: Copy new files over old ones
:: GitHub release zips may have a single root folder or be flat
:: Check if there's exactly one subfolder
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

