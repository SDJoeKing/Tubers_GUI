#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QStyleFactory>
#include <QMessageBox>
#include <QTimer>
#include <QSplitter>
#include <QtCharts>
#include <QValueAxis>
#include <QLineSeries>
#include <QInputDialog>
#include <QEvent>
#include <QWheelEvent>
#include <QVBoxLayout>

#include "filt.h"
#include "tsettings.h"
#include "tlogging.h"
#include "tgate.h"
#include "mtcpclient.h"
#include "tchartview.h"
#include "tchartviewform.h"
#include "tlabel.h"
#include "qcustomplot.h"



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    // btns and actions
    void on_actionConnection_Status_triggered(bool checked);
    void on_actionTools_triggered(bool checked);
    void on_actionSettings_triggered(bool checked);
    void doSettingsConfirmed(QString );
    void on_actionLogging_triggered(bool checked);
    void on_ckBscan_clicked(bool checked);
    void on_btnConnect_clicked(bool checked);

    // connected actions
    void logMsg(QString);
    void toogleStatus(bool arg);
    void runAcquisition();
    void stopAcquisition();
    void doDataReady();
    void on_btnRun_clicked(bool checked);
    void on_spinEnvLevel_valueChanged(int arg1);
    void on_ckGates_clicked(bool checked);
    void on_ckDepthAxis_clicked(bool checked);

    void on_ckRectify_clicked(bool checked);
    void updateBScan(const QList<QPointF> &data, bool forward);
    void on_actionReset_triggered(bool);
    void setRectifyUnchecked();

    void bScanCustomContext(const QPoint &pos);
    void on_btnCal_clicked();

private:
    QTimer _tempTimer;
    Ui::MainWindow *ui;
    mTcpClient *m_client;
    QTimer *m_timer;
    TSettings *m_settings;
    TChartViewForm *m_Ascan;

    QCustomPlot *m_Bscan;

    Tlogging *m_logging;
    QLabel *m_status;
    QByteArray m_serverData;
    int m_minTimerInterval=17; // in ms
    int m_timerInterval=30; // in ms
    float m_vel=5890.0;
    // envelope coefficients
    qfloat16 m_ga;
    qfloat16 m_gr;
    static qfloat16 _env;
    static void resetEnv();
    int m_currentLine=-1;

    // filter param
    int m_order = 3;
    qfloat16 m_LC = 3.0;
    qfloat16 m_HC = 10.0;
    std::vector<double> m_a;
    std::vector<double> m_b;
    void updateFilter();
// private functions
private:
    void resetUI();
    void setConnectionIndicator();
    void doRequestData();
    void updateTimer();
    void set_envelope(float, float);
    qfloat16 envelope(qfloat16);

signals:
    void velocitySet(qfloat16);
    void dataReceived(const QList<QPointF> &data, bool direction);
    void axisTypeChanged(TChartViewForm::AXISTYPE type);
    void acquisitionRun(bool);

    // QWidget interface
};
#endif // MAINWINDOW_H
