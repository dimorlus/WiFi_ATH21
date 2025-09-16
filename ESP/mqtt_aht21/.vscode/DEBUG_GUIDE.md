# Отладка ESP8266 через серийный порт

## Необходимые инструменты

### 1. PuTTY (рекомендуется)
- Скачайте с: https://www.putty.org/
- Установите в стандартное место: `C:\Program Files\PuTTY\putty.exe`

### 2. Альтернативы
- **Arduino IDE Serial Monitor**
- **Termite** - простой терминал
- **Tera Term** - продвинутый терминал
- **HTerm** - легкий терминал

## Настройка подключения

### Параметры порта
```
Порт: COM3 (по умолчанию, проверьте в диспетчере устройств)
Скорость: 115200 бод
Биты данных: 8
Стоп биты: 1
Четность: None
Контроль потока: None
```

### Автоматический запуск через VSCode
1. Нажмите `Ctrl+Shift+P`
2. Выберите `Tasks: Run Task`
3. Выберите `Serial Monitor`

## Отладочный вывод в коде

### Включение отладки
Добавьте в начало файла:
```c
#define DEBUG_ON
```

### Использование отладочного вывода
```c
#ifdef DEBUG_ON
os_printf("Temperature: %d.%d°C, Humidity: %d.%d%%\\r\\n", 
    temp_int, temp_dec, hum_int, hum_dec);
#endif
```

### Системная информация
```c
os_printf("\\r\\nSystem Info:\\r\\n");
os_printf("SDK Version: %s\\r\\n", system_get_sdk_version());
os_printf("Chip ID: %08X\\r\\n", system_get_chip_id());
os_printf("Free Heap: %d bytes\\r\\n", system_get_free_heap_size());
os_printf("Boot Version: %d\\r\\n", system_get_boot_version());
os_printf("Boot Mode: %d\\r\\n", system_get_boot_mode());
```

## Полезные команды для отладки

### WiFi состояние
```c
uint8 status = wifi_station_get_connect_status();
os_printf("WiFi Status: %d\\r\\n", status);
// 0: IDLE, 1: CONNECTING, 2: WRONG_PASSWORD, 
// 3: NO_AP_FOUND, 4: CONNECT_FAIL, 5: GOT_IP
```

### MQTT состояние  
```c
os_printf("MQTT Connected: %s\\r\\n", 
    mqttClient.connState == MQTT_CONNECTED ? "YES" : "NO");
```

### Датчик AHT21
```c
if(aht21_read(&temp, &hum)) {
    os_printf("AHT21 OK - T:%d.%d H:%d.%d\\r\\n", 
        temp/10, temp%10, hum/10, hum%10);
} else {
    os_printf("AHT21 ERROR\\r\\n");
}
```

## Мониторинг в реальном времени

### Автоматическое переподключение
PuTTY автоматически переподключается при сбросе ESP8266.

### Логирование в файл
В PuTTY: Session → Logging → "All session output" → укажите файл

### Фильтрация вывода
Используйте grep в PowerShell:
```powershell
Get-Content -Path "log.txt" -Wait | Where-Object { $_ -match "Temperature\\|ERROR\\|WiFi" }
```

## Типичные проблемы и решения

### Нет связи с устройством
1. Проверьте номер COM порта в диспетчере устройств
2. Убедитесь что ESP8266 подключен и включен
3. Проверьте драйвера USB-Serial (CH340, CP2102, FTDI)

### Нечитаемые символы  
1. Проверьте скорость порта (115200)
2. Проверьте настройки кодировки (UTF-8)
3. Может быть конфликт скоростей в коде

### Зависание терминала
1. Закройте и откройте заново
2. Проверьте контроль потока (должен быть выключен)
3. Перезагрузите ESP8266

## Горячие клавиши в VSCode

- `Ctrl+Shift+M` - Запустить Serial Monitor  
- `Ctrl+Shift+B` - Собрать и прошить
- `Ctrl+Shift+F` - Только прошить
