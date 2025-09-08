import { useState, useCallback, useRef } from 'react'
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer, Brush, ReferenceLine } from 'recharts'

interface InteractiveChartProps {
  data: Array<{
    time: string
    shortTime: string
    fullDateTime: string
    temperature?: number
    humidity?: number
    index: number
    device: string
  }>
  dataKey: 'temperature' | 'humidity'
  title: string
  color: string
  unit: string
  icon: string
  onBrushChange?: (brushData: any) => void
  brushData?: { startIndex?: number; endIndex?: number }
}

export const InteractiveChart = ({ 
  data, 
  dataKey, 
  title, 
  color, 
  unit, 
  icon,
  onBrushChange,
  brushData
}: InteractiveChartProps) => {
  const [brushKey, setBrushKey] = useState(0) // Ключ для принудительного обновления Brush
  const [forceUpdate, setForceUpdate] = useState(0) // Дополнительный триггер обновления
  const chartRef = useRef<any>(null)

  const handleBrushChange = useCallback((newBrushData: any) => {
    if (onBrushChange) {
      onBrushChange(newBrushData)
    }
  }, [onBrushChange])

  const handleResetZoom = () => {
    console.log('InteractiveChart: Reset zoom clicked') // Debug
    setBrushKey(prev => prev + 1) // Принудительно обновляем Brush компонент
    setForceUpdate(prev => prev + 1) // Принудительно обновляем весь компонент
    if (onBrushChange) {
      onBrushChange({ startIndex: undefined, endIndex: undefined })
    }
  }

  // Get data range for Y-axis
  const values = data.map(d => d[dataKey]).filter(v => v !== undefined) as number[]
  const minValue = Math.min(...values)
  const maxValue = Math.max(...values)
  const padding = (maxValue - minValue) * 0.1
  const yAxisDomain = [minValue - padding, maxValue + padding]

  const hasZoom = brushData && (brushData.startIndex !== undefined || brushData.endIndex !== undefined)

  const CustomTooltip = ({ active, payload, label }: any) => {
    console.log('InteractiveChart Tooltip:', { 
      active, 
      payloadLength: payload?.length, 
      label, 
      dataKey
    }) // Debug
    
    if (active && payload && payload.length > 0) {
      console.log('InteractiveChart Payload data:', payload) // Debug
      
      // Получаем данные НАПРЯМУЮ из payload - это правильные данные для текущей позиции
      const firstPayload = payload[0]
      const payloadData = firstPayload?.payload
      
      if (payloadData && payloadData.fullDateTime && firstPayload.value !== undefined) {
        console.log('InteractiveChart Using payload data directly (correct approach):', payloadData) // Debug
        
        return (
          <div className="interactive-tooltip" style={{
            backgroundColor: 'rgba(255, 255, 255, 0.98)',
            border: `2px solid ${color}`,
            borderRadius: '12px',
            boxShadow: '0 8px 24px rgba(0, 0, 0, 0.15)',
            fontSize: '14px',
            fontWeight: '500',
            padding: '12px'
          }}>
            <p style={{
              color: '#2c3e50',
              fontWeight: '600',
              margin: '0 0 8px 0'
            }}>
              📅 {payloadData.fullDateTime}
            </p>
            <p style={{ color: color, margin: '0' }}>
              {icon} {title.split(' ')[0]}: {firstPayload.value?.toFixed(2)}{unit}
            </p>
          </div>
        )
      } else {
        console.log('InteractiveChart No payload data available') // Debug
        return null
      }
    }
    return null
  }

  return (
    <div className="chart-section">
      <div className="chart-header">
        <h3>{icon} {title}</h3>
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
      
      <ResponsiveContainer width="100%" height={400}>
        <LineChart 
          key={`interactive-chart-${brushKey}-${forceUpdate}`}
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
            label={{ value: `${title.split(' ')[0]} (${unit})`, angle: -90, position: 'insideLeft' }}
            fontSize={12}
            stroke="#666"
            domain={yAxisDomain}
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
            type="linear"  // Вместо "monotone" - более предсказуемая интерполяция
            dataKey={dataKey} 
            stroke={color} 
            strokeWidth={1.5}
            dot={false}
            activeDot={false}
            connectNulls={true}  // Соединяет точки через пропуски
            isAnimationActive={true}
            animationDuration={1000}
            animationEasing="ease-in-out"
          />
          <Brush 
            key={brushKey}
            dataKey="shortTime" 
            height={40} 
            stroke={color}
            fill={color}
            fillOpacity={0.15}
            onChange={handleBrushChange}
            startIndex={brushData?.startIndex}
            endIndex={brushData?.endIndex}
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
          {hasZoom && brushData?.startIndex !== undefined && brushData?.endIndex !== undefined &&
            ` (showing ${brushData.endIndex - brushData.startIndex + 1})`
          }
        </span>
      </div>
    </div>
  )
}
