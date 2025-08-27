#include "csvworker.h"
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDebug>

CSVWorker::CSVWorker(QObject *parent) : QObject(parent)
{
}

void CSVWorker::setParameters(const QString& filePath, const QString& pattern)
{
    m_filePath = filePath;
    m_pattern = pattern;
}

void CSVWorker::parseFile()
{
    emit statusChanged("Opening file...");
    emit progressChanged(5);
    
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit error("Cannot open file: " + m_filePath);
        return;
    }
    
    emit statusChanged("Reading file...");
    emit progressChanged(15);
    
    // Прочитать весь файл разом (самое быстрое)
    QByteArray fileData = file.readAll();
    file.close();
    
    emit progressChanged(25);
    emit statusChanged("Parsing data...");
    
    bool isUnixTimestamp = m_pattern.startsWith("%d");
    QVector<DataPoint> results;
    
    // Оценить количество строк для резервирования памяти
    int estimatedLines = fileData.count('\n');
    results.reserve(estimatedLines);
    
    // БЫСТРЫЙ парсинг с QTextStream
    QTextStream stream(&fileData, QIODevice::ReadOnly);
    QString line;
    bool skipHeader = true;
    int lineCount = 0;
    
    while (stream.readLineInto(&line)) {
        lineCount++;
        
        // Обновляем прогресс каждые 50 строк
        if (lineCount % 50 == 0) {
            int progress = 25 + (int)(30.0 * lineCount / estimatedLines);
            emit progressChanged(qMin(progress, 55));
        }
        
        if (line.isEmpty() || line.startsWith('#')) continue;
        
        // Пропустить заголовок
        if (skipHeader && (line.contains("TIME") || line.contains("TEMP"))) {
            skipHeader = false;
            continue;
        }
        
        // БЫСТРЫЙ split вместо regex
        QStringList parts = line.split(';');
        if (parts.size() < 3) continue;
        
        DataPoint point;
        
        if (isUnixTimestamp) {
            // Формат: timestamp;temp;hum
            point.timestamp = QDateTime::fromSecsSinceEpoch(parts[0].toLongLong());
            point.temperature = parts[1].toDouble();
            point.humidity = parts[2].toDouble();
        } else {
            // Формат: DateTime;temp;hum;device
            QString timeStr = parts[0].trimmed();
            point.timestamp = QDateTime::fromString(timeStr, "ddd, MMM dd yyyy HH:mm");
            if (!point.timestamp.isValid()) {
                point.timestamp = QDateTime::currentDateTime();
            }
            point.temperature = parts[1].toDouble();
            point.humidity = parts[2].toDouble();
            if (parts.size() > 3) {
                point.deviceId = parts[3];
            }
        }
        
        results.append(point);
    }
    
    emit statusChanged("Parsing complete");
    emit progressChanged(60);
    
    qDebug() << "Fast parsed" << results.size() << "data points from" << fileData.size() << "bytes";
    emit finished(results);
}
