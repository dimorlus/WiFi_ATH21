// Combined Chart with interactive controls + Brush navigation
import { useState, useCallback } from 'react'
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer, Brush } from 'recharts'
import { useChartInteraction } from '../hooks/useChartInteraction'

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
  fileName?: string
}

export const CombinedChart = ({ data, fileName }: CombinedChartProps) => {
  // Состояние для координации между interactive и brush
  const [brushKey, setBrushKey] = useState(0)
  
  // Interactive управление - основной источник зума
  const { chartRef: interactionRef, zoomRange, resetZoom: resetInteractiveZoom, isDragging } = useChartInteraction({
    data
  })
  
  // Обработчик Brush - обновляем interactive zoom, но только от Brush
  const handleBrushChange = useCallback((brushData: any) => {
    console.log('CombinedChart: Brush change:', brushData) // Debug
    // Когда пользователь перетаскивает Brush, обновляем только отображение
    // НЕ вызываем setZoomRange чтобы избежать циклов
  }, [])
  
  const hasZoom = zoomRange.start !== 0 || zoomRange.end !== data.length - 1

  // Используем ВСЕ данные для LineChart (как в InteractiveChart)
  // Brush будет управлять отображением автоматически
  const safeDisplayData = data.length >= 2 ? data : data.slice(0, Math.min(2, data.length))
  
  // Рассчитываем размер отображаемого диапазона для статистики
  const displayedCount = hasZoom ? zoomRange.end - zoomRange.start + 1 : data.length

  const handleResetZoom = () => {
    console.log('CombinedChart: Reset zoom clicked') // Debug
    
    // Сбрасываем интерактивный зум
    resetInteractiveZoom()
    
    // Принудительно обновляем Brush компонент
    setBrushKey(prev => prev + 1)
  }
  
  console.log('Display data info:', {
    totalData: data.length,
    hasZoom,
    zoomStart: zoomRange.start,
    zoomEnd: zoomRange.end,
    safeDisplayCount: safeDisplayData.length,
    firstItem: safeDisplayData[0],
    lastItem: safeDisplayData[safeDisplayData.length - 1]
  }) // Debug

  // Calculate dual Y-axis domains based on zoom range (if zoomed) or all data
  const domainData = hasZoom ? data.slice(zoomRange.start, zoomRange.end + 1) : data
  const temperatures = domainData.map((d: any) => d.temperature).filter((t: any) => !isNaN(t) && t !== null && isFinite(t))
  const humidities = domainData.map((d: any) => d.humidity).filter((h: any) => !isNaN(h) && h !== null && isFinite(h))
  
  console.log('Domain calculation:', {
    safeDataLength: safeDisplayData.length,
    validTemperatures: temperatures.length,
    validHumidities: humidities.length,
    tempSample: temperatures.slice(0, 5),
    humSample: humidities.slice(0, 5)
  }) // Debug
  
  // Более надежные fallback domains
  let tempDomain: [number, number] = [-10, 50] // Типичный диапазон температур
  let humDomain: [number, number] = [0, 100]   // Типичный диапазон влажности
  
  if (temperatures.length > 0) {
    const tempMin = Math.min(...temperatures)
    const tempMax = Math.max(...temperatures)
    const tempRange = tempMax - tempMin
    
    if (tempRange === 0) {
      // Все значения одинаковые - создаем симметричный диапазон
      const padding = Math.max(2, Math.abs(tempMin) * 0.1)
      tempDomain = [tempMin - padding, tempMax + padding]
    } else if (tempRange < 0.1) {
      // Очень малый диапазон - увеличиваем
      const center = (tempMin + tempMax) / 2
      tempDomain = [center - 1, center + 1]
    } else {
      // Нормальный диапазон
      const padding = Math.max(0.5, tempRange * 0.1)
      tempDomain = [tempMin - padding, tempMax + padding]
    }
  }
  
  if (humidities.length > 0) {
    const humMin = Math.min(...humidities)
    const humMax = Math.max(...humidities)
    const humRange = humMax - humMin
    
    if (humRange === 0) {
      // Все значения одинаковые
      const padding = Math.max(2, Math.abs(humMin) * 0.1)
      humDomain = [Math.max(0, humMin - padding), Math.min(100, humMax + padding)]
    } else if (humRange < 0.1) {
      // Очень малый диапазон
      const center = (humMin + humMax) / 2
      humDomain = [Math.max(0, center - 2), Math.min(100, center + 2)]
    } else {
      // Нормальный диапазон
      const padding = Math.max(1, humRange * 0.1)
      humDomain = [Math.max(0, humMin - padding), Math.min(100, humMax + padding)]
    }
  }

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
        <h3>📊 Combined Temperature & Humidity Chart{fileName ? ` - ${fileName}` : ''}</h3>
        <div className="chart-controls">
          <div className="chart-status">
            {isDragging && <span className="drag-indicator">🤏 Dragging...</span>}
            {hasZoom && (
              <span className="zoom-info">
                🔍 Showing: {zoomRange.start}-{zoomRange.end} ({zoomRange.end - zoomRange.start + 1} points)
              </span>
            )}
            <span className="interactive-zoom-info">
              🎯 Interactive: {zoomRange.start}-{zoomRange.end} ({zoomRange.end - zoomRange.start + 1} points)
            </span>
          </div>
          {hasZoom && (
            <button 
              onClick={handleResetZoom}
              className="reset-zoom-btn"
              title="Reset zoom (or scroll to zoom, drag to pan)"
            >
              🔍 Reset Zoom
            </button>
          )}
        </div>
      </div>
      
      <div className="chart-interaction-hint">
        💡 Use mouse wheel to zoom, drag to pan in chart area, or drag brush below to navigate
      </div>
      
      <div 
        className="chart-container" 
        ref={interactionRef} 
        style={{ 
          position: 'relative',
          userSelect: 'none',
          WebkitUserSelect: 'none',
          MozUserSelect: 'none',
          msUserSelect: 'none'
        }}
      >
        <ResponsiveContainer width="100%" height={500}>
          <LineChart
            key={`combined-chart-${brushKey}`}
            data={safeDisplayData}
            margin={{ top: 20, right: 30, left: 20, bottom: 80 }}
            syncId="anyId"
            style={{ cursor: isDragging ? 'grabbing' : 'grab' }}
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
            height={50} 
            stroke="#34495e"
            fill="#34495e"
            fillOpacity={0.15}
            onChange={handleBrushChange}
            startIndex={hasZoom ? zoomRange.start : undefined}
            endIndex={hasZoom ? zoomRange.end : undefined}
            tickFormatter={(value, index) => {
              // Show every 10th tick to avoid crowding
              if (index % 10 === 0) {
                const timePart = value.toString().split(' ')[1] || value.toString()
                return timePart
              }
              return ''
            }}
            travellerWidth={12}
          />
        </LineChart>
        </ResponsiveContainer>
      </div>
      
      <div className="chart-info">
        <span className="data-points">
          📊 {data.length} data points
          {hasZoom && ` (showing ${displayedCount})`}
        </span>
      </div>
    </div>
  )
}
