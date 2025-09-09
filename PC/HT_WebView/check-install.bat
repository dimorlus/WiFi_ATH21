@echo off
echo Checking HT WebView installation...
echo.

REM Check if Node.js is installed
node --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ Node.js is not installed or not in PATH
    echo Please install Node.js from https://nodejs.org/
    pause
    exit /b 1
) else (
    echo ✅ Node.js is installed: 
    node --version
)

REM Check if build directory exists
if exist "dist\" (
    echo ✅ Build directory exists
) else (
    echo ❌ Build directory not found. Run 'npm run build' first
    pause
    exit /b 1
)

REM Check if server file exists
if exist "deploy-server.js" (
    echo ✅ Server file exists
) else (
    echo ❌ deploy-server.js not found
    pause
    exit /b 1
)

REM Check if install script exists
if exist "install-service.js" (
    echo ✅ Service installer exists
) else (
    echo ❌ install-service.js not found
    pause
    exit /b 1
)

REM Check CSV directory
set CSV_DIR=C:\Data\CSV
if exist "%CSV_DIR%" (
    echo ✅ CSV directory exists: %CSV_DIR%
) else (
    echo ⚠️  CSV directory not found: %CSV_DIR%
    echo You may need to create it or edit CSV_DIR in install-service.js
)

echo.
echo 🎯 Installation check complete!
echo.
echo Next steps:
echo 1. Edit CSV_DIR path in install-service.js if needed
echo 2. Run: node install-service.js
echo 3. Open browser: http://localhost:3000
echo.
pause
