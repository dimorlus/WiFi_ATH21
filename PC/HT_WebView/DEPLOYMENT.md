# HT WebView - Deployment Guide

## Деплой на сервер как Windows Service

### 1. Подготовка сервера

**Минимальные требования:**
- Windows Server или Windows 10/11
- Node.js 18+ ([скачать](https://nodejs.org/))
- 100MB свободного места

### 2. Подготовка к деплою

```bash
# В папке проекта выполните:
deploy.bat
```

Этот скрипт:
- Соберет проект (`npm run build`)
- Подготовит файлы для деплоя
- Установит необходимые зависимости

### 3. Настройка путей

Отредактируйте `install-service.js`:
```javascript
env: [
  {
    name: "PORT",
    value: "3000"  // Порт сервера
  },
  {
    name: "CSV_DIR", 
    value: "C:\\Data\\CSV"  // ПУТЬ К ПАПКЕ С CSV ФАЙЛАМИ
  }
]
```

### 4. Установка как Windows Service

```bash
# Установить сервис (запустить как Администратор)
node install-service.js
```

Сервис будет:
- ✅ Запускаться автоматически при загрузке Windows
- ✅ Работать в фоне без окон
- ✅ Перезапускаться при сбоях
- ✅ Доступен по адресу: http://localhost:3000

### 5. Управление сервисом

```bash
# Удалить сервис
node uninstall-service.js

# Или через Windows Services:
# services.msc → "HT WebView Server"
```

### 6. Доступ к CSV файлам

#### Вариант A: Локальные файлы через API (Рекомендуется)
- Поместите CSV файлы в папку `C:\Data\CSV` (или измените путь в install-service.js)
- Сервер автоматически найдет и покажет все .csv файлы
- Пользователи смогут выбирать файлы из списка на веб-странице

#### Вариант B: Сетевая папка
- Разместите CSV файлы в общей сетевой папке
- Настройте CSV_DIR на сетевой путь: `\\server\share\csv`
- Убедитесь, что сервис имеет права доступа

#### Вариант C: Автоматическая загрузка
- Настройте MQTT Logger для сохранения в папку CSV_DIR
- Файлы будут автоматически появляться в списке

### 7. Сетевой доступ

Для доступа с других компьютеров:

1. **Откройте порт в брандмауэре:**
   ```bash
   netsh advfirewall firewall add rule name="HT WebView" dir=in action=allow protocol=TCP localport=3000
   ```

2. **Измените listen адрес** в `deploy-server.js`:
   ```javascript
   app.listen(PORT, '0.0.0.0', () => {  // 0.0.0.0 для доступа извне
   ```

3. **Доступ по IP:** `http://192.168.1.100:3000`

### 8. Альтернативные варианты деплоя

#### A. Простой HTTP сервер
```bash
npm install -g serve
serve -s dist -l 3000
```

#### B. IIS (Internet Information Services)
- Скопируйте папку `dist` в `C:\inetpub\wwwroot\ht-webview`
- Настройте IIS site

#### C. Apache/Nginx
- Настройте веб-сервер для статических файлов
- Проксируйте API запросы к Node.js серверу

### 9. Логи и мониторинг

Логи Windows Service находятся в:
- Event Viewer → Windows Logs → Application
- Поиск: "HT WebView Server"

### 10. Обновление

Для обновления:
1. Остановите сервис
2. Замените файлы
3. Запустите сервис

```bash
node uninstall-service.js
# Замените файлы
node install-service.js
```

---

## Структура файлов после деплоя:

```
deployment/
├── server.js              # Основной сервер
├── package.json           # Зависимости для продакшена
├── install-service.js     # Установка Windows Service
├── uninstall-service.js   # Удаление Windows Service
├── dist/                  # Статические файлы веб-приложения
│   ├── index.html
│   ├── assets/
│   └── ...
└── csv-data/              # Папка с CSV файлами (опционально)
    ├── data1.csv
    ├── data2.csv
    └── ...
```

## Примеры использования:

1. **Локальный сервер** - `http://localhost:3000`
2. **Корпоративная сеть** - `http://192.168.1.100:3000`
3. **С доменом** - `http://ht-monitor.company.local:3000`
