const express = require('express');
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = process.env.PORT || 3000;

// Serve static files from dist directory
app.use(express.static(path.join(__dirname, 'dist')));

// API endpoint for listing CSV files from local directory
app.get('/api/csv-files', (req, res) => {
  const csvDir = process.env.CSV_DIR || './csv-data';
  
  console.log('API: Getting CSV files from:', csvDir);
  
  if (!fs.existsSync(csvDir)) {
    console.log('CSV directory does not exist:', csvDir);
    return res.json([]);
  }
  
  try {
    const files = fs.readdirSync(csvDir)
      .filter(file => file.endsWith('.csv'))
      .map(file => {
        const filePath = path.join(csvDir, file);
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
    res.status(500).json({ error: error.message });
  }
});

// API endpoint for serving CSV files
app.get('/api/csv/:filename', (req, res) => {
  const csvDir = process.env.CSV_DIR || './csv-data';
  const filename = req.params.filename;
  const filePath = path.join(csvDir, filename);
  
  if (!fs.existsSync(filePath) || !filename.endsWith('.csv')) {
    return res.status(404).json({ error: 'File not found' });
  }
  
  res.setHeader('Content-Type', 'text/csv');
  res.setHeader('Content-Disposition', `attachment; filename="${filename}"`);
  res.sendFile(path.resolve(filePath));
});

// Serve index.html for root and unknown routes (SPA)
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'dist', 'index.html'));
});

// Catch-all fallback for SPA
app.use((req, res) => {
  res.sendFile(path.join(__dirname, 'dist', 'index.html'));
});

app.listen(PORT, '0.0.0.0', () => {
  console.log(`HT WebView Server running on http://localhost:${PORT}`);
  console.log(`CSV directory: ${process.env.CSV_DIR || './csv-data'}`);
});
