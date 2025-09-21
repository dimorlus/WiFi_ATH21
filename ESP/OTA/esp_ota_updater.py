#!/usr/bin/env python3
"""
ESP8266 OTA Update Script
Автоматизирует процесс OTA обновления прошивки ESP8266:

ОСНОВНЫЕ ВОЗМОЖНОСТИ:
1. Автоматическое определение ESP устройств по IP адресу или имени
2. Умный поиск устройств в сети (ARP таблица + сканирование подсети)
3. Извлечение MAC адреса из имени устройства (формат HT_XXXXXXXXXXXX)
4. HTTP сервер для раздачи файлов прошивки (user1.bin/user2.bin)
5. Автоматическая отправка команд OTA и FOTA через telnet
6. Подробная информация о файлах прошивки (размер, дата изменения)

ПРИМЕРЫ ИСПОЛЬЗОВАНИЯ:
  python esp_ota_updater.py 10.0.1.166                    # По IP адресу
  python esp_ota_updater.py HT_3C71BF29A3EC               # По имени устройства  
  python esp_ota_updater.py HT_3C71BF29A3EC --http-port 8000  # Кастомный HTTP порт

АЛГОРИТМ ПОИСКА УСТРОЙСТВ:
1. Если передан IP - использует напрямую
2. Если передано имя:
   a) Проверяет ARP таблицу по имени
   b) Извлекает MAC из имени (HT_XXXXXXXXXXXX -> xx-xx-xx-xx-xx-xx)
   c) Ищет MAC в ARP таблице
   d) Сканирует локальную подсеть (ping + hostname resolution)

ТРЕБОВАНИЯ:
- Python 3.6+
- Стандартные утилиты Windows: arp, ping
- ESP8266 с поддержкой telnet команд OTA/FOTA
"""

import argparse
import socket
import time
import threading
import os
import sys
import datetime
import subprocess
import re
from concurrent.futures import ThreadPoolExecutor
from http.server import HTTPServer, SimpleHTTPRequestHandler
from socketserver import ThreadingMixIn


class ThreadingHTTPServer(ThreadingMixIn, HTTPServer):
    """HTTP сервер с поддержкой многопоточности"""
    daemon_threads = True


def is_ip_address(address):
    """Проверяет, является ли строка IP адресом"""
    ip_pattern = r'^(\d{1,3}\.){3}\d{1,3}$'
    if not re.match(ip_pattern, address):
        return False
    
    # Дополнительная проверка диапазонов
    parts = address.split('.')
    return all(0 <= int(part) <= 255 for part in parts)


def extract_mac_from_hostname(hostname):
    """Извлекает MAC адрес из имени устройства вида HT_3C71BF29A3EC"""
    if '_' in hostname:
        mac_part = hostname.split('_')[-1]  # Берем часть после последнего _
        if len(mac_part) == 12:  # MAC без разделителей должен быть 12 символов
            # Преобразуем 3C71BF29A3EC в 3c-71-bf-29-a3-ec
            mac_formatted = '-'.join([mac_part[i:i+2].lower() for i in range(0, 12, 2)])
            return mac_formatted
    return None


def check_arp_table(hostname):
    """Проверяет ARP таблицу на наличие устройства с указанным именем или MAC"""
    try:
        # Windows: arp -a
        result = subprocess.run(['arp', '-a'], capture_output=True, text=True, timeout=5)
        
        # Сначала пробуем найти по имени
        for line in result.stdout.split('\n'):
            if hostname.lower() in line.lower():
                ip_match = re.search(r'(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})', line)
                if ip_match:
                    return ip_match.group(1)
        
        # Если не нашли по имени, пробуем извлечь MAC из имени и найти по MAC
        expected_mac = extract_mac_from_hostname(hostname)
        if expected_mac:
            print(f"[MAC] Поиск по MAC адресу: {expected_mac}")
            for line in result.stdout.split('\n'):
                if expected_mac in line.lower():
                    ip_match = re.search(r'(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})', line)
                    if ip_match:
                        print(f"[MAC] Найдено соответствие MAC -> IP: {ip_match.group(1)}")
                        return ip_match.group(1)
                        
    except Exception as e:
        print(f"[WARNING] Ошибка проверки ARP таблицы: {e}")
    return None


def get_hostname_by_ip(ip):
    """Получает имя устройства по IP адресу"""
    try:
        hostname = socket.gethostbyaddr(ip)[0]
        return hostname
    except:
        return None


def ping_ip(ip, timeout=1):
    """Проверяет доступность IP адреса через ping"""
    try:
        # Windows: ping -n 1 -w 1000 (1000ms = 1s)
        result = subprocess.run(['ping', '-n', '1', '-w', str(timeout * 1000), ip], 
                              capture_output=True, timeout=timeout + 1)
        return result.returncode == 0
    except:
        return False


