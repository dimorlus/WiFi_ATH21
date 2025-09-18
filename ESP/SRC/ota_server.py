import argparse
import os
import socket
import sys
import threading
from http.server import HTTPServer, SimpleHTTPRequestHandler

import asyncio
import telnetlib3


def get_local_ip(target_ip):
    """Определяет локальный IP-адрес, с которого будет доступен HTTP сервер для ESP."""
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect((target_ip, 1))
        ip = s.getsockname()[0]
    except Exception:
        ip = '127.0.0.1'
    finally:
        s.close()
    return ip


class LoggingHTTPRequestHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        client_ip = self.client_address[0]
        print(f"[HTTP] GET {self.path} from {client_ip}")
        if self.path.endswith("user1.bin"):
            print(f"[HTTP] ESP скачивает user1.bin")
        elif self.path.endswith("user2.bin"):
            print(f"[HTTP] ESP скачивает user2.bin")
        super().do_GET()

def start_http_server(directory, port=80):
    os.chdir(directory)
    handler = LoggingHTTPRequestHandler
    httpd = HTTPServer(("", port), handler)
    print(f"[HTTP] Serving {directory} on port {port}")
    httpd.serve_forever()


def telnet_ota(esp_host, esp_port, ota_url):
    return asyncio.run(telnet_ota_async(esp_host, esp_port, ota_url))


async def telnet_ota_async(esp_host, esp_port, ota_url):
    print(f"[TELNET] Connecting to {esp_host}:{esp_port} ...")
    try:
        reader, writer = await telnetlib3.open_connection(esp_host, esp_port, shell=None, connect_minwait=0.5)
        await asyncio.sleep(1)  # Ждем приглашения, если есть

        # Считываем строки до приглашения OTA
        while True:
            line = await reader.readline()
            if not line:
                break
            line = line.strip()
            if line:
                print(f"[TELNET] {line}")
            if line == "Telnet setup":
                break

        await asyncio.sleep(2)  # Задержка перед OTA
        print(f"[TELNET] Sending: OTA={ota_url}")
        writer.write(f"OTA={ota_url}\n")
        await writer.drain()

        # Ждем OK после OTA
        ok_received = False
        while True:
            resp = await reader.readline()
            if not resp:
                break
            resp = resp.strip()
            print(f"[TELNET] Response: {resp}")
            if resp == "OK":
                ok_received = True
                break
        if not ok_received:
            print("[TELNET] ESP не ответил OK на OTA. Прерывание.")
            writer.close()
            await writer.wait_closed()
            return False

        await asyncio.sleep(2)  # Задержка перед FOTA
        print("[TELNET] Sending: FOTA")
        writer.write("FOTA\n")
        await writer.drain()

        # Ждем OK после FOTA
        ok_received = False
        while True:
            resp = await reader.readline()
            if not resp:
                break
            resp = resp.strip()
            print(f"[TELNET] Response: {resp}")
            if resp == "OK":
                ok_received = True
                break
        if not ok_received:
            print("[TELNET] ESP не ответил OK на FOTA. Прерывание.")
            writer.close()
            await writer.wait_closed()
            return False

        print("[TELNET] OTA команда отправлена. Telnet-соединение оставлено открытым.")
        return (reader, writer)
    except Exception as e:
        print(f"[TELNET] Ошибка: {e}")
        return False


def main():
    parser = argparse.ArgumentParser(description="ESP8266 OTA Update Server")
    parser.add_argument('--esp', required=True, help='ESP8266 address, e.g. 10.0.1.166:23')
    parser.add_argument('--dir', default='.', help='Directory with user1.bin and user2.bin')
    parser.add_argument('--port', type=int, default=80, help='HTTP server port (default: 80)')
    args = parser.parse_args()

    if not os.path.isdir(args.dir):
        print(f"[ERROR] Directory not found: {args.dir}")
        sys.exit(1)
    for fname in ("user1.bin", "user2.bin"):
        if not os.path.isfile(os.path.join(args.dir, fname)):
            print(f"[ERROR] File not found: {fname} in {args.dir}")
            sys.exit(1)

    esp_ip, esp_port = args.esp.split(":")
    esp_port = int(esp_port)
    local_ip = get_local_ip(esp_ip)
    ota_url = f"http://{local_ip}:{args.port}"

    # Запуск HTTP сервера в отдельном потоке
    http_thread = threading.Thread(target=start_http_server, args=(args.dir, args.port), daemon=True)
    http_thread.start()

    telnet_result = telnet_ota(esp_ip, esp_port, ota_url)
    if not telnet_result:
        print("[ERROR] OTA update aborted.")
        sys.exit(2)
    reader, writer = telnet_result

    print("[INFO] OTA process initiated. ESP should connect to the HTTP server.")
    print("[INFO] Waiting 60 seconds for firmware download...")
    try:
        for i in range(60, 0, -1):
            print(f"[INFO] Server will stop in {i} seconds...", end='\r')
            import time
            time.sleep(1)
        print("\n[INFO] Server stopped.")
    except KeyboardInterrupt:
        print("\n[INFO] Server stopped by user.")
    finally:
        if writer:
            try:
                writer.close()
                import asyncio
                asyncio.run(writer.wait_closed())
            except Exception:
                pass


if __name__ == "__main__":
    main()
