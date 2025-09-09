const express = require('express');
const fs = require('fs');
const path = require('path');
const cors = require('cors');

const app = express();
const PORT = 3001; // Используем другой порт для API

// Enable CORS for development
app.use(cors());

const CSV_DIR = process.env.CSV_DIR || './csv-data';

console.log('API Server starting...');
console.log('CSV directory:', CSV_DIR);

// API endpoint to list CSV files
app.get('/api/csv-files', (req, res) => {
  console.log('API: Getting CSV files from:', CSV_DIR);
  
  if (!fs.existsSync(CSV_DIR)) {
    console.log('CSV directory does not exist:', CSV_DIR);
    return res.json([]);
  }

  try {
    const files = fs.readdirSync(CSV_DIR)
      .filter(file => file.endsWith('.csv'))
      .map(file => {
        const filePath = path.join(CSV_DIR, file);
        const stats = fs.statSync(filePath);
        return {
          name: file,
          size: stats.size,
          modified: stats.mtime
        };
      })
      .sort((a, b) => b.modified - a.modified);
    
    console.log(`Found ${files.length} CSV files`);
    res.json(files);
  } catch (error) {
    console.error('Error reading CSV directory:', error);
    res.status(500).json({ error: 'Failed to read CSV files' });
  }
});

// API endpoint to serve a specific CSV file
app.get('/api/csv/:filename', (req, res) => {
  const filename = req.params.filename;
  const filePath = path.join(CSV_DIR, filename);
  
  console.log('API: Serving CSV file:', filename);
  
  if (!fs.existsSync(filePath) || !filename.endsWith('.csv')) {
    return res.status(404).json({ error: 'File not found' });
  }

  res.setHeader('Content-Type', 'text/csv');
  res.sendFile(path.resolve(filePath));
});

app.listen(PORT, () => {
  console.log(`API Server running on http://localhost:${PORT}`);
  console.log('CSV directory:', CSV_DIR);
});
