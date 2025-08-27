#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include <QRegularExpression>

struct DataPoint {
  QDateTime timestamp;
  double temperature;
  double humidity;
  QString deviceId;
};

class CSVParser {
public:
  CSVParser();

  // Установить шаблон парсинга (например: "DateTime('ddd, MMM dd yyyy HH:mm');%f;%f" или "%d;%f;%f")
  void setPattern(const QString& pattern);

  // Парсить файл и вернуть точки данных
  QVector<DataPoint> parseFile(const QString& filePath);
  const QRegularExpression& getRegex() const { return parseRegex; }

private:
  QString parsePattern;
  QRegularExpression parseRegex;

  void buildRegexFromPattern();
  QDateTime parseTimestamp(const QString& value, bool isUnixTime);
  QDateTime fastParseDateTime(const QString& dateStr);
};

#endif // CSVPARSER_H
