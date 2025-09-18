@echo off
rem ESP8266 OTA Updater - Wrapper Script
rem Usage: ota_update.cmd <ESP_IP> [additional_options]
rem Example: ota_update.cmd 10.0.1.166

if "%1"=="" (
    echo.
    echo ESP8266 OTA Updater
    echo.
    echo Usage: ota_update.cmd ^<ESP_IP^> [additional_options]
    echo.
    echo Examples:
    echo   ota_update.cmd 10.0.1.166
    echo   ota_update.cmd 10.0.1.166 --http-port 8000
    echo   ota_update.cmd 10.0.1.166 --manual
    echo.
    echo Available options:
    echo   --esp-port PORT       Telnet port ESP (default: 23)
    echo   --http-port PORT      HTTP port for files (default: 80)
    echo   --firmware-dir DIR    Firmware directory (default: ..\mqtt_aht21\bin)
    echo   --local-ip IP         Local IP (auto-detect if not specified)
    echo   --manual              Manual mode - HTTP server only
    echo.
    pause
    exit /b 1
)

echo.
echo Starting OTA update for ESP8266: %1
echo Firmware directory: ..\mqtt_aht21\bin
echo.

python esp_ota_updater.py %*

echo.
echo Done!
pause