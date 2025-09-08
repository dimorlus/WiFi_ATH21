@echo off
echo Building HT WebView for production...

REM Build the project
call npm run build

REM Copy deployment files
copy deploy-package.json package.json
copy deploy-server.js server.js

REM Install production dependencies
call npm install --production

echo.
echo Deployment ready!
echo.
echo To install as Windows Service:
echo 1. Edit install-service.js to set correct CSV_DIR path
echo 2. Run: node install-service.js
echo.
echo To run manually:
echo node server.js
echo.
echo Then access: http://localhost:3000
