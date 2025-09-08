import { useState, useEffect } from 'react'

interface CSVFile {
  name: string
  path: string
  size: number
  modified: string
}

interface CSVFileBrowserProps {
  onFileSelect: (fileContent: string, fileName: string) => void
}

export const CSVFileBrowser = ({ onFileSelect }: CSVFileBrowserProps) => {
  const [files, setFiles] = useState<CSVFile[]>([])
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    loadFileList()
  }, [])

  const loadFileList = async () => {
    setLoading(true)
    setError(null)
    
    try {
      const response = await fetch('/api/csv-files')
      const data = await response.json()
      
      if (data.error) {
        setError(data.error)
      } else {
        setFiles(data.files)
      }
    } catch (err) {
      setError('Failed to load file list')
    } finally {
      setLoading(false)
    }
  }

  const handleFileClick = async (file: CSVFile) => {
    setLoading(true)
    setError(null)
    
    try {
      const response = await fetch(file.path)
      if (!response.ok) {
        throw new Error('Failed to load file')
      }
      
      const content = await response.text()
      onFileSelect(content, file.name)
    } catch (err) {
      setError(`Failed to load ${file.name}`)
    } finally {
      setLoading(false)
    }
  }

  const formatFileSize = (bytes: number) => {
    if (bytes === 0) return '0 B'
    const k = 1024
    const sizes = ['B', 'KB', 'MB', 'GB']
    const i = Math.floor(Math.log(bytes) / Math.log(k))
    return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i]
  }

  if (loading && files.length === 0) {
    return <div className="csv-browser loading">Loading files...</div>
  }

  return (
    <div className="csv-browser">
      <div className="csv-browser-header">
        <h3>📂 Server CSV Files</h3>
        <button onClick={loadFileList} disabled={loading}>
          🔄 Refresh
        </button>
      </div>
      
      {error && (
        <div className="csv-browser-error">
          ❌ {error}
        </div>
      )}
      
      {files.length === 0 ? (
        <div className="csv-browser-empty">
          No CSV files found on server
        </div>
      ) : (
        <div className="csv-file-list">
          {files.map((file) => (
            <div 
              key={file.name}
              className="csv-file-item"
              onClick={() => handleFileClick(file)}
            >
              <div className="file-name">📄 {file.name}</div>
              <div className="file-details">
                <span className="file-size">{formatFileSize(file.size)}</span>
                <span className="file-date">
                  {new Date(file.modified).toLocaleString('ru-RU')}
                </span>
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  )
}
