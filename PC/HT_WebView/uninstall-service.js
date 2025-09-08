const Service = require('node-windows').Service;
const path = require('path');

// Create a new service object
const svc = new Service({
  name: 'HT WebView Server',
  script: path.join(__dirname, 'deploy-server.js')
});

// Listen for the "uninstall" event
svc.on('uninstall', () => {
  console.log('Service uninstalled successfully!');
});

// Uninstall the service
console.log('Uninstalling HT WebView Server...');
svc.uninstall();
