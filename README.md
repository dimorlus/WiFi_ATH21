"# WiFi_ATH21

ESP8266-based WiFi temperature and humidity monitoring system with MQTT data logging.

## Overview

This project implements a complete IoT solution for monitoring temperature and humidity using the AHT21 sensor connected to an ESP8266 microcontroller. The system sends data via MQTT protocol to a broker, where it can be logged and processed by a PC application.

### Key Features

- **ESP8266 Firmware**: Reads AHT21 sensor data and transmits via WiFi/MQTT
- **PC Logger Application**: Receives MQTT data and logs to CSV files
- **Web Interface**: Configuration and monitoring through HTTP interface
- **SNTP Time Synchronization**: Accurate timestamps for all measurements
- **OTA Updates**: Over-the-air firmware updates capability
- **Multiple Device Support**: Single broker can handle multiple sensor nodes

## Hardware Components

- **ESP8266** (NodeMCU, Wemos D1 Mini, or similar)
- **AHT21** Temperature and Humidity Sensor
- **Power Supply** (5V USB or 3.3V regulated)

## System Architecture

```
┌─────────────────┐    WiFi/MQTT    ┌─────────────────┐    Data     ┌─────────────────┐
│   ESP8266       │ ──────────────→ │   MQTT Broker   │ ──────────→ │   PC Logger     │
│   + AHT21       │                 │                 │             │   Application   │
│   Sensor Node   │                 │ (mosquitto etc) │             │                 │
└─────────────────┘                 └─────────────────┘             └─────────────────┘
        │                                   │                               │
        │ Reads temp/hum                    │ Topic:                        │ Saves to:
        │ every 30s                         │ ORLOV/HUMT/{MAC}/HUMT         │ {MAC}.csv
        │                                   │                               │
        ▼                                   ▼                               ▼
┌─────────────────┐                 ┌─────────────────┐             ┌─────────────────┐
│ Temperature:    │                 │ Data Format:    │             │ CSV Format:     │
│ 20.0°C - 40.0°C │                 │ SNTP: timestamp │             │ SNTP;TEMP;HUM   │
│ Humidity:       │                 │ TEMP: XX.X C    │             │ 1634567890;     │
│ 10% - 90% RH    │                 │ HUM: XX.X %     │             │ 26.4;56.3       │
└─────────────────┘                 └─────────────────┘             └─────────────────┘
```

## Project Structure

```
WiFi_ATH21/
├── ESP/                     # ESP8266 firmware
│   └── mqtt_aht21/         # Main firmware project
│       ├── user/           # Application code
│       ├── mqtt/           # MQTT client library
│       ├── driver/         # Hardware drivers (AHT21, GPIO, etc.)
│       └── include/        # Header files
├── PC/                     # PC applications
│   └── mqtt_logger/        # MQTT data logger
│       ├── src/            # C++ source files
│       ├── CMakeLists.txt  # Build configuration
│       └── readme.md       # Detailed setup instructions
├── Datasheets/             # Component datasheets and libraries
│   └── AHT21/             # AHT21 sensor documentation
├── Schematic/              # Circuit schematics (Proteus format)
├── Production/             # Production files and flashing tools
├── LICENSE                 # MIT License
├── CONTRIBUTING.md         # Contribution guidelines
└── README.md              # This file
```

## Getting Started

### Hardware Setup

1. **Wiring Diagram**:
   ```
   ESP8266    AHT21
   -------    -----
   3.3V   --> VCC
   GND    --> GND
   D1     --> SCL
   D2     --> SDA
   ```

2. **Power Requirements**:
   - ESP8266: 3.3V, ~80mA (WiFi active)
   - AHT21: 3.3V, ~0.5mA

### ESP8266 Firmware Setup

1. **Prerequisites**:
   - ESP8266 SDK v0.9.4 or higher
   - Xtensa GCC toolchain
   - esptool for flashing
   - Make utility

2. **Hardware Connection**:
   ```
   ESP8266 (NodeMCU)    AHT21 Sensor
   ==================   ============
   3.3V (3V3)      ──→  VCC
   GND             ──→  GND  
   D1 (GPIO5)      ──→  SCL (I2C Clock)
   D2 (GPIO4)      ──→  SDA (I2C Data)
   ```

3. **Building the Firmware**:
   ```bash
   cd ESP/mqtt_aht21
   
   # For Windows (MinGW):
   mingw32-make SDK_BASE="C:/Espressif/ESP8266_SDK" FLAVOR="release" all
   
   # For Linux/Mac:
   make SDK_BASE="/opt/Espressif/ESP8266_SDK" FLAVOR="release" all
   ```

