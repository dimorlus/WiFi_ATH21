# Удалить старые установки (если есть)
C:\vcpkg\vcpkg.exe remove paho-mqtt:x64-windows
C:\vcpkg\vcpkg.exe remove paho-mqttpp3:x64-windows

# Установить для MinGW
C:\vcpkg\vcpkg.exe install paho-mqtt:x64-mingw-dynamic
C:\vcpkg\vcpkg.exe install paho-mqttpp3:x64-mingw-dynamic