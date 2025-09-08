import { useState, useCallback, useRef } from 'react'
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer, Brush } from 'recharts'

interface CombinedChartProps {
  data: Array<{
    time: string
    shortTime: string
    fullDateTime: string
    temperature: number
    humidity: number
    index: number
    device: string
  }>
}

export const CombinedChart = ({ data }: CombinedChartProps) => {
  const [brushData, setBrushData] = useState<{ startIndex?: number; endIndex?: number }>({})
  const [brushKey, setBrushKey] = useState(0) // Ключ для принудительного обновления Brush
  const [forceUpdate, setForceUpdate] = useState(0) // Дополнительный триггер обновления
  const chartRef = useRef<any>(null)

  const handleBrushChange = useCallback((newBrushData: any) => {
    console.log('Brush change:', newBrushData) // Debug
    setBrushData({
      startIndex: newBrushData?.startIndex,
      endIndex: newBrushData?.endIndex
    })
  }, [])

  const handleResetZoom = () => {
    console.log('Reset zoom clicked') // Debug
    console.log('Current brushData:', brushData) // Debug
    
    // Сбрасываем состояние brush
    setBrushData({ startIndex: undefined, endIndex: undefined })
    
    // Принудительно обновляем ключ для пересоздания Brush
    setBrushKey(prev => {
      console.log('Updating brush key from', prev, 'to', prev + 1) // Debug
      return prev + 1
    })
    
    // Принудительно обновляем весь компонент
    setForceUpdate(prev => prev + 1)
    
    // Дополнительно попробуем через setTimeout для гарантии обновления
    setTimeout(() => {
      console.log('Delayed reset - final brushData should be empty') // Debug
    }, 100)
  }

  // Calculate dual Y-axis domains
  const temperatures = data.map(d => d.temperature)
  const humidities = data.map(d => d.humidity)
  
  const tempMin = Math.min(...temperatures)
  const tempMax = Math.max(...temperatures)
  const tempPadding = (tempMax - tempMin) * 0.1
  const tempDomain = [tempMin - tempPadding, tempMax + tempPadding]
  
  const humMin = Math.min(...humidities)
  const humMax = Math.max(...humidities)
  const humPadding = (humMax - humMin) * 0.1
  const humDomain = [humMin - humPadding, humMax + humPadding]

  const hasZoom = brushData.startIndex !== undefined || brushData.endIndex !== undefined

  const CustomTooltip = ({ active, payload, label }: any) => {
    console.log('Tooltip render:', { 
      active, 
      payloadLength: payload?.length, 
      label
    }) // Debug
    
    if (active && payload && payload.length > 0) {
      console.log('Payload data:', payload) // Debug
      
      // Получаем данные НАПРЯМУЮ из payload - это правильные данные для текущей позиции
      const firstPayload = payload[0]
      const payloadData = firstPayload?.payload
      
      if (payloadData && payloadData.fullDateTime) {
        console.log('Using payload data directly (correct approach):', payloadData) // Debug
        
        // Получаем значения из payload для каждой линии
        const tempPayload = payload.find((p: any) => p.dataKey === 'temperature')
        const humPayload = payload.find((p: any) => p.dataKey === 'humidity')
        
        return (
          <div className="combined-tooltip">
            <p className="tooltip-label">
              📅 {payloadData.fullDateTime}
            </p>
            {tempPayload && (
              <p style={{ color: '#e74c3c', margin: '4px 0' }}>
                🌡️ Temperature: {tempPayload.value?.toFixed(2)}°C
              </p>
            )}
            {humPayload && (
              <p style={{ color: '#3498db', margin: '4px 0' }}>
                💧 Humidity: {humPayload.value?.toFixed(2)}%
              </p>
            )}
          </div>
        )
      } else {
        console.log('No payload data available') // Debug
        return null
      }
    }
    return null
  }

  return (
    <div className="chart-section combined-chart">
      <div className="chart-header">
        <h3>📊 Combined Temperature & Humidity Chart</h3>
        <div className="chart-controls">
          {hasZoom && (
            <button 
              onClick={handleResetZoom}
              className="reset-zoom-btn"
              title="Reset zoom"
            >
              🔍 Reset Zoom
            </button>
          )}
        </div>
      </div>
      
      <ResponsiveContainer width="100%" height={500}>
        <LineChart 
          key={`chart-${brushKey}-${forceUpdate}`}
          ref={chartRef}
          data={data}
          margin={{ top: 20, right: 30, left: 20, bottom: 80 }}
          syncId="anyId"
        >
          <CartesianGrid 
            strokeDasharray="3 3" 
            stroke="#e0e0e0" 
            opacity={0.5} 
          />
          <XAxis 
            dataKey="shortTime" 
            angle={-45}
            textAnchor="end"
            height={80}
            fontSize={12}
            stroke="#666"
            interval="preserveStartEnd"
          />
          <YAxis 
            yAxisId="temp"
            orientation="left"
            label={{ value: 'Temperature (°C)', angle: -90, position: 'insideLeft' }}
            fontSize={12}
            stroke="#e74c3c"
            domain={tempDomain}
            tickFormatter={(value) => value.toFixed(1)}
          />
          <YAxis 
            yAxisId="hum"
            orientation="right"
            label={{ value: 'Humidity (%)', angle: 90, position: 'insideRight' }}
            fontSize={12}
            stroke="#3498db"
            domain={humDomain}
            tickFormatter={(value) => value.toFixed(1)}
          />
          <Tooltip 
            content={<CustomTooltip />}
            isAnimationActive={false}
            allowEscapeViewBox={{ x: false, y: false }}
            wrapperStyle={{ outline: 'none' }}
          />
          <Legend 
            wrapperStyle={{
              paddingTop: '20px',
              fontSize: '14px'
            }}
          />
          <Line 
            yAxisId="temp"
            type="linear" 
            dataKey="temperature" 
            stroke="#e74c3c" 
            strokeWidth={1.5}
            dot={false}
            name="Temperature"
            activeDot={false}
            connectNulls={true}
            isAnimationActive={true}
            animationDuration={1000}
            animationEasing="ease-in-out"
          />
          <Line 
            yAxisId="hum"
            type="linear" 
            dataKey="humidity" 
            stroke="#3498db" 
            strokeWidth={1.5}
            dot={false}
            name="Humidity"
            activeDot={false}
            connectNulls={true}
            isAnimationActive={true}
            animationDuration={1000}
            animationEasing="ease-in-out"
          />
          <Brush 
            key={brushKey} 
            dataKey="shortTime" 
            height={40} 
            stroke="#34495e"
            fill="#34495e"
            fillOpacity={0.15}
            onChange={handleBrushChange}
            startIndex={brushData.startIndex}
            endIndex={brushData.endIndex}
            tickFormatter={(value, index) => {
              // Show every 10th tick to avoid crowding, показываем только время без даты
              if (index % 10 === 0) {
                // Извлекаем только время из формата "06.09 08:10"
                const timePart = value.toString().split(' ')[1] || value.toString()
                return timePart
              }
              return ''
            }}
            travellerWidth={12}
          />
        </LineChart>
      </ResponsiveContainer>
      
      <div className="chart-info">
        <span className="data-points">
          📊 {data.length} data points
          {hasZoom && brushData.startIndex !== undefined && brushData.endIndex !== undefined &&
            ` (showing ${brushData.endIndex - brushData.startIndex + 1})`
          }
        </span>
      </div>
    </div>
  )
}
