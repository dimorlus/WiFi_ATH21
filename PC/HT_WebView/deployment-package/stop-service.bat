@echo off
echo =========================================
echo    HT WebView Service - STOP SCRIPT
echo =========================================
echo.

echo Stopping HT WebView service...
net stop "HT WebView Server" 2>nul
if %errorlevel% == 0 (
    echo ✅ Service stopped successfully
) else (
    echo ⚠️  Service was not running or error occurred
)

echo.
echo Checking service status...
sc query "HT WebView Server" | find "STATE" 2>nul
if %errorlevel% == 0 (
    echo.
    echo Service status checked. If STOPPED - ready for update.
) else (
    echo ⚠️  Service not found or not installed
)

echo.
echo ℹ️  Next steps:
echo    1. Install new version with: node install-service.js
echo    2. Start service with: net start "HT WebView Server"
echo.
pause
