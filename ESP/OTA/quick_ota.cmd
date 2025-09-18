@echo off
rem Quick ESP8266 OTA Update
rem Usage: quick_ota.cmd <ESP_IP>

if "%1"=="" (
    echo Usage: quick_ota.cmd ^<ESP_IP^>
    echo Example: quick_ota.cmd 10.0.1.166
    pause
    exit /b 1
)

echo Starting quick OTA update for ESP8266: %1
python esp_ota_updater.py %1 --http-port 8000