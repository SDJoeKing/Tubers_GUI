#ifndef TCHARTVIEW_H
#define TCHARTVIEW_H

#include <QChartView>
#include <QObject>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>

#include "mtcpclient.h"
#include "tgate.h"

class TChartView : public QChartView
{
    Q_OBJECT
public:
    explicit TChartView(QWidget *parent = nullptr);
    void plot(QList<QPointF> data, bool axisIsDepth);

public slots:
    // void toogleGates(bool);

private:

    QChart *m_chart;
    QLineSeries *m_series;
    QValueAxis *m_X;
    QValueAxis *m_Y;
    TGate *m_gate1;
    TGate *m_gate2;
    float yMin=-20;
    float yMax = 20;
    float xMin=0;
    float xMax = mTcpClient::DATA_SIZE/2;

    // QWidget interface
protected:
    // virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // TCHARTVIEW_H
