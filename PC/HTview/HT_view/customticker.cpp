#include "customticker.h"

ThreeStepTicker::ThreeStepTicker() : m_tickStep(1.0)
{
}

void ThreeStepTicker::setTickStep(double step)
{
  m_tickStep = step;
}

QVector<double> ThreeStepTicker::createTickVector(double tickStep, const QCPRange &range)
{
  QVector<double> result;

  // Основные риски каждый градус (с подписями)
  double start = qFloor(range.lower / m_tickStep) * m_tickStep;
  double end = qCeil(range.upper / m_tickStep) * m_tickStep;

  for (double tick = start; tick <= end; tick += m_tickStep) {
    if (range.contains(tick)) {
      result.append(tick);
    }
  }

  return result;
}

QVector<double> ThreeStepTicker::createSubTickVector(int subTickCount, const QVector<double> &ticks)
{
  QVector<double> result;

  // Средние риски через 0.5° (без подписей)
  for (int i = 0; i < ticks.size() - 1; ++i) {
    result.append(ticks[i] + m_tickStep / 2.0);
  }

  // Мелкие риски через 0.1° (если масштаб позволяет)
  if (m_tickStep <= 2.0) {  // только для мелких масштабов
    QVector<double> fineResult = result;
    for (int i = 0; i < ticks.size() - 1; ++i) {
      for (double fine = ticks[i] + 0.1; fine < ticks[i] + m_tickStep; fine += 0.1) {
        if (qAbs(fine - (ticks[i] + m_tickStep / 2.0)) > 0.05) { // избегаем дублирования с 0.5°
          fineResult.append(fine);
        }
      }
    }
    return fineResult;
  }

  return result;
}
