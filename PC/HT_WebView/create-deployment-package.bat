@echo off
echo Creating deployment package...

REM Create deployment directory
if exist "deployment-package" rmdir /s /q deployment-package
mkdir deployment-package

echo Copying essential files...

REM Copy built application
xcopy "dist" "deployment-package\dist" /e /i /q

REM Copy server and service files
copy "deploy-server.js" "deployment-package\"
copy "install-service.js" "deployment-package\"
copy "uninstall-service.js" "deployment-package\"
copy "package.json" "deployment-package\"

REM Copy documentation
copy "QUICK-START.md" "deployment-package\"
copy "DEPLOYMENT.md" "deployment-package\"
copy "README.md" "deployment-package\"

REM Copy utility scripts
copy "check-install.bat" "deployment-package\"

REM Create CSV directory placeholder
mkdir "deployment-package\csv-data"
echo This directory is for CSV files > "deployment-package\csv-data\README.txt"

echo.
echo Creating ZIP archive...

REM Create ZIP using PowerShell (works on Windows 10+)
powershell -command "Compress-Archive -Path 'deployment-package\*' -DestinationPath 'HT-WebView-Server.zip' -Force"

echo.
echo ✅ Deployment package created: HT-WebView-Server.zip
echo.
echo Contents:
echo   📁 dist/                    - Built web application
echo   📄 deploy-server.js         - Production server
echo   📄 install-service.js       - Service installer
echo   📄 uninstall-service.js     - Service uninstaller  
echo   📄 package.json             - Dependencies
echo   📄 check-install.bat        - Installation checker
echo   📄 *.md                     - Documentation
echo   📁 csv-data/                - CSV files directory
echo.
echo 🚀 Ready for deployment!
echo.
pause
