const Service = require('node-windows').Service;
const path = require('path');

// Create a new service object
const svc = new Service({
  name: 'HT WebView Server',
  description: 'Temperature & Humidity Data Visualizer Web Server',
  script: path.join(__dirname, 'deploy-server.js'),
  env: [
    {
      name: "PORT",
      value: "3000"
    },
    {
      name: "CSV_DIR", 
      value: "c:\\mqtt_logger\\LOG" // Путь к папке с CSV файлами
    }
  ]
});

// Listen for the "install" event, which indicates the process is available as a service.
svc.on('install', () => {
  console.log('Service installed successfully!');
  console.log('Starting service...');
  svc.start();
});

svc.on('start', () => {
  console.log('Service started successfully!');
  console.log('HT WebView Server is now running as Windows Service');
  console.log('Access at: http://localhost:3000');
});

// Install the script as a service.
console.log('Installing HT WebView Server as Windows Service...');
svc.install();
