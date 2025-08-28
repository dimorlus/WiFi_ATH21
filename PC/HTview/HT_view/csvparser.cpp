#include "csvparser.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>

// === МАКРОС ДЛЯ ОТЛАДКИ ===
#define DEBUG_MODE 0  // 1 = включить отладку, 0 = отключить

#if DEBUG_MODE
    #define DEBUG_LOG(x) (qDebug() << x)
#else
    #define DEBUG_LOG(x) ((void)0)
#endif
// === КОНЕЦ МАКРОСА ===

CSVParser::CSVParser() {
    // По умолчанию: timestamp;temp;humidity
    setPattern("%d;%f;%f");
}

void CSVParser::setPattern(const QString& pattern) {
    // УБРАТЬ: m_pattern = pattern;
    parsePattern = pattern;  // используем существующую переменную
    DEBUG_LOG("Pattern set to:" << pattern);
}

void CSVParser::buildRegexFromPattern() {
    // Оставляем для совместимости, но не используем
    parseRegex.setPattern(".*");
    DEBUG_LOG("Pattern set to:" << parsePattern);  // ЗАМЕНИТЬ qDebug на DEBUG_LOG
}

QDateTime CSVParser::parseTimestamp(const QString& value, bool isUnixTime) {
    if (isUnixTime) {
        qint64 timestamp = value.toLongLong();
        return QDateTime::fromSecsSinceEpoch(timestamp);
    } else {
        return fastParseDateTime(value.trimmed());
    }
}

QDateTime CSVParser::fastParseDateTime(const QString& dateStr) {
    // Быстрый парсер для формата "Thu, Aug 21 2025 14:17"
    QStringList parts = dateStr.split(' ');
    if (parts.size() < 5) return QDateTime::currentDateTime();
    
    QString monthStr = parts[1];
    int day = parts[2].toInt();
    int year = parts[3].toInt();
    
    QStringList timeParts = parts[4].split(':');
    int hour = timeParts[0].toInt();
    int minute = timeParts.size() > 1 ? timeParts[1].toInt() : 0;
    
    // Быстрое преобразование месяца
    int month = 1;
    if (monthStr == "Jan") month = 1;
    else if (monthStr == "Feb") month = 2;
    else if (monthStr == "Mar") month = 3;
    else if (monthStr == "Apr") month = 4;
    else if (monthStr == "May") month = 5;
    else if (monthStr == "Jun") month = 6;
    else if (monthStr == "Jul") month = 7;
    else if (monthStr == "Aug") month = 8;
    else if (monthStr == "Sep") month = 9;
    else if (monthStr == "Oct") month = 10;
    else if (monthStr == "Nov") month = 11;
    else if (monthStr == "Dec") month = 12;
    
    QDate date(year, month, day);
    QTime time(hour, minute);
    
    return QDateTime(date, time);
}

QVector<DataPoint> CSVParser::parseFile(const QString& filePath) {
    QVector<DataPoint> results;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        DEBUG_LOG("Cannot open file:" << filePath);  // ЗАМЕНИТЬ qDebug на DEBUG_LOG
        return results;
    }
    
    QByteArray fileData = file.readAll();
    file.close();
    
    bool isUnixTimestamp = parsePattern.startsWith("%d");
    int estimatedLines = fileData.count('\n');
    results.reserve(estimatedLines);
    
    QTextStream stream(&fileData, QIODevice::ReadOnly);
    QString line;
    bool skipHeader = true;
    
    while (stream.readLineInto(&line)) {
        if (line.isEmpty() || line.startsWith('#')) continue;
        
        if (skipHeader && (line.contains("TIME") || line.contains("TEMP"))) {
            skipHeader = false;
            continue;
        }
        
        QStringList parts = line.split(';');
        if (parts.size() < 3) continue;
        
        DataPoint point;
        
        if (isUnixTimestamp) {
            point.timestamp = QDateTime::fromSecsSinceEpoch(parts[0].toLongLong());
            point.temperature = parts[1].toDouble();
            point.humidity = parts[2].toDouble();
        } else {
            point.timestamp = fastParseDateTime(parts[0].trimmed());
            point.temperature = parts[1].toDouble();
            point.humidity = parts[2].toDouble();
            if (parts.size() > 3) {
                point.deviceId = parts[3];
            }
        }
        
        results.append(point);
    }
    
    DEBUG_LOG("Parsed" << results.size() << "data points");
    return results;
}
