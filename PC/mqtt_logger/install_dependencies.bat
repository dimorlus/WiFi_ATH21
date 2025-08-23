@echo off
echo ========================================
echo MQTT Logger - Установка зависимостей
echo ========================================

REM Проверяем наличие vcpkg
if not exist "C:\vcpkg\vcpkg.exe" (
    echo Vcpkg не найден. Устанавливаем...
    
    cd C:\
    if exist "vcpkg" (
        echo Удаляем старую установку vcpkg...
        rmdir /s /q vcpkg
    )
    
    echo Клонируем vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git
    
    cd vcpkg
    echo Собираем vcpkg...
    call bootstrap-vcpkg.bat
    
    echo Интегрируем vcpkg...
    vcpkg integrate install
) else (
    echo Vcpkg найден: C:\vcpkg\vcpkg.exe
)

echo.
echo Устанавливаем библиотеки MQTT для MinGW...
C:\vcpkg\vcpkg.exe install paho-mqtt:x64-mingw-dynamic
C:\vcpkg\vcpkg.exe install paho-mqttpp3:x64-mingw-dynamic

echo.
echo Проверяем установленные пакеты...
C:\vcpkg\vcpkg.exe list

echo.
echo ========================================
echo Установка завершена!
echo ========================================
echo.
echo Теперь вы можете:
echo 1. Открыть папку проекта в VS Code
echo 2. Нажать Ctrl+Shift+P и выбрать "CMake: Configure"
echo 3. Выбрать компилятор MinGW
echo 4. Собрать проект через "CMake: Build"
echo.
pause