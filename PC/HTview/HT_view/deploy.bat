@echo off
setlocal enabledelayedexpansion

echo === HT View Deployment Script ===

rem Сборка релиза
echo Building release...
cmake --build build --config Release
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

rem Очистка папки развертывания
if exist deploy rmdir /s /q deploy
mkdir deploy

rem Копирование exe и config
copy build\Release\HT_view.exe deploy\
if exist config.ini copy config.ini deploy\

rem Использование windeployqt для минимального набора DLL
echo Deploying minimal Qt dependencies...
"C:\Qt\6.6.0\msvc2019_64\bin\windeployqt.exe" ^
    --release ^
    --no-translations ^
    --no-system-d3d-compiler ^
    --no-opengl-sw ^
    --no-network ^
    --no-sql ^
    --no-qmltooling ^
    --no-quick-import ^
    --no-multimedia ^
    --no-svg ^
    --no-serialport ^
    deploy\HT_view.exe

echo Removing unnecessary plugins...
rem Удалить ненужные форматы изображений
if exist deploy\imageformats (
    pushd deploy\imageformats
    del /q qicns.dll qpdf.dll qtga.dll qwbmp.dll 2>nul
    popd
)

rem Удалить ненужные каталоги
rmdir /s /q deploy\generic 2>nul
rmdir /s /q deploy\networkinformation 2>nul
rmdir /s /q deploy\tls 2>nul

echo Final deployment size:
for /f %%i in ('dir /s /b deploy\*.dll ^| find /c /v ""') do echo DLL files: %%i
for /f %%i in ('dir /s deploy') do echo Total size: %%i

echo.
echo Deployment complete! Contents:
dir /b deploy\
if exist deploy\platforms echo Platforms: & dir /b deploy\platforms\
if exist deploy\imageformats echo Image formats: & dir /b deploy\imageformats\
if exist deploy\styles echo Styles: & dir /b deploy\styles\

pause