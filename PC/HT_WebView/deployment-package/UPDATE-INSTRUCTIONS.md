# 🔄 Инструкции по обновлению HT WebView Server

## 📋 Перед обновлением

### 1. Остановите текущую службу

**Вариант А: Через Windows Services**
- Откройте `services.msc`
- Найдите "HT WebView Service"  
- Нажмите правой кнопкой → "Остановить"

**Вариант Б: Через командную строку (Администратор)**
```batch
net stop "HT WebView Service"
```

**Вариант В: Используйте скрипт**
```batch
stop-service.bat
```

### 2. Проверьте что служба остановлена
```batch
sc query "HT WebView Service"
```
Должно показать: `STATE: 1 STOPPED`

## 🚀 Установка новой версии

### 3. Распакуйте новый пакет
- Распакуйте `HT-WebView-Server.zip` в **ту же папку** где была старая версия
- Подтвердите замену файлов

### 4. Установите/обновите службу
Запустите PowerShell **как Администратор** и выполните:

```powershell
cd "путь\к\папке\с\сервером"
node install-service.js
```

### 5. Запустите службу
```batch
net start "HT WebView Service"
```

### 6. Проверьте работу
- Откройте: http://localhost:3000
- Загрузите CSV файл
- **Проверьте новую функцию**: в заголовке графика должно появиться имя файла!

## ✨ Что нового в версии 1.1.0

- 🏷️ **Отображение имени файла в заголовках графиков**
- 📊 Combined Chart: `📊 Combined Temperature & Humidity Chart - filename.csv`
- 🌡️ Temperature Chart: `🌡️ Temperature Chart - filename.csv`  
- 💧 Humidity Chart: `💧 Humidity Chart - filename.csv`

## 🔧 Устранение проблем

**Если служба не останавливается:**
```batch
taskkill /f /im node.exe
```

**Если порт 3000 занят:**
```batch
netstat -ano | findstr :3000
taskkill /f /pid [PID_NUMBER]
```

**Если служба не устанавливается:**
- Убедитесь что PowerShell запущен как Администратор
- Проверьте что старая служба остановлена и удалена

## 📞 Поддержка

При проблемах проверьте:
1. Логи Windows (Event Viewer → Windows Logs → Application)
2. CSV_DIR указывает на правильную папку
3. Порт 3000 свободен
4. Права администратора предоставлены

---
*HT WebView v1.1.0 - Обновление с отображением имени файлов*
