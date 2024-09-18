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

private:
    Ui::MainWindow *ui;
    mTcpClient *m_client;
    QTimer *m_timer;
    TSettings *m_settings;
    QChartView *m_Ascan;
    QChartView *m_Bscan;
    Tlogging *m_logging;
    QLabel *m_status;
    QByteArray m_serverData;
    int m_minTimerInterval=17; // in ms
    int m_timerInterval=30; // in ms
    float m_vel=0.0;

    // envelope coefficients
    qfloat16 m_ga;
    qfloat16 m_gr;
// private functions
private:
    void resetUI();
    void setConnectionIndicator();
    void doRequestData();
    void updateTimer();
    void set_envelope(float, float);
    qfloat16 envelope(qfloat16);
signals:
    void dataReceived();
};
#endif // MAINWINDOW_H
