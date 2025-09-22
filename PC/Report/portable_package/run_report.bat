@echo off
chcp 65001 >nul 2>&1
title Temperature & Humidity Report Generator
echo.
echo ╔═══════════════════════════════════════════════════╗
echo ║     Temperature & Humidity Report Generator       ║
echo ╚═══════════════════════════════════════════════════╝
echo.

REM Check if template exists
if not exist "template.docx" (
    echo ❌ ERROR: template.docx not found!
    echo    Please make sure template.docx is in the same folder.
    echo.
    pause
    exit /b 1
)

REM List CSV files
echo 📁 CSV files found in current folder:
echo.
for %%f in (*.csv) do (
    echo    %%f
)
echo.

REM Get CSV filename
set /p csvfile="📄 Enter CSV filename (with .csv extension): "
if not exist "%csvfile%" (
    echo ❌ ERROR: File %csvfile% not found!
    pause
    exit /b 1
)

REM Get month
set /p month="📅 Enter month number (1-12): "

REM Get year  
set /p year="📅 Enter year (e.g., 2025): "

REM Get output filename (optional)
echo.
echo 💾 Output filename (press Enter for auto-generated):
set /p output="   "

echo.
echo 🚀 Generating report...
echo ╰─ File: %csvfile%
echo ╰─ Period: %month%/%year%
echo.

REM Run the generator
if "%output%"=="" (
    TempHumidityReporter.exe "%csvfile%" %month% %year%
) else (
    TempHumidityReporter.exe "%csvfile%" %month% %year% --output "%output%"
)

echo.
if %errorlevel%==0 (
    echo ✅ Report generated successfully!
    echo.
    echo 📋 Generated files:
    for %%f in (*.docx) do (
        echo    %%f
    )
) else (
    echo ❌ Error generating report!
)

echo.
pause