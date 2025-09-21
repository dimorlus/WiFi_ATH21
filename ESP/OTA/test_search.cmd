@echo off
echo ==========================================
echo         ESP8266 Device Search Test
echo ==========================================
echo.
echo Этот файл демонстрирует различные способы поиска ESP устройств
echo.
echo ПРИМЕРЫ ИСПОЛЬЗОВАНИЯ:
echo.
echo 1. Поиск по IP адресу (классический способ):
echo    python esp_ota_updater.py 10.0.1.166
echo.
echo 2. Поиск по полному имени устройства:
echo    python esp_ota_updater.py HT_3C71BF29A3EC
echo.
echo 3. Поиск по частичному имени (найдет любое устройство содержащее "HT_"):
echo    python esp_ota_updater.py HT_
echo.
echo 4. С кастомным HTTP портом:
echo    python esp_ota_updater.py HT_3C71BF29A3EC --http-port 8000
echo.
echo 5. Через быстрый CMD wrapper:
echo    quick_ota.cmd HT_3C71BF29A3EC
echo.
echo АЛГОРИТМ ПОИСКА:
echo 1. Определение типа адреса (IP или имя)
echo 2. Проверка ARP таблицы по имени
echo 3. Извлечение MAC из имени устройства
echo 4. Поиск по MAC в ARP таблице
echo 5. Сканирование локальной сети
echo.
echo Если у вас есть ESP с именем вида HT_xxxxxxx, попробуйте один из способов выше!
echo.
pause