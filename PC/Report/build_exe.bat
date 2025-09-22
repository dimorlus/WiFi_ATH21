@echo off
echo Building TempHumidityReporter.exe...
echo.

REM Activate virtual environment
call .venv\Scripts\activate.bat

REM Build executable
pyinstaller --onefile --name=TempHumidityReporter --add-data="template.docx;." --hidden-import=docx --hidden-import=pandas --console --clean main.py

echo.
echo Build complete!
echo Check 'dist' folder for TempHumidityReporter.exe
echo.
echo To run: copy template.docx to same folder as exe, then:
echo TempHumidityReporter.exe your_data.csv 8 2025
pause