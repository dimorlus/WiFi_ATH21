# MQTT Logger

A C++ application for connecting to MQTT brokers and logging data from ESP8266 temperature/humidity sensors to CSV files with custom device naming.

## For Users

### What is MQTT Logger?

MQTT Logger is a background application that automatically collects temperature and humidity data from wireless sensors and saves it to CSV files. It's designed to work with ESP8266-based sensors that send data via MQTT protocol.

### Key Features for Users

- **Automatic Data Collection**: Runs in background, collecting sensor data 24/7
- **Named Devices**: Shows friendly names instead of MAC addresses (e.g., "Vica office" instead of "HT_3C71BF29A68E")
- **CSV Export**: Data saved in Excel-compatible format for easy analysis
- **Real-time Logging**: New sensor readings are immediately saved to files
- **Windows Service**: Can run automatically when Windows starts

### Quick Start for Users

1. **Installation**: 
   
   - Download and run the installer
   - The application will automatically start collecting data

2. **View Your Data**:
   
   - Open Windows Explorer and navigate to `C:\mqtt_Logger\LOG\`
   - You'll find CSV files named after your devices (e.g., "Vica office.csv")
   - Open these files in Excel or any spreadsheet application

3. **Understanding the Data**:
   
   ```csv
   TIME;TEMP;HUM;DEV
   Mon, Jan 01 2024 14:30;23.5;65.2;Vica office
   Mon, Jan 01 2024 14:31;23.6;65.1;Vica office
   ```
   
   - **TIME**: When the measurement was taken
   - **TEMP**: Temperature in Celsius
   - **HUM**: Humidity percentage
   - **DEV**: Device name

### Managing the Service

**Start Data Collection:**

```cmd
startservice.cmd
```

**Stop Data Collection:**

```cmd
stopservice.cmd
```

**Check if Running:**

- Open Task Manager
- Look for "mqtt_logger" in Services tab

### Troubleshooting for Users

**No data appearing?**

1. Check if sensors are powered on
2. Verify WiFi connection on sensors
3. Contact administrator to check MQTT broker connection

**Missing device names?**

- New sensors will appear with their MAC address until added to configuration

**Excel shows weird formatting?**

- Use "Data → Text to Columns" in Excel
- Choose semicolon (;) as separator

---

## For Developers and Administrators

### System Requirements

- **Windows 10/11** (64-bit recommended)
- **Network access** to MQTT broker
- **Administrator rights** for service installation

### Project Structure

```
mqtt_logger/
├── src/                    # Source code
│   ├── main.cpp           # Main application entry point
│   ├── config.cpp         # Configuration management
│   ├── config.h           # Configuration header
│   ├── logger.cpp         # Data logging functionality
│   ├── logger.h           # Logger header
│   ├── replace_macros.cpp # Data transformation utilities
│   └── replace_macros.h   # Macro replacement header
├── build/                 # Build directory (created during compilation)
├── config.ini            # Configuration file
├── CMakeLists.txt         # CMake build configuration
├── Build.bat             # Windows build script
├── build.sh              # Linux build script
├── install_dependencies.bat # Dependency installer
├── setservice.cmd        # Windows service installer
├── startservice.cmd      # Service start script
├── stopservice.cmd       # Service stop script
└── nssm.exe              # Non-Sucking Service Manager
```

### Installation

#### Prerequisites

- **Windows**: MinGW-w64 or MSVC compiler, CMake 3.16+
- **Linux**: GCC, CMake 3.16+
- **vcpkg**: Package manager for C++ libraries

#### 1. Install vcpkg

```bash
# Windows
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Linux
cd /opt
sudo git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
sudo ./bootstrap-vcpkg.sh
```

#### 2. Install Dependencies

**Windows:**

```cmd
# Run the automated installer
install_dependencies.bat

# Or manually:
C:\vcpkg\vcpkg.exe install paho-mqtt:x64-mingw-dynamic
C:\vcpkg\vcpkg.exe install paho-mqttpp3:x64-mingw-dynamic
C:\vcpkg\vcpkg.exe integrate install
```

#### 3. Build the Project

**Windows:**

```cmd
# Automated build
Build.bat

