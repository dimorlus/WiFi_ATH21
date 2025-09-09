import { useEffect, useCallback, RefObject } from 'react'

interface ZoomData {
  startIndex?: number
  endIndex?: number
}

interface UseWheelZoomProps {
  chartRef: RefObject<HTMLDivElement>
  data: any[]
  onBrushChange: (data: ZoomData) => void
  brushData: ZoomData
}

export const useWheelZoom = ({ chartRef, data, onBrushChange, brushData }: UseWheelZoomProps) => {
  
  const handleWheel = useCallback((event: WheelEvent) => {
    if (!chartRef.current || data.length === 0) return
    
    event.preventDefault()
    event.stopPropagation()
    
    const rect = chartRef.current.getBoundingClientRect()
    const mouseX = event.clientX - rect.left
    const chartWidth = rect.width
    
    // Текущий диапазон данных
    const currentStart = brushData.startIndex ?? 0
    const currentEnd = brushData.endIndex ?? data.length - 1
    const currentRange = currentEnd - currentStart
    
    // Позиция мыши относительно диапазона (от 0 до 1)
    const mousePosition = Math.max(0, Math.min(1, (mouseX - 60) / (chartWidth - 100)))
    
    // Фактор зума (отрицательное значение deltaY = zoom in) - сделаем менее чувствительным
    const zoomFactor = event.deltaY > 0 ? 1.1 : 0.9
    const newRange = Math.max(5, Math.min(data.length, currentRange * zoomFactor))
    
    // Вычисляем новые границы относительно позиции мыши
    const mouseDataPosition = currentStart + (currentRange * mousePosition)
    const newStart = Math.max(0, mouseDataPosition - (newRange * mousePosition))
    const newEnd = Math.min(data.length - 1, newStart + newRange)
    
    const adjustedStart = Math.max(0, newEnd - newRange)
    
    const newZoomData = {
      startIndex: Math.round(adjustedStart),
      endIndex: Math.round(newEnd)
    }
    
    // Уведомляем родительский компонент
    onBrushChange(newZoomData)
  }, [data.length, brushData, onBrushChange])

  // Подключаем обработчики событий только для wheel
  useEffect(() => {
    const chartElement = chartRef.current
    if (!chartElement) return

    // Слушаем wheel на всем контейнере, но проверяем область
    const handleWheelWithBounds = (event: WheelEvent) => {
      const rect = chartElement.getBoundingClientRect()
      const mouseX = event.clientX - rect.left
      const mouseY = event.clientY - rect.top
      
      // Проверяем, что мышь в области графика (не в области Brush)
      const chartWidth = rect.width
      const chartHeight = rect.height
      
      if (mouseX >= 80 && mouseX <= chartWidth - 60 && 
          mouseY >= 10 && mouseY <= chartHeight - 100) {
        handleWheel(event)
      }
    }

    chartElement.addEventListener('wheel', handleWheelWithBounds, { passive: false })

    return () => {
      chartElement.removeEventListener('wheel', handleWheelWithBounds)
    }
  }, [handleWheel])

  return {
    // Возвращаем только isDragging: false для совместимости
    isDragging: false
  }
}
