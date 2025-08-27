# выполнить под sudo
sudo useradd --system --no-create-home --shell /usr/sbin/nologin mqttlogger

# директории
sudo mkdir -p /opt/mqtt_logger
sudo mkdir -p /var/lib/mqtt_logger/data
sudo mkdir -p /var/log/mqtt_logger

# копируем бинарь и config.ini (предположим вы в сборочном каталоге)
sudo cp ./mqtt_logger /opt/mqtt_logger/
sudo cp ./config.ini /opt/mqtt_logger/

# права
sudo chown -R mqttlogger:mqttlogger /opt/mqtt_logger /var/lib/mqtt_logger /var/log/mqtt_logger
sudo chmod +x /opt/mqtt_logger/mqtt_logger