# Исправление ошибок сборки MQTT Logger

## Проблема
CMake не может найти библиотеки paho-mqtt, потому что они установлены не для правильного целевого триплета.

## Решение

### Шаг 1: Переустановить библиотеки для MinGW

```cmd
# Удалить старые установки (если есть)
C:\vcpkg\vcpkg.exe remove paho-mqtt:x64-windows
C:\vcpkg\vcpkg.exe remove paho-mqttpp3:x64-windows

# Установить для MinGW
C:\vcpkg\vcpkg.exe install paho-mqtt:x64-mingw-dynamic
C:\vcpkg\vcpkg.exe install paho-mqttpp3:x64-mingw-dynamic
```

### Шаг 2: Установить переменную окружения VCPKG_ROOT

В PowerShell:
```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
```

В CMD:
```cmd
setx VCPKG_ROOT "C:\vcpkg"
```

Перезапустите VS Code после этого.

### Шаг 3: Проверить установку

```cmd
C:\vcpkg\vcpkg.exe list | findstr paho
```

Должно показать что-то вроде:
```
paho-mqtt:x64-mingw-dynamic
paho-mqttpp3:x64-mingw-dynamic
```

### Шаг 4: Альтернативный CMakeLists.txt

Если проблема не решается, замените содержимое `CMakeLists.txt` на содержимое из `CMakeLists_alternative.txt`.

### Шаг 5: Ручная настройка (крайний случай)

Если ничего не помогает, можно вручную указать пути в VS Code:

1. Нажмите `Ctrl+Shift+P`
2. Выберите "CMake: Edit User-Local CMake Kits"
3. Добавьте:

```json
{
  "name": "MinGW with vcpkg",
  "compilers": {
    "C": "C:\\MinGW\\bin\\gcc.exe",
    "CXX": "C:\\MinGW\\bin\\g++.exe"
  },
  "toolchainFile": "C:\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake",
  "environmentVariables": {
    "VCPKG_TARGET_TRIPLET": "x64-mingw-dynamic"
  }
}
```

### Шаг 6: Проверка путей

Убедитесь, что файлы существуют:
- `C:\vcpkg\installed\x64-mingw-dynamic\include\mqtt\async_client.h`
- `C:\vcpkg\installed\x64-mingw-dynamic\lib\libpaho-mqtt3as.a`
- `C:\vcpkg\installed\x64-mingw-dynamic\lib\libpaho-mqttpp3.a`

### Шаг 7: Полная пересборка

```cmd
# В терминале VS Code
rm -rf build
```

Затем снова `Ctrl+Shift+P` → "CMake: Configure"

## Если всё равно не работает

Попробуйте собрать вручную из командной строки:

```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic
mingw32-make
```

## Проверка установки vcpkg

```cmd
C:\vcpkg\vcpkg.exe integrate show
```

Должно показать, что интеграция активна для пользователя.
