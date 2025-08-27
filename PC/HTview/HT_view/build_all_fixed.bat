@echo off
setlocal enabledelayedexpansion

echo === HT View Complete Build and Package ===

rem Найти правильную версию Qt
set QT_DIR=
if exist "C:\Qt\6.6.0\msvc2019_64\lib\cmake\Qt6" (
    set QT_DIR=C:\Qt\6.6.0\msvc2019_64
    echo Found Qt 6.6.0 MSVC2019
) else (
    echo === ERROR ===
    echo Qt6 not found!
    pause
    exit /b 1
)

echo Using Qt from: %QT_DIR%

rem Установить переменные окружения
set PATH=%QT_DIR%\bin;C:\Program Files\CMake\bin;%PATH%
set CMAKE_PREFIX_PATH=%QT_DIR%
set Qt6_DIR=%QT_DIR%\lib\cmake\Qt6

rem 1. Очистить и создать папку сборки
echo 1. Preparing build directory...
if exist build rmdir /s /q build
mkdir build
cd build

rem 2. Настроить проект
echo 2. Configuring project...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=%QT_DIR% -DQt6_DIR=%Qt6_DIR%
if errorlevel 1 (
    echo === ERROR ===
    echo CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

rem 3. Собрать проект
echo 3. Building project...
cmake --build . --config Release
if errorlevel 1 (
    echo === ERROR ===
    echo Build failed!
    cd ..
    pause
    exit /b 1
)

cd ..

rem 4. Найти исполняемый файл
echo Found executable: build\Release\HT_view.exe

rem 5. Подготовить папку развертывания
echo 4. Preparing deployment...
if exist deploy rmdir /s /q deploy
mkdir deploy

rem 6. Копировать исполняемый файл
copy "build\Release\HT_view.exe" deploy\

rem 7. Развернуть Qt зависимости
echo 5. Deploying Qt dependencies...
"%QT_DIR%\bin\windeployqt.exe" --release --no-translations deploy\HT_view.exe

rem 7.1. Удалить ненужные плагины
echo 5.1. Cleaning unnecessary files...
rmdir /s /q deploy\generic 2>nul
rmdir /s /q deploy\networkinformation 2>nul
rmdir /s /q deploy\tls 2>nul
del /q deploy\Qt6Network.dll 2>nul
del /q deploy\Qt6Svg.dll 2>nul
del /q deploy\Qt6Pdf.dll 2>nul

echo Cleaned deployment, files remaining:
for /f %%i in ('dir /s /b deploy\*.* ^| find /c /v ""') do echo %%i files

rem 8. Копировать документацию и конфигурацию
echo 5.2. Adding documentation and configuration...
if exist README.md copy README.md deploy\
if exist screenshot.png copy screenshot.png deploy\
if exist config.ini copy config.ini deploy\ && echo Using existing config.ini

rem 9. Создать qt.conf
echo [Paths] > deploy\qt.conf
echo Plugins = . >> deploy\qt.conf

rem 10. Создать установщик
if exist "C:\Program Files (x86)\NSIS\makensis.exe" (
    echo.
    echo 6. Creating installer...
    "C:\Program Files (x86)\NSIS\makensis.exe" installer.nsi
    if not errorlevel 1 (
        echo === INSTALLER CREATED ===
        echo Setup file: HT_View_Setup.exe
        for %%F in (HT_View_Setup.exe) do echo Size: %%~zF bytes
    ) else (
        echo Warning: Installer creation failed
    )
) else (
    echo Note: NSIS not found
)

rem 11. Показать результат
echo.
echo === SUCCESS ===
echo Application deployed to 'deploy' folder
echo Total files deployed:
for /f %%i in ('dir /s /b deploy\*.* ^| find /c /v ""') do echo %%i files

echo.
echo Main files:
dir /b deploy\*.exe deploy\*.dll deploy\*.conf deploy\*.ini deploy\*.md 2>nul

echo.
echo Ready to run: deploy\HT_view.exe

rem 12. Тест запуска
set /p test="Test run the application? (y/n): "
if /i "%test%"=="y" start "" "deploy\HT_view.exe"

echo.
echo Distribution ready!
if exist HT_View_Setup.exe echo - HT_View_Setup.exe (installer)
echo - deploy\ folder (portable)

pause