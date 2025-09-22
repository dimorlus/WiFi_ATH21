@echo off
echo Creating portable package...

REM Create ZIP archive (if 7zip is available)
where 7z >nul 2>&1
if %errorlevel%==0 (
    7z a -tzip TempHumidityReporter_Portable.zip portable_package\*
    echo ✅ Created: TempHumidityReporter_Portable.zip
) else (
    echo ⚠️  7zip not found. Please manually create ZIP from portable_package folder.
    echo.
    echo Contents to include:
    echo - TempHumidityReporter.exe
    echo - template.docx
    echo - run_report.bat
    echo - README.md
    echo - sample_data.csv
)

echo.
pause