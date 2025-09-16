# HUMT ESP8266 Device (MQTT Temperature & Humidity Node)

Русская версия | [English version](./README.en.md)

Это прошивка для узла на ESP8266 (NON-OS SDK 2.2.1) с датчиком AHT21B, публикацией температуры/влажности по MQTT и возможностями локальной конфигурации через Web и Telnet (порт 80 и 23). Основа — форк/адаптация библиотеки `esp_mqtt` (Tuan PM) + прикладная логика энергосбережения и настройки.

## Краткий обзор

Устройство после старта:

1. Загружает (или инициализирует) конфигурацию из flash.
2. Подключается к Wi‑Fi (STA) и синхронизирует время через SNTP.
3. Подключается к MQTT брокеру и публикует первичные статусы (alive/info, данные датчика).
4. Ожидает подтверждение публикации и уходит в сон (deep или light) согласно параметрам сборки (`DEEP_SLEEP_MINUTES`, `LIGHT_SLEEP_SECONDS`).

## Основные возможности

* MQTT клиент: QoS1 публикации, подписки на сервисные топики, LWT.
* Температура/Влажность/Точка росы с датчика AHT21B (битбанговый I2C).
* OTA обновление по HTTP (GET бинарника `user1/user2.bin`) через MQTT команду (`/FOTA`).
* Web-интерфейс конфигурации (изменение Wi‑Fi / MQTT / TZ / TLS уровня; заливка сертификатов и ключа).
* Telnet setup (порт 23) — интерпретация простых команд (через `parse()`), удобна для скриптовой настройки.
* SSL/TLS уровни (0..3) с загрузкой CA и client key/cert в flash.
* Два режима сна: deep sleep (минимальное потребление, требует перемычку GPIO16→RST) и light sleep (RAM retention + рестарт после wake).
* Watchdog логика таймеров и таймаут переподключения (перезапуск при длительной потере соединения).
* Динамическая остановка HTTP/Telnet сервисов перед сном (опционально `SLEEP_STOP_SERVICES`).

## MQTT Топики

Формат базового префикса: `MQTT_TOPIC_BASE/MQTT_TOPIC_TYPE/DEV` и/или `MQTT_TOPIC_BASE/<MAC>/...`

| Суффикс | Макрос | Направление | Назначение |
|---------|--------|-------------|-----------|
| /alive  | TOPIC_ALIVE | Publish | Периодический статус устройства |
| /lwt    | TOPIC_INIT  | LWT Publish | Last Will (offline) |
| /ANS    | TOPIC_ANS   | Publish | Ответ на команду конфигурации (parse) |
| /CFG    | TOPIC_CFG   | Subscribe | Запрос инфо / управление (16-бит маска / флаги) |
| /SFG    | TOPIC_SFG   | Subscribe | Строковые команды/скрипты для `parse()` |
| /INF    | TOPIC_INF   | Publish | Информационная сводка (Wi‑Fi, heap, версия) |
| /FOTA   | TOPIC_FOTA  | Subscribe | URL для OTA обновления |
| /TZD    | TOPIC_TZD   | Subscribe | Установка tzDiff (hex) |
| /TZR    | TOPIC_TZR   | Publish | Текущая TZ строка |
| /HUMT   | TOPIC_HUMT  | Publish | Пакет датчика: время, T, RH, Dew |

QoS: определяется `MQTT_QOS` (в проекте =1). Keepalive — `MQTT_KEEPALIVE` (120 c). Last Will — `/lwt`.

## Конфигурация (структура `SYSCFG`)

Flash-двойная буферизация (две области + флаг) по адресу `CFG_LOCATION`.
Поля: `sta_ssid`, `sta_pwd`, `mqtt_host`, `mqtt_port`, `mqtt_topic_base`, `mqtt_user`, `mqtt_pass`, `node_name`, `node_place`, `TZ`, `mqtt_keepalive`, `security`, `utc`, `tzDiff`.
Сброс к дефолту при несовпадении `CFG_HOLDER`.

## OTA обновление

Через публикацию в топик `/FOTA` строки URL (варианты: `http://host/path/user1.bin`, `host/path/user1.bin`). Прошивка сама подменяет `user1/user2` в зависимости от текущего слота (`system_upgrade_userbin_check`). После успеха — перезапуск.

## Web интерфейс (порт 80)

Главная `/`:

* Отображение версии, SDK, MAC (формат DEV ID), RSSI, heap.
* Поля ввода: Type (node_name), Place, SSID, Password, TZ, Server, Port, TLS level, Topic base, User, Password.
* Ссылки: `certu` (CA cert), `keyu` (private key), `reset`, `clear`.
* POST форматы: либо `enctype='text/plain'` (по строкам), либо стандартный URL encoded (в коде предусмотрена обработка обоих вариантов).

Дополнительные эндпоинты:

