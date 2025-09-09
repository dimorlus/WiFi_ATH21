# HT WebView

🌡️ **Temperature & Humidity Data Visualizer**

A modern web-based application for visualizing temperature and humidity data from CSV files. Built with React, TypeScript, and Vite.

## Features

- 📁 **CSV File Import** - Load data from your sensor logs
- 📊 **Interactive Charts** - Zoom with mouse wheel, drag to pan
- 📈 **Combined & Separate Views** - Temperature and humidity charts
- 🎛️ **Brush Navigation** - Quick data range selection
- 📊 **Real-time Statistics** - Min/Max/Average calculations
- 📈 **Data Preview** - Quick overview of recent measurements
- 🎨 **Modern UI** - Clean, responsive design
- 🌙 **Dark Mode** - Automatic theme switching
- 🔗 **Network Access** - Can be accessed from local network

## Supported CSV Format

The application expects CSV files with the following format:

```csv
TIME;TEMP;HUM;DEV
Mon, Jan 01 2024 14:30;23.5;65.2;Vica office
Mon, Jan 01 2024 14:31;23.6;65.1;Vica office
```

Where:
- **TIME**: Timestamp in readable format
- **TEMP**: Temperature in Celsius
- **HUM**: Humidity percentage
- **DEV**: Device name

## Quick Start

### Prerequisites

- Node.js 18+ (Download from [nodejs.org](https://nodejs.org/))
- A modern web browser

### Installation

1. **Install Node.js** (if not already installed)
   - Download from https://nodejs.org/
   - Choose the LTS version
   - Run the installer

2. **Install dependencies:**
   ```bash
   npm install
   ```

3. **Start development server:**
   ```bash
   npm run dev
   ```

4. **Open in browser:**
   - Local: http://localhost:5173
   - Network: http://[your-ip]:5173

### Building for Production

```bash
# Build the project
npm run build

# Preview the build
npm run preview
```

## Network Access

To access from other devices on your network:

1. **Start with network access:**
   ```bash
   npm run dev
   ```

2. **Find your IP address:**
   - Windows: `ipconfig`
   - Find your local IP (e.g., 192.168.1.100)

3. **Access from other devices:**
   - Open browser on any device in your network
   - Go to: `http://192.168.1.100:5173`

## Usage

1. **Load CSV File:**
   - Click "📁 Select CSV File" or drag & drop
   - Choose your sensor data file
   - Data will be automatically parsed and displayed

2. **Interactive Charts:**
   - **Mouse Wheel**: Zoom in/out on chart area
   - **Drag**: Pan left/right when zoomed in
   - **Brush Bar**: Use bottom slider for quick navigation
   - **Chart Types**: Switch between Combined and Separate views

3. **View Statistics:**
   - Temperature and humidity statistics calculated automatically
   - Min/Max/Average values shown in cards
   - Data point count and range information

4. **Device Filtering:**
   - Filter data by specific sensor devices
   - View data from individual sensors or all combined

## Integration with WiFi_ATH21 System

This web viewer is designed to work with:

- **ESP8266 Sensors** - WiFi temperature/humidity sensors
- **MQTT Logger** - Collects data and saves to CSV files
- **HT_view** - Desktop Qt application for detailed analysis

The web viewer provides a lightweight, network-accessible interface for quick data monitoring.

## Tech Stack

- **React 18** - UI framework
- **TypeScript** - Type safety
- **Vite** - Build tool and dev server
- **CSS3** - Styling with dark mode support

## Development

### Project Structure

```
src/
├── App.tsx          # Main application component
├── App.css          # Application styles
├── main.tsx         # Entry point
└── index.css        # Global styles
```

### Available Scripts

- `npm run dev` - Start development server
- `npm run build` - Build for production
- `npm run preview` - Preview production build
- `npm run lint` - Run ESLint

## Current Interactive Features

- ✅ **Mouse Wheel Zoom** - Zoom in/out on chart data
- ✅ **Drag to Pan** - Navigate through data by dragging
- ✅ **Brush Navigation** - Quick range selection with bottom slider
- ✅ **Combined Charts** - Temperature and humidity on dual Y-axis
- ✅ **Separate Charts** - Individual charts for detailed analysis
- ✅ **Device Filtering** - Filter data by sensor device
- ✅ **Production Deploy** - Windows Service installation

## Future Enhancements

- 🔄 Auto-refresh when CSV files change
- 📱 Mobile-optimized interface
- 📊 Advanced statistical analysis
- 💾 Export functionality
- 🎯 Real-time MQTT data streaming

## License

Part of the WiFi_ATH21 project.

## Support

For issues or questions, please refer to the main WiFi_ATH21 project repository.
