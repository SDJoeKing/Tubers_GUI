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
#include <QSizePolicy>
#include <QFileDialog>
#include <QStack>

#include "mtcpclient.h"
#include "tgate.h"
#include "tchartview.h"
namespace Ui {
class TChartViewForm;
}

class TChartViewForm : public QWidget
{
    Q_OBJECT

public:
    explicit TChartViewForm(QWidget *parent = nullptr);
    ~TChartViewForm();

    enum AXISTYPE
    {
        SAMPLE = 4000,
        DEPTH = 4001,
        ABSOLUTEY=4002,
        FULLY=4003
    };

    int getAscanLength();

public slots:
    void plot(const QList<QPointF> &, bool);
    void toogleGates(bool);
    void changeAxisType(TChartViewForm::AXISTYPE);
    void setVelocity(double);
    void clear();
    void doZoomInOut(QRectF);
    void acquisitionStatus(bool);
    void toogleSave(bool);
    void startThickCal(bool);
private slots:

    void backButtonEnabled(bool);
    void on_btnDataTip_clicked(bool checked);
    void on_btnZoom_clicked(bool checked);
    void on_btnReset_clicked(bool checked);
    void on_btnSave_clicked();

    void on_btnBack_clicked();
    void doThicknessCal();


    void on_comboLength_currentIndexChanged(int index);

private:
    QTimer m_timer;
    TChartView *m_chartView;
    QChart *m_chart;
    QLineSeries *m_series;
    QLineSeries *m_ruler;
    QLabel *m_dataTip;
    QValueAxis *m_X;
    QValueAxis *m_Y;
    TGate *m_gate1;
    TGate *m_gate2;
    const float yMin=-500;
    const float yMax = 500;
    const float xMin=0;
    const float xMax = mTcpClient::DATA_SIZE/2;
    double m_vel = 5890.0;
    bool _dataTipOn=false;
    bool _zoomOn=false;
    AXISTYPE _xAxisType = AXISTYPE::SAMPLE;
    AXISTYPE _yAxisType = AXISTYPE::FULLY;
    QList<QLabel *>_dataTipList;
    QLabel *generateLabel(QWidget *parent);
    int depthToPoint(double depth);
    double pointToDepth(int point);
    void updateLabelPosition();
    bool inRange(const QPointF &, QValueAxis *, QValueAxis *);
    bool acquisitionRunning = false;
    QStack<QPair<QRectF, AXISTYPE>> zoomRectTrack;
    qreal maxInd(const QRectF &rect);
private:
    Ui::TChartViewForm *ui;

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void setRectifyUncheck();
    void calculatedThickness(double);
    void fsChanged(double);
};

#endif // TCHARTVIEWFORM_H
