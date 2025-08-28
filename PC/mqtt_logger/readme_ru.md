# MQTT Logger

Программа для подключения к MQTT брокеру и записи данных от ESP8266 устройств в CSV файлы.

## Установка и настройка

### 1. Установка vcpkg (менеджер пакетов)

```bash
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

### 2. Установка библиотек

```bash
# Установка MQTT библиотек
vcpkg install paho-mqtt:x64-windows
vcpkg install paho-mqttpp3:x64-windows

# Интеграция с системой
vcpkg integrate install
```

### 3. Настройка VS Code

Установите расширения:
- C/C++
- CMake Tools
- CMake

### 4. Компиляция проекта

1. Откройте папку проекта в VS Code
2. Нажмите `Ctrl+Shift+P` и выберите "CMake: Configure"
3. Выберите компилятор MinGW
4. Для сборки нажмите `Ctrl+Shift+P` и "CMake: Build"

Или используйте встроенные задачи VS Code:
- `Ctrl+Shift+P` → "Tasks: Run Task" → "CMake: Build"

### 5. Запуск программы

```bash
cd build
./mqtt_logger.exe
```

## Конфигурация

Программа использует файл конфигурации `config.ini`:

- **Windows**: `%APPDATA%\mqtt_logger\config.ini`
- **Linux**: `~/.config/mqtt_logger/config.ini`

При первом запуске файл создается автоматически с настройками по умолчанию.

## Формат данных

Программа ожидает данные в формате:
```
SNTP: 1755580662, TEMP: 26.4 C, HUM: 56.3 %, DEW: 17.0 C
```

И преобразует их в CSV:
```
1755580662;26.4;56.3
```

## Файлы данных

CSV файлы сохраняются в:
- **Windows**: `%APPDATA%\mqtt_logger\data\`
- **Linux**: `~/.local/share/mqtt_logger/data/`

Имя файла соответствует MAC-адресу устройства (например: `HT_3C71BF29A68E.csv`).

## Топики MQTT

Программа подписывается на топики вида:
```
ORLOV/HUMT/+/HUMT
```

Где `+` - wildcard для MAC-адреса устройства.

## Отладка

Для отладки:
1. Установите точки останова в коде
2. Нажмите `F5` или используйте конфигурацию "Debug MQTT Logger"

## Устранение неполадок

### Ошибки компиляции
- Убедитесь, что vcpkg установлен и проинтегрирован
- Проверьте пути к компилятору в `.vscode/c_cpp_properties.json`
- Перезапустите VS Code после установки библиотек

### Проблемы с MQTT
- Проверьте настройки брокера в `config.ini`
- Убедитесь, что брокер доступен и работает
- Проверьте учетные данные для подключения

### Права доступа к файлам
- На Windows убедитесь, что программа может создавать файлы в `%APPDATA%`
- На Linux проверьте права доступа к домашней папке
