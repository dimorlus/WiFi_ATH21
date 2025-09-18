#!/usr/bin/env python3
"""
ESP8266 OTA Update Script
Автоматизирует процесс OTA обновления прошивки ESP8266:
1. Запускает HTTP сервер для раздачи файлов user1.bin/user2.bin
2. Подключается к ESP через telnet и отправляет команды OTA и FOTA
"""

import argparse
import socket
import time
import threading
import os
import sys
import datetime
from http.server import HTTPServer, SimpleHTTPRequestHandler
from socketserver import ThreadingMixIn


class ThreadingHTTPServer(ThreadingMixIn, HTTPServer):
    """HTTP сервер с поддержкой многопоточности"""
    daemon_threads = True


def get_file_info(filepath):
    """Получает информацию о файле: размер, дату модификации"""
    if not os.path.exists(filepath):
        return None
    
    stat = os.stat(filepath)
    size = stat.st_size
    mtime = datetime.datetime.fromtimestamp(stat.st_mtime)
    
    # Форматируем размер в удобочитаемый вид
    if size < 1024:
        size_str = f"{size} B"
    elif size < 1024 * 1024:
        size_str = f"{size / 1024:.1f} KB"
    else:
        size_str = f"{size / (1024 * 1024):.1f} MB"
    
    return {
        'size': size,
        'size_str': size_str,
        'mtime': mtime,
        'mtime_str': mtime.strftime("%Y-%m-%d %H:%M:%S")
    }


def print_firmware_info(firmware_dir):
    """Выводит информацию о доступных файлах прошивки"""
    print("\n" + "="*60)
    print("ИНФОРМАЦИЯ О ФАЙЛАХ ПРОШИВКИ")
    print("="*60)
    
    user1_path = os.path.join(firmware_dir, 'user1.bin')
    user2_path = os.path.join(firmware_dir, 'user2.bin')
    
    for filename in ['user1.bin', 'user2.bin']:
        filepath = os.path.join(firmware_dir, filename)
        info = get_file_info(filepath)
        
        if info:
            print(f"📁 {filename}:")
            print(f"   Путь: {os.path.abspath(filepath)}")
            print(f"   Размер: {info['size_str']} ({info['size']:,} байт)")
            print(f"   Изменен: {info['mtime_str']}")
        else:
            print(f"❌ {filename}: файл не найден")
        print()
    
    print("="*60)


def get_local_ip():
    """Определяет локальный IP адрес компьютера"""
    try:
        # Подключаемся к внешнему адресу чтобы определить локальный IP
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.connect(("8.8.8.8", 80))
            return s.getsockname()[0]
    except Exception:
        return "127.0.0.1"


def send_telnet_command(host, port, command, timeout=5):
    """Отправляет команду через telnet и возвращает ответ"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(timeout)
        sock.connect((host, port))
        
        # Ждем приглашение "Telnet setup"
        welcome = sock.recv(1024).decode('utf-8', errors='ignore')
        print(f"ESP ответ: {welcome.strip()}")
        
        # Отправляем команду
        command_line = command + "\r\n"
        sock.send(command_line.encode('utf-8'))
        print(f"Отправлено: {command}")
        
        # Получаем ответ
        time.sleep(0.5)
        response = sock.recv(1024).decode('utf-8', errors='ignore')
        print(f"ESP ответ: {response.strip()}")
        
        sock.close()
        return response.strip()
        
    except Exception as e:
        print(f"Ошибка telnet соединения: {e}")
        return None


def start_http_server(directory, port):
    """Запускает HTTP сервер в отдельном потоке"""
    def server_thread():
        os.chdir(directory)
        httpd = ThreadingHTTPServer(("0.0.0.0", port), SimpleHTTPRequestHandler)
        print(f"HTTP сервер запущен на порту {port}, каталог: {directory}")
        httpd.serve_forever()
    
    thread = threading.Thread(target=server_thread, daemon=True)
    thread.start()
    time.sleep(1)  # Даем серверу время запуститься
    return thread


def main():
    parser = argparse.ArgumentParser(description="ESP8266 OTA Update Automation")
    parser.add_argument('esp_ip', help='IP адрес ESP8266 (например: 10.0.1.166)')
    parser.add_argument('--esp-port', type=int, default=23, help='Telnet порт ESP (по умолчанию: 23)')
    parser.add_argument('--http-port', type=int, default=80, help='HTTP порт для файлов (по умолчанию: 80)')
    parser.add_argument('--firmware-dir', default='../mqtt_aht21/bin', help='Каталог с файлами прошивки (по умолчанию: ../mqtt_aht21/bin)')
    parser.add_argument('--local-ip', help='Локальный IP адрес (автоопределение если не указан)')
    
    args = parser.parse_args()
    
    # Проверяем существование каталога с прошивкой
    if not os.path.exists(args.firmware_dir):
        print(f"Ошибка: каталог {args.firmware_dir} не найден!")
        return 1
    
    # Показываем информацию о файлах прошивки
    print_firmware_info(args.firmware_dir)
    
    # Проверяем наличие файлов прошивки
    user1_path = os.path.join(args.firmware_dir, 'user1.bin')
    user2_path = os.path.join(args.firmware_dir, 'user2.bin')
    
    if not os.path.exists(user1_path) and not os.path.exists(user2_path):
        print(f"Ошибка: файлы user1.bin и user2.bin не найдены в {args.firmware_dir}!")
        return 1
    
    # Определяем локальный IP
    local_ip = args.local_ip or get_local_ip()
    print(f"Локальный IP адрес: {local_ip}")
    
    # Запускаем HTTP сервер
    print(f"Запуск HTTP сервера...")
    http_thread = start_http_server(args.firmware_dir, args.http_port)
    
    # Автоматический режим - отправляем команды через telnet
    print(f"Подключение к ESP8266 по адресу {args.esp_ip}:{args.esp_port}...")
    
    # Отправляем команду OTA
    ota_url = f"http://{local_ip}:{args.http_port}"
    ota_command = f"OTA={ota_url}"
    
    print(f"Отправка команды OTA...")
    response = send_telnet_command(args.esp_ip, args.esp_port, ota_command)
    
    if response and "OK" in response:
        print("Команда OTA принята ESP!")
        
        # Отправляем команду FOTA
        print("Отправка команды FOTA...")
        time.sleep(1)
        response = send_telnet_command(args.esp_ip, args.esp_port, "FOTA")
        
        if response and "OK" in response:
            print("Команда FOTA принята! ESP должен начать загрузку прошивки...")
            print("Ожидание загрузки... (можно прервать Ctrl+C)")
            
            try:
                # Ожидаем некоторое время для завершения загрузки
                for i in range(60):
                    time.sleep(1)
                    print(f"Ожидание... {i+1}/60 сек", end='\r')
                print("\nВремя ожидания истекло.")
            except KeyboardInterrupt:
                print("\nОжидание прервано пользователем.")
        else:
            print("Ошибка: ESP не принял команду FOTA!")
            return 1
    else:
        print("Ошибка: ESP не принял команду OTA!")
        return 1
    
    print("Процесс OTA обновления завершен.")
    return 0


if __name__ == "__main__":
    sys.exit(main())