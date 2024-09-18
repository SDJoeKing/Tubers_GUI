#ifndef TCHARTVIEW_H
#define TCHARTVIEW_H

#include <QChartView>
#include <QObject>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>

#include "mtcpclient.h"

class TChartView : public QChartView
{
    Q_OBJECT
public:
    explicit TChartView(mTcpClient *client, QWidget *parent = nullptr);
    void plot(QList<QPointF> data, bool axisIsDepth);

private:

    QChart *m_chart;
    QLineSeries *m_series;
    QValueAxis *m_X;
    QValueAxis *m_Y;

};

#endif // TCHARTVIEW_H
