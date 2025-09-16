# VSCode Setup Complete! 🎉

Ваш ESP8266 проект успешно настроен для работы в VSCode!

## ✅ Что настроено

### 🔧 Сборка и компиляция
- **IntelliSense** для ESP8266 NON-OS SDK
- **Автодополнение** кода с поддержкой всех библиотек
- **Обнаружение ошибок** на лету
- **Подсветка синтаксиса** для C и Makefile

### ⚡ Быстрые команды (Tasks)
| Задача | Горячие клавиши | Описание |
|--------|----------------|----------|
| Build All | `Ctrl+Shift+B` | Полная сборка |
| Flash | `Ctrl+Shift+F` | Прошивка ESP8266 |
| Rebuild | `Ctrl+Shift+R` | Очистка + сборка |
| Clean | `Ctrl+Shift+C` | Очистка |
| Serial Monitor | `Ctrl+Shift+M` | Серийный монитор |
| **Build User1** | `Ctrl+Alt+1` | **Сборка user1.bin (OTA)** |
| **Build User2** | `Ctrl+Alt+2` | **Сборка user2.bin (OTA)** |
| **Flash User1** | `Ctrl+Shift+Alt+1` | **Прошивка user1.bin** |
| **Flash User2** | `Ctrl+Shift+Alt+2` | **Прошивка user2.bin** |
| Build Release | `Ctrl+Alt+B` | Полная релизная сборка |

### 📁 Структура настроек
```
.vscode/
├── c_cpp_properties.json    # Настройки IntelliSense
├── tasks.json              # Задачи сборки и прошивки
├── launch.json             # Конфигурация запуска
├── settings.json           # Настройки рабочей области
├── keybindings.json        # Горячие клавиши
├── snippets.code-snippets  # Сниппеты кода
├── extensions.json         # Рекомендуемые расширения
└── DEBUG_GUIDE.md         # Руководство по отладке
```

## 🚀 Быстрый старт

### 1. Сборка проекта
```
Ctrl+Shift+P → Tasks: Run Task → ESP8266: Build All
```
или просто `Ctrl+Shift+B`

### 2. Прошивка устройства
```
Ctrl+Shift+P → Tasks: Run Task → ESP8266: Flash
```
или просто `Ctrl+Shift+F`

### 3. Отладка через серийный порт
```
Ctrl+Shift+P → Tasks: Run Task → Serial Monitor (PuTTY)
```
или просто `Ctrl+Shift+M`

## 📋 Доступные задачи

### Основные
- ✅ **ESP8266: Build All** - полная сборка
- ✅ **ESP8266: Clean** - очистка
- ✅ **ESP8266: Rebuild** - очистка + сборка  
- ✅ **ESP8266: Flash** - прошивка

### OTA (Over-The-Air updates)
- ✅ **ESP8266: Build User1 (OTA)** - сборка user1.bin
- ✅ **ESP8266: Build User2 (OTA)** - сборка user2.bin  
- ✅ **ESP8266: Flash User1** - прошивка user1.bin
- ✅ **ESP8266: Flash User2** - прошивка user2.bin

### Дополнительные
- ✅ **ESP8266: Flash Init** - инициализация флеш
- ✅ **ESP8266: Flash Boot** - прошивка загрузчика
- ✅ **ESP8266: Flash SSL Certificates** - SSL сертификаты
- ✅ **ESP8266: Build Release (bld.bat)** - релизная сборка

### Отладка
- ✅ **Serial Monitor (PuTTY)** - серийный монитор PuTTY
- ✅ **Serial Monitor (PowerShell)** - встроенный монитор
- ✅ **Check COM Ports** - проверка COM портов

## 🛠 Настройка портов

По умолчанию используется **COM3**. Для изменения:

1. **Для прошивки**: отредактируйте `settings.mk` → `ESPPORT`
2. **Для монитора**: отредактируйте `.vscode/tasks.json` → Serial Monitor задачи

## 📝 Сниппеты кода

Введите префикс и нажмите `Tab`:

- `esp_task` → Шаблон задачи ESP8266
- `esp_timer` → Callback таймера  
- `esp_wifi_event` → Обработчик WiFi событий
- `mqtt_pub` → Публикация MQTT
- `mqtt_sub` → Подписка MQTT
- `esp_debug` → Отладочный вывод
- `esp_gpio` → Настройка GPIO
- `i2c_write` → Запись в I2C

## 🔍 Отладка

Читайте подробное руководство: [DEBUG_GUIDE.md](.vscode/DEBUG_GUIDE.md)

### Быстрая отладка
```c
#define DEBUG_ON

// Используйте в коде:
#ifdef DEBUG_ON
os_printf("Debug: Temperature = %d°C\\r\\n", temperature);
#endif
```

## ⚠️ Важные примечания

1. **Eclipse совместимость**: Все настройки Eclipse сохранены, вы можете продолжать использовать Eclipse параллельно
2. **Toolchain**: Используется тот же xtensa-lx106-elf GCC что и в Eclipse
3. **SDK**: Проект настроен на ESP8266_NONOS_SDK-2.2.1
4. **Makefile**: Используется существующий Makefile без изменений

## 🎯 Что дальше?

Теперь вы можете:
- ✨ Разрабатывать с автодополнением и проверкой ошибок
- ⚡ Быстро собирать проект горячими клавишами
- 🔧 Прошивать ESP8266 из VSCode
- 🐛 Отлаживать через серийный порт
- 📦 Собирать OTA обновления

**Удачной разработки! 🚀**

---
*Настройка выполнена для проекта WiFi_AHT21 ESP8266 MQTT Temperature Monitor*
