@echo off
rem Quick ESP8266 OTA Update
rem Usage: quick_ota.cmd <ESP_IP_or_Name>

if "%1"=="" (
    echo Usage: quick_ota.cmd ^<ESP_IP_or_Name^>
    echo Examples: 
    echo   quick_ota.cmd 10.0.1.166           - by IP address
    echo   quick_ota.cmd HT_3C71BF29A3EC      - by device name
    echo   quick_ota.cmd HT_                  - search device containing "HT_"
    pause
    exit /b 1
)

echo Starting quick OTA update for: %1
python esp_ota_updater.py %1 --http-port 8000