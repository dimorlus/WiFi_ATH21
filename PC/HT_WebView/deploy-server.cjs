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
  
  if (!fs.existsSync(csvDir)) {
    return res.json({ files: [], error: 'CSV directory not found' });
  }
  
  try {
    const files = fs.readdirSync(csvDir)
      .filter(file => file.endsWith('.csv'))
      .map(file => ({
        name: file,
        path: `/api/csv/${file}`,
        size: fs.statSync(path.join(csvDir, file)).size,
        modified: fs.statSync(path.join(csvDir, file)).mtime
      }));
    
    res.json({ files });
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

// Serve index.html for all other routes (SPA)
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'dist', 'index.html'));
});

app.listen(PORT, '0.0.0.0', () => {
  console.log(`HT WebView Server running on http://localhost:${PORT}`);
  console.log(`CSV directory: ${process.env.CSV_DIR || './csv-data'}`);
});
