# 🚀 Как правильно запустить VSCode для ESP8266 проекта

## ⚠️ ВАЖНО: Правильный способ открытия

### ✅ Правильно:
1. Откройте командную строку (PowerShell)
2. Перейдите в папку проекта:
   ```
   cd "d:\Projects\LED\Alex\WiFi_AHT21\ESP\mqtt_aht21"
   ```
3. Запустите VSCode из этой папки:
   ```
   code .
   ```

### ❌ Неправильно:
- Открывать папку `d:\Projects\LED\Alex\WiFi_AHT21\ESP`
- Открывать VSCode и потом File → Open Folder

---

## 🔧 Альтернативные способы:

### Способ 1: Через workspace файл
1. Двойной клик на файл `mqtt_aht21.code-workspace`
2. VSCode откроется с правильными настройками

### Способ 2: Через File Explorer
1. Откройте папку `d:\Projects\LED\Alex\WiFi_AHT21\ESP\mqtt_aht21` в проводнике
2. Правый клик → "Open with Code" (если установлено)

### Способ 3: Из VSCode
1. Запустите VSCode
2. File → Open Folder
3. Выберите **именно папку** `mqtt_aht21`, а не `ESP`

---

## 🎯 Проверка что все работает правильно:

### 1. Проверьте статус бар VSCode
В нижней части должно быть:
- `ESP8266 (Current SDK)` - конфигурация IntelliSense 
- `C/C++` - языковой сервер активен

### 2. Проверьте Tasks (Ctrl+Shift+P → Tasks: Run Task)
Должны быть доступны:
- `ESP8266: Build All`
- `ESP8266: Flash`
- `User1: Build`
- `User2: Build` ← **Это ваша быстрая команда!**
- `User1: Flash`
- `User2: Flash` ← **Это ваша быстрая команда!**

### 3. Проверьте IntelliSense
Откройте `user/user_main.c`:
- `#include "ets_sys.h"` - не должно подсвечиваться красным
- Автодополнение должно работать для ESP8266 функций

---

## 🔥 Быстрые команды после правильного открытия:

1. **Ctrl+Shift+P** → Tasks: Run Task → **User2: Build**
2. **Ctrl+Shift+P** → Tasks: Run Task → **User2: Flash**

Или через меню:
- **Terminal → Run Task... → User2: Build**

---

## 🐛 Если все еще не работает:

### Перезагрузите конфигурацию:
1. `Ctrl+Shift+P`
2. "Developer: Reload Window"

### Проверьте что C++ Extension установлен:
1. `Ctrl+Shift+X` (Extensions)
2. Найдите "C/C++" от Microsoft
3. Убедитесь что включен

### Принудительно обновите IntelliSense:
1. `Ctrl+Shift+P`
2. "C/C++: Reset IntelliSense Database"

---

## 📁 Структура должна выглядеть так в VSCode:

```
ESP8266 MQTT AHT21 PROJECT
├── .vscode/
│   ├── c_cpp_properties.json ✅
│   ├── tasks.json ✅
│   └── settings.json ✅
├── build/
├── driver/
├── include/
├── modules/
├── mqtt/
├── user/
└── Makefile ✅
```

**Если вы видите папку `ESP` на верхнем уровне - значит открыли неправильную папку!**
