@echo off
echo === Creating Portable Version ===

if not exist deploy\HT_view.exe (
    echo Error: Please run build_all.bat first!
    pause
    exit /b 1
)

rem Создать портативную папку
if exist HT_View_Portable rmdir /s /q HT_View_Portable
mkdir HT_View_Portable

rem Копировать все файлы
echo Copying files...
xcopy deploy\*.* HT_View_Portable\ /s /y /q

rem Создать README
echo HT View - Portable Version > HT_View_Portable\README_Portable.txt
echo ========================== >> HT_View_Portable\README_Portable.txt
echo. >> HT_View_Portable\README_Portable.txt
echo This is a portable version that doesn't require installation. >> HT_View_Portable\README_Portable.txt
echo Just run HT_view.exe to start the application. >> HT_View_Portable\README_Portable.txt
echo. >> HT_View_Portable\README_Portable.txt
echo Files included: >> HT_View_Portable\README_Portable.txt
echo - HT_view.exe     : Main application >> HT_View_Portable\README_Portable.txt
echo - README.md       : Complete user guide >> HT_View_Portable\README_Portable.txt
echo - config.ini      : Configuration file >> HT_View_Portable\README_Portable.txt
echo - Qt6*.dll        : Required libraries >> HT_View_Portable\README_Portable.txt
echo - platforms/      : Platform plugins >> HT_View_Portable\README_Portable.txt
echo - imageformats/   : Image format support >> HT_View_Portable\README_Portable.txt
echo. >> HT_View_Portable\README_Portable.txt
echo For complete documentation, open README.md in any text editor >> HT_View_Portable\README_Portable.txt
echo or web browser. >> HT_View_Portable\README_Portable.txt
echo. >> HT_View_Portable\README_Portable.txt
echo Quick Start: >> HT_View_Portable\README_Portable.txt
echo 1. Run HT_view.exe >> HT_View_Portable\README_Portable.txt
echo 2. Click Open to load a CSV file >> HT_View_Portable\README_Portable.txt
echo 3. Use D/W/M keys for Day/Week/Month views >> HT_View_Portable\README_Portable.txt
echo 4. Mouse wheel to zoom, drag to pan >> HT_View_Portable\README_Portable.txt
echo. >> HT_View_Portable\README_Portable.txt
echo System Requirements: Windows 10/11 64-bit >> HT_View_Portable\README_Portable.txt
echo Version: 1.0 >> HT_View_Portable\README_Portable.txt
for /f %%i in ('dir /s /b HT_View_Portable\*.* ^| find /c /v ""') do echo Total files: %%i >> HT_View_Portable\README_Portable.txt

rem Создать ZIP архив (если есть PowerShell)
echo Creating ZIP archive...
powershell -command "try { Compress-Archive -Path 'HT_View_Portable\*' -DestinationPath 'HT_View_Portable.zip' -Force; Write-Host 'ZIP created successfully' } catch { Write-Host 'ZIP creation failed' }" 2>nul

echo.
echo === SUCCESS ===
echo Portable version created:
echo - Folder: HT_View_Portable\
if exist HT_View_Portable.zip (
    echo - Archive: HT_View_Portable.zip
    for %%F in (HT_View_Portable.zip) do (
        set /a size_mb=%%~zF/1024/1024
        echo   Size: %%~zF bytes (~!size_mb! MB^)
    )
) else (
    echo - ZIP creation failed or PowerShell not available
    echo   You can manually create archive from HT_View_Portable folder
)

echo.
echo Ready to distribute!

pause