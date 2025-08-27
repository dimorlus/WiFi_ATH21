#ifndef CUSTOMTICKER_H
#define CUSTOMTICKER_H

#include "qcustomplot.h"

class ThreeStepTicker : public QCPAxisTicker
{
public:
  explicit ThreeStepTicker();
  void setTickStep(double step);

protected:
  virtual QVector<double> createTickVector(double tickStep, const QCPRange &range) override;
  virtual QVector<double> createSubTickVector(int subTickCount, const QVector<double> &ticks) override;

private:
  double m_tickStep;
};

#endif // CUSTOMTICKER_H