`/keyu` — форма загрузки приватного ключа (base64).
`/certu` — форма загрузки CA cert (base64).
`/clear` — сброс `cfg_holder` и сохранение (очистка настроек).
`/id` — JSON идентификатор и capabilities.
`/heap`, `/sdk`, `/ver`, `/reset`.

## Telnet setup (порт 23)

Простой TCP сервер с одной сессией. При подключении переходит в режим Setup (`EnterSetup()`), при закрытии — `LeaveSetup()`, автоматически перезагружает конфигурацию. Каждая строка передаётся в `parse()`; ответ отправляется обратно. Используется для более низкоуровневой настройки или отладки без web.

## Датчик AHT21B

Битбанговый I2C (файлы `i2c.c/h`). Чтение 6 байт, расчёт RH/Temperature и вычисление точки росы (приближённый лог). Периодическое чтение вызывается в `PostData()` (каждые 10 минут по 1s таймеру) и при первичном подключении MQTT.

## Режимы сна

Параметры в `user_main.c`:

* `DEEP_SLEEP_MINUTES` (>0) — после публикации уходит через `system_deep_sleep()` на указанные минуты (нужна перемычка GPIO16→RST). RF опция выставляется на `system_deep_sleep_set_option(1)`.
* Если `DEEP_SLEEP_MINUTES == 0` и `LIGHT_SLEEP_SECONDS > 0` — используется упрощённый light sleep через FPM: отключение сервисов, `wifi_set_opmode(NULL_MODE)`, `wifi_fpm_do_sleep(us)`, затем `system_restart()` после wake.
* Перед сном выключаются HTTP/Telnet (если `SLEEP_STOP_SERVICES`), MQTT disconnect, Wi‑Fi disconnect.

## Память и строки

Крупные HTML шаблоны (`index.c`) размещены во flash (`ICACHE_RODATA_ATTR STORE_ATTR`) и при необходимости копируются в RAM (функция `rom_cpy`). Избегается прямое побайтное чтение из flash для printf.

## Логи и отладка

Основной макрос `PRN` (os_printf). В ряде модулей можно включить подробный вывод раскомментированием `#define PRN os_printf`.

## Сборка и прошивка (Windows MinGW пример)

```bash
mingw32-make -f Makefile all
mingw32-make -f Makefile flash        # прошивка активного user bin
mingw32-make -f Makefile fullflash    # полная прошивка (boot + app + init)
```

Настройки путей к SDK задаются в Makefile / переменных окружения (пример в исходном проекте Tuan PM). Текущая прошивка использует карту flash SPI_FLASH_SIZE_MAP=2.

## Сертификаты TLS

Через web формы (`/certu`, `/keyu`). Данные (base64) конвертируются функцией `b642data()` и пишутся по секторам `CA_CERT_FLASH_ADDRESS`, `CLIENT_CERT_FLASH_ADDRESS`.

## События MQTT / жизненный цикл

* `mqttConnectedCb`: подписка на сервисные топики, публикация alive/info, запуск таймера `_1s_timer`.
* `mqttPublishedCb`: если установлен `PostFlag` (датчик опубликован) — инициирует переход в сон.
* `mqttDataCb`: разбор команд (FOTA, TZD, SFG скрипты, CFG маски, OTA trigger).

## Ошибки и устойчивость

* Таймаут потери связи (`DISCONNECTED_TIMEOUT`) ведёт к `system_restart()`.
* Отслеживание upgrade (`UpgradeRq` → OTA).
* HTTP сервер защищён от переполнения буфера и неверного Content-Length.

## LED / GPIO

* `RLED` (GPIO15) — мигание статуса (alive или активность в setup).
* `KEY0` (GPIO0) — возможно задействован как кнопка (в коде проверяется для сброса/сохранения CFG при удержании > ~5 cек (500 * 10ms)).
* I2C: GPIO4 (SCL), GPIO5 (SDA) настроены pull-up.

## Быстрый старт

1. Настроить переменные в `include/mqtt_config.h` (или сменить `CFG_HOLDER` для сброса на дефолт).
2. Собрать и прошить.
3. Подключить UART @115200 для логов.
4. Найти устройство по MQTT (топик `/alive` / `/INF`).
5. Настроить через Web (http://DEVICE_IP/) или Telnet (порт 23).
6. Проверить публикации `/HUMT`.


## Потенциальные расширения

* Duty-cycle схема с периодическим включением для ещё меньшего среднего потребления.
* Сжатие/минификация HTML или chunked выдача.
* Расширенные диагностические топики (heap fragmentation, RSSI history).

---

## Legacy: Original esp_mqtt README

Адаптировано из проекта Tuan PM `esp_mqtt`. Ниже — только лицензионное уведомление; оригинальное README сокращено для компактности.

### MIT License (esp_mqtt)

Copyright (c) 2014-2015 Tuan PM

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
