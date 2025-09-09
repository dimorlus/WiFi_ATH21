import { useState, useCallback, useRef, useEffect } from 'react'

interface UseChartInteractionProps {
  data: any[]
  onBrushChange?: (brushData: any) => void
}

export const useChartInteraction = ({ data, onBrushChange }: UseChartInteractionProps) => {
  const [zoomRange, setZoomRange] = useState<{ start: number; end: number }>({ 
    start: 0, 
    end: data.length - 1 
  })
  const [isDragging, setIsDragging] = useState(false)
  const [dragStart, setDragStart] = useState<{ x: number; range: { start: number; end: number } } | null>(null)
  const chartRef = useRef<HTMLDivElement>(null)

  // Update zoom range when data changes
  useEffect(() => {
    if (data.length > 0) {
      setZoomRange({ start: 0, end: data.length - 1 })
    }
  }, [data.length])

  const handleWheel = useCallback((event: WheelEvent) => {
    if (!chartRef.current || data.length === 0) return
    
    // Проверяем, что мышь в области графика (не в области Brush)
    const rect = chartRef.current.getBoundingClientRect()
    const mouseX = event.clientX - rect.left
    const mouseY = event.clientY - rect.top
    const chartWidth = rect.width
    const chartHeight = rect.height
    
    // Если мышь в области Brush - не обрабатываем
    if (mouseY > chartHeight - 120) {
      return
    }
    
    // Если мышь вне области графика - не обрабатываем
    if (mouseX < 60 || mouseX > chartWidth - 40 || mouseY < 5) {
      return
    }
    
    event.preventDefault()
    
    // Вычисляем позицию мыши относительно графика (0-1)
    const mousePosition = Math.max(0, Math.min(1, mouseX / chartWidth))
    
    // Текущий диапазон
    const currentStart = zoomRange.start
    const currentEnd = zoomRange.end
    const currentRange = currentEnd - currentStart
    
    // Фактор зума (отрицательное значение deltaY = zoom in)
    const zoomFactor = event.deltaY > 0 ? 1.2 : 0.8
    const newRange = Math.max(10, Math.min(data.length, currentRange * zoomFactor))
    
    // Вычисляем новые границы относительно позиции мыши
    const mouseDataPosition = currentStart + (currentRange * mousePosition)
    const newStart = Math.max(0, mouseDataPosition - (newRange * mousePosition))
    const newEnd = Math.min(data.length - 1, newStart + newRange)
    
    const adjustedStart = Math.max(0, newEnd - newRange)
    
    const newZoomRange = {
      start: Math.round(adjustedStart),
      end: Math.round(newEnd)
    }
    
    setZoomRange(newZoomRange)
    
    // Уведомляем родительский компонент
    if (onBrushChange) {
      onBrushChange({
        startIndex: newZoomRange.start,
        endIndex: newZoomRange.end
      })
    }
  }, [data.length, zoomRange, onBrushChange])

  const handleMouseDown = useCallback((event: MouseEvent) => {
    if (!chartRef.current) return
    
    // Проверяем, что мышь в области графика (не в области Brush)
    const rect = chartRef.current.getBoundingClientRect()
    const mouseX = event.clientX - rect.left
    const mouseY = event.clientY - rect.top
    const chartWidth = rect.width
    const chartHeight = rect.height
    
    // Если мышь в области Brush - не обрабатываем
    if (mouseY > chartHeight - 120) {
      return
    }
    
    // Если мышь вне области графика - не обрабатываем
    if (mouseX < 60 || mouseX > chartWidth - 40 || mouseY < 5) {
      return
    }
    
    // Предотвращаем выделение текста
    event.preventDefault()
    event.stopPropagation()
    
    setIsDragging(true)
    setDragStart({
      x: event.clientX,
      range: { ...zoomRange }
    })
    
    // Меняем курсор и отключаем выделение текста
    document.body.style.cursor = 'grabbing'
    document.body.style.userSelect = 'none'
  }, [zoomRange])

  const handleMouseMove = useCallback((event: MouseEvent) => {
    if (!isDragging || !dragStart || !chartRef.current) return

    const deltaX = event.clientX - dragStart.x
    const rect = chartRef.current.getBoundingClientRect()
    const chartWidth = rect.width
    
    // Вычисляем смещение в единицах данных
    const dataRange = dragStart.range.end - dragStart.range.start
    const pixelsPerDataPoint = chartWidth / dataRange
    const dataOffset = -deltaX / pixelsPerDataPoint
    
    // Новые границы с учетом смещения
    let newStart = dragStart.range.start + dataOffset
    let newEnd = dragStart.range.end + dataOffset
    
    // Ограничиваем границы
    if (newStart < 0) {
      newEnd = newEnd - newStart
      newStart = 0
    }
    if (newEnd >= data.length) {
      newStart = newStart - (newEnd - (data.length - 1))
      newEnd = data.length - 1
    }
    
    const newZoomRange = {
      start: Math.max(0, Math.round(newStart)),
      end: Math.min(data.length - 1, Math.round(newEnd))
    }
    
    setZoomRange(newZoomRange)
    
    // Уведомляем родительский компонент
    if (onBrushChange) {
      onBrushChange({
        startIndex: newZoomRange.start,
        endIndex: newZoomRange.end
      })
    }
  }, [isDragging, dragStart, data.length, onBrushChange])

  const handleMouseUp = useCallback(() => {
    setIsDragging(false)
    setDragStart(null)
    document.body.style.cursor = 'default'
    document.body.style.userSelect = 'auto'
  }, [])

  // Подключаем обработчики событий
  useEffect(() => {
    const chartElement = chartRef.current
    if (!chartElement) return

    // Слушаем события на всем контейнере, но проверяем область в обработчиках
    chartElement.addEventListener('wheel', handleWheel, { passive: false })
    chartElement.addEventListener('mousedown', handleMouseDown)
    document.addEventListener('mousemove', handleMouseMove)
    document.addEventListener('mouseup', handleMouseUp)
    
    return () => {
      chartElement.removeEventListener('wheel', handleWheel)
      chartElement.removeEventListener('mousedown', handleMouseDown)
      document.removeEventListener('mousemove', handleMouseMove)
      document.removeEventListener('mouseup', handleMouseUp)
    }
  }, [handleWheel, handleMouseDown, handleMouseMove, handleMouseUp])

  const resetZoom = useCallback(() => {
    const fullRange = { start: 0, end: data.length - 1 }
    setZoomRange(fullRange)
    
    if (onBrushChange) {
      onBrushChange({
        startIndex: undefined,
        endIndex: undefined
      })
    }
  }, [data.length, onBrushChange])

  return {
    chartRef,
    zoomRange,
    resetZoom,
    isDragging,
    setZoomRange // Экспортируем для внешнего управления
  }
}
