# HUMT ESP8266 Device (MQTT Temperature & Humidity Node)

[Русская версия](./README.md) | English

Firmware for an ESP8266 (NON-OS SDK 2.2.1) based environmental node with an AHT21B temperature/humidity sensor. Publishes readings via MQTT and supports local configuration through a lightweight Web UI (port 80) and a Telnet setup interface (port 23). Built on a fork/adaptation of `esp_mqtt` (Tuan PM) plus application logic for power management, OTA and secure configuration.

## Quick Overview

Boot sequence:

1. Load (or initialize) persisted configuration from flash (dual sector scheme).
2. Connect to Wi‑Fi (STA) and sync time via SNTP.
3. Connect to the MQTT broker, publish initial status/info + sensor packet.
4. After confirmed publish, enter sleep (deep or light) according to build parameters (`DEEP_SLEEP_MINUTES`, `LIGHT_SLEEP_SECONDS`).

## Features

* MQTT client (QoS1), subscriptions to service/control topics, LWT support.
* Temperature / Humidity / Dew point from AHT21B (bit‑banged I2C).
* HTTP OTA update (download `user1/user2.bin`) triggered through MQTT `/FOTA` topic.
* Web configuration interface (Wi‑Fi, MQTT, TZ, TLS level, credentials, topic base).
* Telnet setup (port 23) – simple line protocol interpreted by `parse()` for scripting / batch configuration.
* SSL/TLS levels (0..3) with CA + client key/cert stored in flash (base64 upload forms).
* Two sleep modes: Deep sleep (lowest consumption, requires GPIO16→RST) and Light sleep (FPM, RAM retained, restart after wake).
* Timers/watchdog style reconnect / restart on prolonged disconnection.
* Optional pre‑sleep shutdown of HTTP/Telnet services (`SLEEP_STOP_SERVICES`).

## MQTT Topics

Base prefix pattern: `MQTT_TOPIC_BASE/MQTT_TOPIC_TYPE/DEV` and/or `MQTT_TOPIC_BASE/<MAC>/...`

| Suffix | Macro        | Direction  | Purpose                                                |
|--------|--------------|-----------|--------------------------------------------------------|
| /alive | TOPIC_ALIVE  | Publish    | Periodic alive / heartbeat                             |
| /lwt   | TOPIC_INIT   | LWT Publish| Broker Last Will ("offline")                          |
| /ANS   | TOPIC_ANS    | Publish    | Answer to configuration command (`parse` response)     |
| /CFG   | TOPIC_CFG    | Subscribe  | Info request / bitmask control flags                  |
| /SFG   | TOPIC_SFG    | Subscribe  | String script/commands for `parse()`                  |
| /INF   | TOPIC_INF    | Publish    | Informational snapshot (Wi‑Fi, heap, version)         |
| /FOTA  | TOPIC_FOTA   | Subscribe  | OTA firmware URL                                      |
| /TZD   | TOPIC_TZD    | Subscribe  | Set tzDiff (hex)                                      |
| /TZR   | TOPIC_TZR    | Publish    | Current TZ string                                     |
| /HUMT  | TOPIC_HUMT   | Publish    | Sensor packet: time, Temperature, RH, Dew             |

QoS is defined by `MQTT_QOS` (set to 1). Keepalive: `MQTT_KEEPALIVE` (120 s). LWT topic: `/lwt`.

## Configuration (`SYSCFG`)

Dual sector flash persistence at `CFG_LOCATION` with a flag indicating active sector. Fields:
`sta_ssid`, `sta_pwd`, `mqtt_host`, `mqtt_port`, `mqtt_topic_base`, `mqtt_user`, `mqtt_pass`, `node_name`, `node_place`, `TZ`, `mqtt_keepalive`, `security`, `utc`, `tzDiff`.
Defaults re-applied when `CFG_HOLDER` changes.

## OTA Update

Triggered by publishing a URL to `/FOTA` (e.g.: `http://host/path/user1.bin` or `host/path/user1.bin`). Firmware automatically swaps `user1/user2` based on current slot (`system_upgrade_userbin_check`). Device restarts after success.

## Web Interface (Port 80)

Root `/` page:

* Shows version, SDK, MAC (DEV ID format), RSSI, heap.
* Input fields: Type (node_name), Place, SSID, Password, TZ, Server, Port, TLS level, Topic base, User, Password.
* Links: `certu` (CA cert upload), `keyu` (private key upload), `reset`, `clear`.
* POST encodings supported: `text/plain` (line based) or standard URL encoded.

