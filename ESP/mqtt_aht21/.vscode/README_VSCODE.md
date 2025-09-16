# ESP8266 MQTT Temperature Monitor

## Конфигурация VSCode для ESP8266

Этот проект настроен для работы в VSCode с поддержкой:
- Компиляции через mingw32-make
- IntelliSense для ESP8266 NON-OS SDK
- Отладки через последовательный порт
- Автоматических задач сборки и прошивки

## Команды сборки

Используйте Ctrl+Shift+P и выберите "Tasks: Run Task", затем:

### Основные команды
- **ESP8266: Build All** - полная сборка проекта
- **ESP8266: Clean** - очистка объектных файлов
- **ESP8266: Rebuild** - очистка + сборка
- **ESP8266: Flash** - прошивка устройства

### OTA команды
- **ESP8266: Build User1 (OTA)** - сборка user1.bin для OTA
- **ESP8266: Build User2 (OTA)** - сборка user2.bin для OTA
- **ESP8266: Flash User1** - прошивка user1.bin
- **ESP8266: Flash User2** - прошивка user2.bin

### Дополнительные команды
- **ESP8266: Flash Init** - инициализация флеш памяти
- **ESP8266: Flash Boot** - прошивка загрузчика
- **ESP8266: Flash SSL Certificates** - загрузка SSL сертификатов
- **ESP8266: Build Release (bld.bat)** - сборка релизных версий

### Отладка
- **Serial Monitor** - открытие серийного монитора (PuTTY)

## Настройка портов

По умолчанию используется COM3. Для изменения порта отредактируйте:
- `settings.mk` - ESPPORT переменная
- `.vscode/tasks.json` - Serial Monitor задача

## Горячие клавиши

- **Ctrl+Shift+B** - Быстрая сборка (Build All)
- **Ctrl+Shift+P** - Палитра команд для запуска задач

## IntelliSense

Настроен автокомплит для:
- ESP8266 NON-OS SDK функции
- Локальные заголовочные файлы
- MQTT библиотека
- Пользовательские модули
