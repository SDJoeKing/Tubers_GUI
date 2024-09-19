#ifndef TCHARTVIEWFORM_H
#define TCHARTVIEWFORM_H

#include <QWidget>
#include <QChartView>
#include <QObject>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLabel>

#include "mtcpclient.h"
#include "tgate.h"

namespace Ui {
class TChartViewForm;
}

class TChartViewForm : public QWidget
{
    Q_OBJECT

public:
    explicit TChartViewForm(QWidget *parent = nullptr);
    ~TChartViewForm();
    void plot(QList<QPointF> data);
    enum AXISTYPE
    {
        SAMPLE = 0x0001,
        DEPTH = 0x0002,
        ABSOLUTEY=0x0003,
        FULLY=0x0004
    };

public slots:
    void toogleGates(bool);
    void changeAxisType(TChartViewForm::AXISTYPE);
    void setVelocity(qfloat16);
    void clear();

private slots:


    void on_btnDataTip_clicked(bool checked);

    void on_btnZoom_clicked(bool checked);

private:
    QChartView *m_chartView;
    QChart *m_chart;
    QLineSeries *m_series;
    QLabel *m_dataTip;
    QValueAxis *m_X;
    QValueAxis *m_Y;
    TGate *m_gate1;
    TGate *m_gate2;
    float yMin=-500;
    float yMax = 500;
    float xMin=0;
    float xMax = mTcpClient::DATA_SIZE/2;
    qfloat16 m_vel;
    // QWidget interface


private:
    Ui::TChartViewForm *ui;

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // TCHARTVIEWFORM_H
