#!/bin/bash
set -e

# путь с билдом (измените под вашу структуру)
BUILD_DIR="/path/to/project/build"   # <- замените на /home/you/... или /mnt/d/...
INSTALL_DIR="/opt/mqtt_logger"
SERVICE_NAME="mqtt_logger"

# создаём системного пользователя (если уже есть — пропускаем)
if ! id -u mqttlogger >/dev/null 2>&1; then
  sudo useradd --system --no-create-home --shell /usr/sbin/nologin mqttlogger
fi

sudo mkdir -p "$INSTALL_DIR"
sudo cp "$BUILD_DIR/mqtt_logger" "$INSTALL_DIR/"
sudo cp "$BUILD_DIR/config.ini" "$INSTALL_DIR/"
sudo chown -R mqttlogger:mqttlogger "$INSTALL_DIR"
sudo chmod +x "$INSTALL_DIR/mqtt_logger"

# создаём unit (перезапишет если уже есть)
sudo tee /etc/systemd/system/${SERVICE_NAME}.service > /dev/null <<'EOF'
[Unit]
Description=MQTT logger service
After=network.target

[Service]
Type=simple
User=mqttlogger
Group=mqttlogger
WorkingDirectory=${INSTALL_DIR}
ExecStart=${INSTALL_DIR}/mqtt_logger
Restart=on-failure
RestartSec=5
StartLimitBurst=5
StartLimitIntervalSec=60
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable --now ${SERVICE_NAME}
echo "Service ${SERVICE_NAME} installed and started."
echo "Follow logs: sudo journalctl -u ${SERVICE_NAME} -f"