# Or manual build
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

## Configuration

The application uses `config.ini` for all settings:

```ini
[MQTT]
URL=dorlov.no-ip.com:1883
user=dimorlus
passwd=dorlov
topic_base=ORLOV

[DATA]
Header=TIME;TEMP;HUM;DEV
Search=SNTP: ([0-9]+), TEMP: ([0-9\.]+) C, HUM: ([0-9\.]+) %, DEW: ([0-9\.]+) C
Replace=$DateTime('ddd, MMM dd yyyy HH:mm', \1);\2;\3;$NAME

[NAME]
HT_3C71BF29A68E = Vica office
HT_3C71BF29A837 = Lab
HT_3C71BF29AB79 = Clean room
HT_3C71BF28FBD8 = Dima office

[LOG]
Folder="d:\Projects\LED\Alex\WiFi_AHT21\PC\LOG"
Name=$NAME_W$TIME('ww')
```

### Configuration Sections

#### [MQTT] - Broker Connection

- **`URL`**: MQTT broker address and port (e.g., `dorlov.no-ip.com:1883`)
- **`user`** / **`passwd`**: MQTT authentication credentials
- **`topic_base`**: Base topic for MQTT subscriptions (e.g., `ORLOV`)

#### [DATA] - Data Processing

- **`Header`**: CSV column headers (`TIME;TEMP;HUM;DEV`)
- **`Search`**: Regex pattern to parse incoming sensor messages
- **`Replace`**: Template for CSV output format with timestamp conversion

#### [NAME] - Device Mapping

Maps MAC addresses to friendly names:

```ini
HT_3C71BF29A68E = "Vica office"
HT_3C71BF29A837 = "Lab"
```

#### [LOG] - File Output

- **`Folder`**: Directory for CSV files (supports environment variables like `%APPDATA%`)
- **`Name`**: Filename pattern with macro support

**Filename Macros:**

- **`$NAME`**: Device friendly name from [NAME] section
- **`$DEV`**: Device MAC address (e.g., HT_3C71BF29A68E)
- **`$TIME('format')`**: Current date/time when file is created

**Examples:**

```ini
Name=$NAME                    # "Vica office.csv"
Name=$DEV                     # "HT_3C71BF29A68E.csv"  
Name=$NAME_W$TIME('ww')       # "Vica office_W35.csv" (week 35)
Name=$TIME('yyyy-MM')_$NAME   # "2024-08_Vica office.csv"
```

### Time Format Patterns

The `$TIME('format')` and `$DateTime('format', timestamp)` macros support these format specifiers:

#### Date Formats

| Pattern | Description      | Example |
| ------- | ---------------- | ------- |
| `yyyy`  | 4-digit year     | 2024    |
| `yy`    | 2-digit year     | 24      |
| `MMMM`  | Full month name  | January |
| `MMM`   | Short month name | Jan     |
| `MM`    | 2-digit month    | 01      |
| `M`     | Month number     | 1       |
| `dddd`  | Full day name    | Monday  |
| `ddd`   | Short day name   | Mon     |
| `dd`    | 2-digit day      | 01      |
| `d`     | Day number       | 1       |
| `ww`    | ISO week number  | 35      |

#### Time Formats

| Pattern | Description              | Example |
| ------- | ------------------------ | ------- |
| `HH`    | 24-hour format (2-digit) | 14      |
| `H`     | 24-hour format           | 14      |
| `hh`    | 12-hour format (2-digit) | 02      |
| `h`     | 12-hour format           | 2       |
| `mm`    | 2-digit minutes          | 30      |
| `m`     | Minutes                  | 30      |
| `ss`    | 2-digit seconds          | 45      |
| `s`     | Seconds                  | 45      |
| `AM/PM` | Upper case AM/PM         | PM      |
| `am/pm` | Lower case am/pm         | pm      |

#### Common Format Examples