4. **Flashing the Firmware**:
   ```bash
   # Windows:
   mingw32-make ESPPORT="COM3" flash
   
   # Linux/Mac:
   make ESPPORT="/dev/ttyUSB0" flash
   ```

5. **First-time Setup**:
   - Connect to WiFi hotspot: `ESP8266_XXXXXX` (where XXXXXX is part of MAC address)
   - Open browser to: `http://192.168.4.1`
   - Configure your WiFi credentials
   - Set MQTT broker settings
   - Save configuration and reboot

### PC Logger Setup

The PC application receives MQTT data and logs it to CSV files with timestamp conversion.

#### Windows Setup:

1. **Install vcpkg** (package manager):
   ```bash
   cd C:\
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

2. **Install dependencies**:
   ```bash
   vcpkg install paho-mqtt:x64-windows
   vcpkg install paho-mqttpp3:x64-windows
   ```

3. **Build application**:
   ```bash
   cd PC/mqtt_logger
   mkdir build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
   cmake --build . --config Release
   ```

#### Linux Setup:

1. **Install dependencies**:
   ```bash
   # Ubuntu/Debian:
   sudo apt update
   sudo apt install build-essential cmake libpaho-mqtt-dev libpaho-mqttpp-dev
   
   # Or build from source with vcpkg
   ```

2. **Build application**:
   ```bash
   cd PC/mqtt_logger
   mkdir build && cd build
   cmake ..
   make -j4
   ```

3. **Run the application**:
   ```bash
   ./mqtt_logger
   ```

#### Configuration:

The application creates a configuration file on first run:
- **Windows**: `%APPDATA%\mqtt_logger\config.ini`
- **Linux**: `~/.config/mqtt_logger/config.ini`

Example configuration:
```ini
[MQTT]
URL=your-broker.com:1883
user=your_mqtt_username
passwd=your_mqtt_password
topic_base=ORLOV

[DATA]
Header=SNTP;TEMP;HUM
Search="SNTP: ([0-9]+), TEMP: ([0-9\.]+) C, HUM: ([0-9\.]+) %"
Replace="\1;\2;\3"
LogFolder=  # Leave empty for default location
```

## Usage

### MQTT Protocol

The system uses the following MQTT structure:

- **Topic Pattern**: `ORLOV/HUMT/{DEVICE_ID}/HUMT`
- **Data Format**: `SNTP: {timestamp}, TEMP: {temp} C, HUM: {hum} %, DEW: {dewpoint} C`

Example:
```
Topic: ORLOV/HUMT/HT_3C71BF29A68E/HUMT
Data: SNTP: 1755580662, TEMP: 26.4 C, HUM: 56.3 %, DEW: 17.0 C
```

### Data Logging

The PC application converts MQTT data to CSV format:
```csv
SNTP,TEMP,HUM
1755580662,26.4,56.3
```

Files are saved as `{DEVICE_ID}.csv` in the configured data directory.

## Configuration

### ESP8266 Configuration

Access the web interface at the device's IP address:

- **WiFi Settings**: SSID and password
- **MQTT Broker**: Server address, port, credentials
- **Device Settings**: Update interval, device name
- **NTP Server**: Time synchronization settings

### PC Logger Configuration

Edit `config.ini`:
```ini
[MQTT]
URL=your-broker.com:1883
user=your_username
passwd=your_password
topic_base=ORLOV