def scan_subnet_for_hostname(hostname, base_network=None):
    """Сканирует подсеть в поисках устройства с указанным именем"""
    if not base_network:
        local_ip = get_local_ip()
        if not local_ip:
            return None
        # Получаем базовую сеть (например, 10.0.1 из 10.0.1.200)
        base_network = '.'.join(local_ip.split('.')[:-1])
    
    print(f"[SCAN] Сканирование сети {base_network}.1-254...")
    found_devices = []
    
    def check_host(host_num):
        ip = f"{base_network}.{host_num}"
        if ping_ip(ip, timeout=0.5):  # Быстрый ping
            hostname_found = get_hostname_by_ip(ip)
            if hostname_found and hostname.lower() in hostname_found.lower():
                found_devices.append((ip, hostname_found))
                print(f"[FOUND] {ip} -> {hostname_found}")
    
    # Параллельное сканирование для скорости
    with ThreadPoolExecutor(max_workers=50) as executor:
        executor.map(check_host, range(1, 255))
    
    if found_devices:
        return found_devices[0][0]  # Возвращаем первый найденный IP
    return None


def find_esp_by_hostname(hostname):
    """Ищет ESP устройство по имени"""
    print(f"[SEARCH] Поиск устройства '{hostname}'...")
    
    # 1. Быстрая проверка ARP таблицы
    print("[ARP] Проверка ARP таблицы...")
    ip = check_arp_table(hostname)
    if ip:
        print(f"[SUCCESS] Найдено в ARP таблице: {ip}")
        return ip
    
    # 2. Сканирование подсети
    print("[SCAN] Сканирование локальной сети...")
    ip = scan_subnet_for_hostname(hostname)
    if ip:
        print(f"[SUCCESS] Найдено при сканировании: {ip}")
        return ip
    
    print(f"[ERROR] Устройство '{hostname}' не найдено в сети")
    return None


def parse_esp_address(address):
    """Определяет тип адреса (IP или имя) и возвращает IP"""
    if is_ip_address(address):
        print(f"[INFO] Используется IP адрес: {address}")
        return address
    else:
        print(f"[INFO] Поиск устройства по имени: {address}")
        return find_esp_by_hostname(address)


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
    parser.add_argument('esp_address', help='IP адрес ESP8266 (например: 10.0.1.166) или имя устройства (например: HT_3C71BF29A3EC)')
    parser.add_argument('--esp-port', type=int, default=23, help='Telnet порт ESP (по умолчанию: 23)')
    parser.add_argument('--http-port', type=int, default=80, help='HTTP порт для файлов (по умолчанию: 80)')
    parser.add_argument('--firmware-dir', default='../mqtt_aht21/bin', help='Каталог с файлами прошивки (по умолчанию: ../mqtt_aht21/bin)')
    parser.add_argument('--local-ip', help='Локальный IP адрес (автоопределение если не указан)')
    
    args = parser.parse_args()
    
    # Определяем IP адрес ESP (может быть задан как IP или как имя устройства)
    esp_ip = parse_esp_address(args.esp_address)
    if not esp_ip:
        print(f"[ERROR] Не удалось определить IP адрес для '{args.esp_address}'")
        return 1
    
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
    print(f"[NET] Локальный IP адрес: {local_ip}")
    print(f"[ESP] ESP8266 IP адрес: {esp_ip}")
    
    # Запускаем HTTP сервер
    print(f"[HTTP] Запуск HTTP сервера...")
    http_thread = start_http_server(args.firmware_dir, args.http_port)
    
    # Автоматический режим - отправляем команды через telnet
    print(f"[TELNET] Подключение к ESP8266 по адресу {esp_ip}:{args.esp_port}...")
    
    # Отправляем команду OTA
    ota_url = f"http://{local_ip}:{args.http_port}"
    ota_command = f"OTA={ota_url}"
    
    print(f"[OTA] Отправка команды OTA...")
    response = send_telnet_command(esp_ip, args.esp_port, ota_command)
    
    if response and "OK" in response:
        print("[SUCCESS] Команда OTA принята ESP!")
        
        # Отправляем команду FOTA
        print("[FOTA] Отправка команды FOTA...")
        time.sleep(1)
        response = send_telnet_command(esp_ip, args.esp_port, "FOTA")
        
        if response and "OK" in response:
            print("[SUCCESS] Команда FOTA принята! ESP должен начать загрузку прошивки...")
            print("[WAIT] Ожидание загрузки... (можно прервать Ctrl+C)")
            
            try:
                # Ожидаем некоторое время для завершения загрузки
                for i in range(60):
                    time.sleep(1)
                    print(f"[WAIT] Ожидание... {i+1}/60 сек", end='\r')
                print("\n[TIMEOUT] Время ожидания истекло.")
            except KeyboardInterrupt:
                print("\n[STOP] Ожидание прервано пользователем.")
        else:
            print("[ERROR] ESP не принял команду FOTA!")
            return 1
    else:
        print("[ERROR] ESP не принял команду OTA!")
        return 1
    
    print("[DONE] Процесс OTA обновления завершен.")
    return 0


if __name__ == "__main__":
    sys.exit(main())