Extra endpoints:

* `/keyu` – private key (base64) upload form.
* `/certu` – CA certificate (base64) upload form.
* `/clear` – reset `cfg_holder` & save (factory defaults on next boot).
* `/id` – JSON identity / capability data.
* `/heap`, `/sdk`, `/ver`, `/reset` – diagnostics & control.

## Telnet Setup (Port 23)

Single-client TCP server. On connect: `EnterSetup()`; on disconnect: `LeaveSetup()` (triggers config reload). Each received line forwarded to `parse()`; response echoed back. Convenient for automation or when HTTP is disabled before sleep.

## AHT21B Sensor

Bit‑banged I2C implementation. Reads 6 raw bytes, converts to relative humidity & temperature, computes dew point (approximate logarithmic formula). Periodic acquisition in `PostData()` (every 10 minutes via 1s timer) and also right after first MQTT connect.

## Sleep Modes

Build parameters in `user_main.c`:

* `DEEP_SLEEP_MINUTES` (>0): after publish uses `system_deep_sleep()` for specified minutes (requires GPIO16→RST). RF option set via `system_deep_sleep_set_option(1)`.
* If `DEEP_SLEEP_MINUTES == 0` and `LIGHT_SLEEP_SECONDS > 0`: simplified light sleep via FPM (`wifi_set_opmode(NULL_MODE)`, `wifi_fpm_do_sleep(us)`, then `system_restart()` upon wake).
* Prior to sleep: HTTP/Telnet stopped (if `SLEEP_STOP_SERVICES`), MQTT disconnect, Wi‑Fi disconnect.

## Flash Strings / Memory

Large HTML templates (`index.c`) stored in flash with `ICACHE_RODATA_ATTR STORE_ATTR` and copied to RAM using `rom_cpy` before formatting. Avoids unsafe byte-wise flash reads in formatted output.

## Logging & Debug

Primary macro: `PRN` (wrapping `os_printf`). Extra verbose debug can be enabled in modules by uncommenting `#define PRN os_printf` lines.

## Build & Flash (Windows MinGW Example)

```bash
mingw32-make -f Makefile all
mingw32-make -f Makefile flash        # flash active user bin
mingw32-make -f Makefile fullflash    # boot + app + init
```

SDK path & toolchain configured via Makefile or environment variables (see upstream `esp_mqtt`). SPI flash map used: `SPI_FLASH_SIZE_MAP=2`.

## TLS Certificates

Upload through `/certu` (CA) and `/keyu` (private key). Base64 content decoded via `b642data()` and stored at `CA_CERT_FLASH_ADDRESS`, `CLIENT_CERT_FLASH_ADDRESS` sectors.

## MQTT Lifecycle Events

* `mqttConnectedCb`: subscribes to control topics, publishes alive/info, starts 1s timer.
* `mqttPublishedCb`: when `PostFlag` set (sensor published) triggers sleep transition.
* `mqttDataCb`: parses commands (FOTA URL, TZD hex, SFG scripts, CFG mask, OTA trigger).

## Robustness & Recovery

* Disconnection timeout (`DISCONNECTED_TIMEOUT`) leads to `system_restart()`.
* OTA tracking via `UpgradeRq` flag.
* HTTP server enforces buffer bounds & correct Content-Length.

## LED / GPIO

* `RLED` (GPIO15) – status blink (alive / setup activity).
* `KEY0` (GPIO0) – button (long press > ~5s handled for config save/reset logic).
* I2C pins: GPIO4 (SCL), GPIO5 (SDA) with pull-ups.

## Quick Start

1. Adjust defaults in `include/mqtt_config.h` (or change `CFG_HOLDER` to force re-init).
2. Build & flash.
3. Connect UART @115200 for logs.
4. Discover device via MQTT (`/alive` / `/INF`).
5. Configure using Web (http://DEVICE_IP/) or Telnet (port 23).
6. Verify `/HUMT` sensor publications.

## Possible Future Enhancements

* Duty-cycle scheduling for even lower average consumption.
* HTML minification or chunked transfer.
* Extended diagnostics topics (heap fragmentation, RSSI history).

---

## Legacy Attribution (esp_mqtt)

Adapted from Tuan PM `esp_mqtt`. Below is the retained license notice.

### MIT License (esp_mqtt)

Copyright (c) 2014-2015 Tuan PM

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
