#ifndef CSVWORKER_H
#define CSVWORKER_H

#include <QObject>
#include <QThread>
#include "csvparser.h"

class CSVWorker : public QObject
{
  Q_OBJECT

public:
  explicit CSVWorker(QObject *parent = nullptr);
  void setParameters(const QString& filePath, const QString& pattern);

public slots:
  void parseFile();

signals:
  void progressChanged(int percentage);
  void statusChanged(const QString& status);
  void finished(const QVector<DataPoint>& data);
  void error(const QString& message);

private:
  QString m_filePath;
  QString m_pattern;
};

#endif // CSVWORKER_H
