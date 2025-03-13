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

    enum x_AXISTYPE
    {
        TIME = 4000,
        DEPTH = 4001
    };

    enum y_AXISTYPE
    {
        RECTIFY=4002,
        FULL=4003
    };

public slots:
    void plot(const QList<QPointF> &, bool);
    void toggleGates(bool);
    void changeXAxisType(const TChartViewForm::x_AXISTYPE&);
    void changeYAxisType(const TChartViewForm::y_AXISTYPE&);
    void setVelocity(double);
    void clear();
    void doZoomInOut(QRectF);
    void acquisitionStatus(bool);
    void toggleSave(bool);
    void startThickCal(bool);
    void updateFs(const float);
    void reset();
    QRectF chartRect() const;
    QPointF ascanValue(const QPointF &) const;
    bool gatesToggled();
    int gatePeak();
private slots:

    void backButtonEnabled(bool);
    void on_btnDataTip_clicked(bool checked);
    void on_btnZoom_clicked(bool checked);
    void on_btnReset_clicked(bool checked);
    void on_btnSave_clicked();
    void updateXMax(const float &);
    void on_btnBack_clicked();
    void doThicknessCal();


    void on_btnIncr_clicked();

    void on_btnDecr_clicked();

    void updateGatePosition();

    void on_comboAxis_currentIndexChanged(int index);

    void on_btnConfig_clicked();

    void on_spinGain_valueChanged(double arg1);

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
    const float yMin=0;
    const float yMax = 100;
    const float yRangeMin = 100;
    const float yRangeMax = 100;
    const float xMin=0;
    float xMax = mTcpClient::DATA_SIZE/2/125e3;
    double m_vel = 5890.0;
    bool _dataTipOn=false;
    bool _zoomOn=false;
    x_AXISTYPE _xAxisType = x_AXISTYPE::TIME;
    y_AXISTYPE _yAxisType = y_AXISTYPE::FULL;
    QList<QLabel *>_dataTipList;
    QLabel *generateLabel(QWidget *parent);
    double depthToTime(const double &depth);
    double timeToDepth(const double &time);
    void updateLabelPosition();
    bool inRange(const QPointF &, QValueAxis *, QValueAxis *);
    bool acquisitionRunning = false;
    QStack<QPair<QRectF, x_AXISTYPE>> zoomRectTrack;
    qreal maxInd(const QRectF &rect, bool);
    float fs = 125e6;
    QPair<QString, int> m_xUnit;
    QPair<QString, int> m_yUnit;
    float m_scale = 1.0;
    float m_spinGain;
private:
    Ui::TChartViewForm *ui;
    void setXRange(const float &, const float &);
    void setYRange(const float &, const float &);

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    void setXUnit(const QString &newXUnit);

    void setYUnit(const QString &newYUnit);

signals:
    void setRectifyCheck();
    void calculatedThickness(double);
    void sendThreshold(double);
    void scaleSet(const float&);
    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // TCHARTVIEWFORM_H