```ini
# Date formats
$TIME('yyyy-MM-dd')           # 2024-08-27
$TIME('dd/MM/yyyy')           # 27/08/2024  
$TIME('MMM dd, yyyy')         # Aug 27, 2024
$TIME('ddd, MMM dd yyyy')     # Tue, Aug 27 2024

# Time formats
$TIME('HH:mm')                # 14:30
$TIME('HH:mm:ss')             # 14:30:45
$TIME('hh:mm AM/PM')          # 02:30 PM

# Combined formats
$TIME('yyyy-MM-dd HH:mm')     # 2024-08-27 14:30
$TIME('ddd, MMM dd HH:mm')    # Tue, Aug 27 14:30
$TIME('yyyy-MM-dd hh:mm am/pm') # 2024-08-27 02:30 pm

# Week and special formats
$TIME('ww')                   # 35 (week number)
$TIME('yyyy-W ww')            # 2024-W 35
```

#### Literal Text in Formats

Use single quotes to include literal text:

```ini
$TIME('Week' ww 'of' yyyy)    # Week 35 of 2024
$TIME('Log_'yyyy-MM-dd)       # Log_2024-08-27
```

### Complete Configuration Example

```ini
[MQTT]
URL=dorlov.no-ip.com:1883
user=dimorlus
passwd=dorlov
topic_base=ORLOV

[DATA]
Header=TIME;TEMP;HUM;DEV
Search=SNTP: ([0-9]+), TEMP: ([0-9\.]+) C, HUM: ([0-9\.]+) %, DEW: ([0-9\.]+) C
Replace=$DateTime('ddd, MMM dd yyyy HH:mm', \1);\2;\3;$NAME

[NAME]
HT_3C71BF29A68E = Vica office
HT_3C71BF29A837 = Lab
HT_3C71BF29AB79 = Clean room
HT_3C71BF28FBD8 = Dima office

[LOG]
Folder="d:\Projects\LED\Alex\WiFi_AHT21\PC\LOG"
Name=$NAME_W$TIME('ww')
```

### Advanced Filename Examples

**Weekly Log Files:**

```ini
Name=$NAME_W$TIME('ww')       # Vica office_W35.csv
```

Creates new file each week.

**Monthly Log Files:**

```ini
Name=$TIME('yyyy-MM')_$NAME   # 2024-08_Vica office.csv
```

Creates new file each month.

**Daily Log Files:**

```ini
Name=$NAME_$TIME('yyyy-MM-dd') # Vica office_2024-08-27.csv
```

Creates new file each day.

**Device Type Grouping:**

```ini
Name=Sensors_$TIME('yyyy-MM')  # Sensors_2024-08.csv
```

All devices log to same monthly file.

### Data Processing Macros

In the **Replace** pattern, you can use:

#### Timestamp Conversion

```ini
Replace=$DateTime('ddd, MMM dd yyyy HH:mm', \1);\2;\3;$NAME
```

- `\1` = First regex capture group (Unix timestamp)
- `\2` = Second capture group (temperature)  
- `\3` = Third capture group (humidity)
- `$NAME` = Device friendly name

#### Current Time

```ini
Replace=$TIME('yyyy-MM-dd HH:mm:ss');\2;\3;$NAME
```

Uses current system time instead of sensor timestamp.

#### Multiple Timestamp Formats

```ini
# For different sensor message formats:
Search=Time:([0-9]+),Temp:([0-9\.]+),Hum:([0-9\.]+)
Replace=$DateTime('yyyy-MM-dd HH:mm', \1);\2;\3;$DEV
```

### Troubleshooting Time Formats

**Wrong timestamp format:**

- Check if sensor sends Unix timestamp (seconds since 1970)
- Verify regex captures timestamp in first group `\1`
- Test with different DateTime format patterns

**Files not created with expected names:**

- Check folder permissions for LOG directory
- Verify $TIME macros use valid format patterns  
- Restart service after config changes

**Week numbers seem wrong:**

- Week numbers follow ISO 8601 standard (Monday = week start)
- Week 1 contains January 4th
- Some years have 53 weeks