[DATA]
Header=SNTP;TEMP;HUM
Search="SNTP: ([0-9]+), TEMP: ([0-9\.]+) C, HUM: ([0-9\.]+) %"
Replace="\1;\2;\3"
```

## Troubleshooting

### ESP8266 Issues

#### WiFi Connection Problems
- **Check credentials**: Verify SSID and password in web interface
- **Signal strength**: Ensure ESP8266 is within range of WiFi router
- **Power supply**: Use stable 3.3V supply (USB power recommended)
- **Reset procedure**: Hold GPIO0 low during power-on for programming mode

#### MQTT Connection Issues
- **Broker accessibility**: Ping the MQTT broker from your network
- **Port settings**: Default MQTT port is 1883 (8883 for SSL)
- **Authentication**: Verify username/password if broker requires authentication
- **Topic permissions**: Ensure user has publish rights to configured topics
- **Network firewall**: Check for blocked MQTT ports

#### Sensor Reading Issues
- **I2C wiring**: Verify SDA (GPIO4) and SCL (GPIO5) connections
- **Pull-up resistors**: AHT21 may need 4.7kΩ pull-ups (usually built-in on breakout boards)
- **Power supply**: AHT21 requires stable 3.3V supply
- **Timing issues**: Sensor needs 20ms startup time after power-on

#### Common Error Messages
- `wifi: connect failed`: Wrong SSID/password or network issues
- `mqtt: connection refused`: Broker unreachable or authentication failed
- `sensor: read timeout`: I2C communication problem or sensor failure

### PC Logger Issues

#### MQTT Connection Problems
- **Broker settings**: Verify broker URL, port, and credentials in `config.ini`
- **Network connectivity**: Test broker connection with MQTT client (like MQTT Explorer)
- **Firewall rules**: Ensure outbound connections to MQTT port are allowed
- **SSL/TLS issues**: For secure connections, verify certificate settings

#### File System Issues
- **Permissions**: Ensure write access to data directory
  - Windows: Check `%APPDATA%\mqtt_logger\` folder permissions
  - Linux: Check `~/.local/share/mqtt_logger/` folder permissions
- **Disk space**: Verify sufficient space for CSV log files
- **File locking**: Close any CSV files that might be open in Excel or other programs

#### Data Format Issues
- **Regex patterns**: Verify `Search` pattern matches incoming MQTT data exactly
- **Replace format**: Check `Replace` pattern uses correct capture groups
- **Character encoding**: Ensure UTF-8 encoding for international characters

#### Build and Compilation Issues
- **Missing libraries**: Install Paho MQTT libraries through vcpkg or package manager
- **CMake version**: Requires CMake 3.15 or higher
- **Compiler compatibility**: Use C++17 compatible compiler (GCC 7+, MSVC 2017+)

### Hardware Debugging

#### Voltage Levels
- **ESP8266**: 3.3V ±0.3V on VCC pin
- **AHT21**: 3.3V ±0.3V on VCC pin
- **Logic levels**: All I2C and GPIO signals should be 3.3V logic

#### Current Consumption
- **Normal operation**: ~80mA during WiFi transmission, ~20mA in standby
- **Power saving**: ESP8266 supports deep sleep for battery applications

#### Temperature Ranges
- **Operating range**: ESP8266: -40°C to +125°C, AHT21: -40°C to +80°C
- **Accuracy**: AHT21: ±0.3°C for temperature, ±2% RH for humidity

### Monitoring and Diagnostics

#### Serial Console Output
Connect to ESP8266 serial port (115200 baud) to view debug messages:
```
WiFi connected to: YourSSID
IP address: 192.168.1.100
MQTT connecting to: broker.example.com:1883
MQTT connected, subscribing to topics
Sensor reading: 24.5°C, 55.2% RH
Published: ORLOV/HUMT/HT_ABC123DEF456/HUMT
```

#### MQTT Message Monitoring
Use MQTT client tools to monitor published messages:
- **Topic**: `ORLOV/HUMT/+/HUMT`
- **Expected format**: `SNTP: 1234567890, TEMP: 24.5 C, HUM: 55.2 %, DEW: 15.8 C`

#### Log File Analysis
Check PC logger output files:
- **Location**: Data directory configured in `config.ini`
- **Format**: `{DEVICE_MAC}.csv` with semicolon-separated values
- **Content**: Timestamp, temperature, humidity per line

### Getting Help

1. **Check this troubleshooting section** first
2. **Search existing GitHub issues** for similar problems
3. **Enable debug output** in both ESP8266 and PC logger
4. **Create detailed issue report** including:
   - Hardware setup and wiring
   - Software versions and configuration
   - Error messages and logs
   - Steps to reproduce the problem

## Development

### Building from Source

1. **ESP8266 Development**:
   - Use ESP8266 SDK and appropriate toolchain
   - Modify `user_main.c` for custom functionality

2. **PC Application**:
   - Uses C++ with Paho MQTT library
   - CMake build system
   - Cross-platform (Windows/Linux)

### Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Credits

- ESP MQTT library by [Tuan PM](https://github.com/tuanpmt/esp_mqtt)
- AHT21 sensor library
- Paho MQTT C++ library

## Support

For issues and questions:
1. Check the troubleshooting section
2. Review existing issues on GitHub
3. Create a new issue with detailed description

## Making the Repository Public

This repository is now properly prepared for public release with:

- ✅ **Comprehensive Documentation**: Complete README with setup instructions
- ✅ **Open Source License**: MIT License allowing free use and modification  
- ✅ **Clear Project Structure**: Well-organized code and documentation
- ✅ **Contribution Guidelines**: CONTRIBUTING.md with development standards
- ✅ **Clean Repository**: Build artifacts removed, proper .gitignore
- ✅ **Technical Documentation**: Architecture diagrams and troubleshooting

To make the repository public on GitHub:
1. Go to repository Settings
2. Scroll to "Danger Zone" 
3. Click "Change repository visibility"
4. Select "Make public"
5. Confirm the change

The repository is now ready for public access and community contributions!" 
