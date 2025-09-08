import { useState, useMemo } from 'react'
import { InteractiveChart } from './components/InteractiveChart'
import { CombinedChart } from './components/CombinedChart'
import './App.css'

interface SensorData {
  time: string
  temperature: number
  humidity: number
  device: string
}

interface ChartData {
  time: string
  [key: string]: number | string
}

function App() {
  const [data, setData] = useState<SensorData[]>([])
  const [selectedFile, setSelectedFile] = useState<File | null>(null)
  const [selectedDevice, setSelectedDevice] = useState<string>('all')
  const [chartMode, setChartMode] = useState<'combined' | 'separate'>('combined')
  const [brushData, setBrushData] = useState<{ startIndex?: number; endIndex?: number }>({})

  // Prepare chart data
  const chartData = useMemo(() => {
    if (data.length === 0) return []
    
    const filteredData = selectedDevice === 'all' 
      ? data 
      : data.filter(d => d.device === selectedDevice)
    
    return filteredData.map((item, index) => ({
      index,
      time: item.time, // Keep original time string with date
      shortTime: new Date(item.time).toLocaleString('ru-RU', { 
        day: '2-digit',
        month: '2-digit', 
        hour: '2-digit', 
        minute: '2-digit' 
      }), // Включаем дату для уникальности: "06.09 08:10"
      fullDateTime: new Date(item.time).toLocaleString('ru-RU', {
        day: '2-digit',
        month: '2-digit', 
        year: 'numeric',
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit'
      }),
      temperature: item.temperature,
      humidity: item.humidity,
      device: item.device
    }))
  }, [data, selectedDevice])

  // Get unique devices
  const devices = useMemo(() => {
    const uniqueDevices = [...new Set(data.map(d => d.device))]
    return uniqueDevices
  }, [data])

  // Statistics for visible data
  const stats = useMemo(() => {
    if (chartData.length === 0) return null
    
    let visibleData = chartData
    if (brushData.startIndex !== undefined && brushData.endIndex !== undefined) {
      visibleData = chartData.slice(brushData.startIndex, brushData.endIndex + 1)
    }
    
    const temperatures = visibleData.map(d => d.temperature)
    const humidities = visibleData.map(d => d.humidity)
    
    return {
      temperature: {
        min: Math.min(...temperatures),
        max: Math.max(...temperatures),
        avg: temperatures.reduce((sum, val) => sum + val, 0) / temperatures.length,
        current: temperatures[temperatures.length - 1]
      },
      humidity: {
        min: Math.min(...humidities),
        max: Math.max(...humidities),
        avg: humidities.reduce((sum, val) => sum + val, 0) / humidities.length,
        current: humidities[humidities.length - 1]
      },
      count: visibleData.length,
      timeRange: visibleData.length > 0 ? {
        start: visibleData[0].fullDateTime,
        end: visibleData[visibleData.length - 1].fullDateTime
      } : null
    }
  }, [chartData, brushData])

  const handleBrushChange = (newBrushData: any) => {
    setBrushData({
      startIndex: newBrushData?.startIndex,
      endIndex: newBrushData?.endIndex
    })
  }

  const handleFileSelect = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0]
    if (file) {
      setSelectedFile(file)
      parseCSVFile(file)
    }
  }

  const parseCSVFile = async (file: File) => {
    const text = await file.text()
    const lines = text.split('\n')
    const parsedData: SensorData[] = []

    for (const line of lines) {
      if (line.trim() && !line.startsWith('TIME')) {
        const [time, temp, hum, device] = line.split(';')
        if (time && temp && hum && device) {
          // Try to parse different date formats
          let parsedTime = time.trim()
          
          // If it doesn't include date, add today's date
          if (!parsedTime.includes(',') && !parsedTime.includes('-') && !parsedTime.includes('/')) {
            const today = new Date().toLocaleDateString('ru-RU')
            parsedTime = `${today} ${parsedTime}`
          }
          
          parsedData.push({
            time: parsedTime,
            temperature: parseFloat(temp.trim()),
            humidity: parseFloat(hum.trim()),
            device: device.trim().replace(/"/g, '')
          })
        }
      }
    }

    console.log(`Loaded ${parsedData.length} records`)
    setData(parsedData)
  }

  return (
    <div className="App">
      <header className="app-header">
        <h1>🌡️ HT WebView</h1>
        <p>Temperature & Humidity Data Visualizer</p>
      </header>

      <main className="main-content">
        <div className="file-upload">
          <label htmlFor="csv-file" className="file-label">
            📁 Select CSV File
          </label>
          <input
            id="csv-file"
            type="file"
            accept=".csv"
            onChange={handleFileSelect}
            className="file-input"
          />
          {selectedFile && (
            <p className="file-info">
              Loaded: {selectedFile.name} ({data.length} records)
            </p>
          )}
        </div>

        {data.length > 0 && (
          <>
            <div className="controls-panel">
              <div className="device-filter">
                <label htmlFor="device-select">📱 Device Filter: </label>
                <select 
                  id="device-select"
                  value={selectedDevice}
                  onChange={(e) => setSelectedDevice(e.target.value)}
                  className="device-select"
                >
                  <option value="all">All Devices</option>
                  {devices.map(device => (
                    <option key={device} value={device}>{device}</option>
                  ))}
                </select>
              </div>

              <div className="chart-mode-toggle">
                <label htmlFor="chart-mode">📊 View Mode: </label>
                <select 
                  id="chart-mode"
                  value={chartMode}
                  onChange={(e) => setChartMode(e.target.value as 'combined' | 'separate')}
                  className="mode-select"
                >
                  <option value="combined">Combined Chart</option>
                  <option value="separate">Separate Charts</option>
                </select>
              </div>
            </div>

            {stats && (
              <div className="stats-enhanced">
                <div className="stat-card">
                  <h3>🌡️ Temperature</h3>
                  <div className="stat-row">
                    <span className="stat-label">Current:</span>
                    <span className="stat-value current">{stats.temperature.current?.toFixed(1)}°C</span>
                  </div>
                  <div className="stat-row">
                    <span className="stat-label">Min:</span>
                    <span className="stat-value">{stats.temperature.min.toFixed(1)}°C</span>
                  </div>
                  <div className="stat-row">
                    <span className="stat-label">Max:</span>
                    <span className="stat-value">{stats.temperature.max.toFixed(1)}°C</span>
                  </div>
                  <div className="stat-row">
                    <span className="stat-label">Avg:</span>
                    <span className="stat-value">{stats.temperature.avg.toFixed(1)}°C</span>
                  </div>
                </div>
                
                <div className="stat-card">
                  <h3>💧 Humidity</h3>
                  <div className="stat-row">
                    <span className="stat-label">Current:</span>
                    <span className="stat-value current">{stats.humidity.current?.toFixed(1)}%</span>
                  </div>
                  <div className="stat-row">
                    <span className="stat-label">Min:</span>
                    <span className="stat-value">{stats.humidity.min.toFixed(1)}%</span>
                  </div>
                  <div className="stat-row">
                    <span className="stat-label">Max:</span>
                    <span className="stat-value">{stats.humidity.max.toFixed(1)}%</span>
                  </div>
                  <div className="stat-row">
                    <span className="stat-label">Avg:</span>
                    <span className="stat-value">{stats.humidity.avg.toFixed(1)}%</span>
                  </div>
                </div>
                
                <div className="stat-card">
                  <h3>📊 Data Info</h3>
                  <div className="stat-row">
                    <span className="stat-label">Records:</span>
                    <span className="stat-value">{stats.count}</span>
                  </div>
                  <div className="stat-row">
                    <span className="stat-label">Device:</span>
                    <span className="stat-value">{selectedDevice === 'all' ? 'All' : selectedDevice}</span>
                  </div>
                  <div className="stat-row">
                    <span className="stat-label">Devices:</span>
                    <span className="stat-value">{devices.length}</span>
                  </div>
                  {stats.timeRange && (
                    <div className="stat-row">
                      <span className="stat-label">Range:</span>
                      <span className="stat-value time-range">
                        {stats.timeRange.start} - {stats.timeRange.end}
                      </span>
                    </div>
                  )}
                </div>
              </div>
            )}

            <div className="charts-container">
              {chartMode === 'combined' ? (
                <CombinedChart data={chartData} />
              ) : (
                <>
                  <InteractiveChart
                    data={chartData}
                    dataKey="temperature"
                    title="Temperature Chart"
                    color="#e74c3c"
                    unit="°C"
                    icon="🌡️"
                    onBrushChange={handleBrushChange}
                    brushData={brushData}
                  />
                  
                  <InteractiveChart
                    data={chartData}
                    dataKey="humidity"
                    title="Humidity Chart"
                    color="#3498db"
                    unit="%"
                    icon="💧"
                    onBrushChange={handleBrushChange}
                    brushData={brushData}
                  />
                </>
              )}
            </div>
          </>
        )}
      </main>

      <footer className="app-footer">
        <p>HT WebView - Part of WiFi_ATH21 Project</p>
      </footer>
    </div>
  )
}

export default App
