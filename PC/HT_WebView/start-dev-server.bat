@echo off
echo Starting HT WebView Server in development mode...
echo Using CSV directory: c:\mqtt_logger\LOG
echo.

set CSV_DIR=c:\mqtt_logger\LOG
set PORT=3000

node server.js

pause
