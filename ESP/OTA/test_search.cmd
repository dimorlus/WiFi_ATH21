@echo off
echo Тестирование поиска ESP устройства по имени
echo.
echo Пример использования:
echo   python esp_ota_updater.py HT_3C71BF29A3EC
echo   python esp_ota_updater.py 10.0.1.166
echo.
echo Если у вас есть ESP с именем вида HT_xxxxxxx, попробуйте:
echo   python esp_ota_updater.py HT_
echo (скрипт найдет устройство, содержащее "HT_" в имени)
echo.
pause