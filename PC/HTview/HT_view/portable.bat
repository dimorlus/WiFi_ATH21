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
xcopy deploy\*.* HT_View_Portable\ /s /y

rem Создать README
echo HT View - Portable Version > HT_View_Portable\README.txt
echo. >> HT_View_Portable\README.txt
echo This is a portable version that doesn't require installation. >> HT_View_Portable\README.txt
echo Just run HT_view.exe to start the application. >> HT_View_Portable\README.txt
echo. >> HT_View_Portable\README.txt
echo Features: >> HT_View_Portable\README.txt
echo - Load CSV files with temperature/humidity data >> HT_View_Portable\README.txt
echo - Interactive charts with zoom and pan >> HT_View_Portable\README.txt
echo - Export to PNG/PDF >> HT_View_Portable\README.txt
echo - Print support >> HT_View_Portable\README.txt
echo - Keyboard shortcuts (D/W/M for time scales) >> HT_View_Portable\README.txt

rem Создать ZIP архив (если есть PowerShell)
powershell -command "Compress-Archive -Path 'HT_View_Portable\*' -DestinationPath 'HT_View_Portable.zip' -Force" 2>nul

if exist HT_View_Portable.zip (
    echo.
    echo === SUCCESS ===
    echo Portable version created:
    echo - Folder: HT_View_Portable\
    echo - Archive: HT_View_Portable.zip
    for %%F in (HT_View_Portable.zip) do echo   Size: %%~zF bytes
) else (
    echo.
    echo Portable folder created: HT_View_Portable\
    echo Note: Install 7-Zip or WinRAR to create archives
)

pause