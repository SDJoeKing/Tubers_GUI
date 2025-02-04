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
#include <QSizePolicy>
#include <QFutureWatcher>
#include <QtConcurrent>

#include "tsettings.h"
#include "tlogging.h"
#include "mtcpclient.h"
#include "tchartviewform.h"
#include "qcustomplot.h"
#include "DspFilters/Dsp.h"
#include "processor.h"
#include "testserver.h"

// #define TEST_SERVER // PLEASE ENSURE IN TEST SERVER MODE THE ACTUAL REMOTE SERVER IS NOT CONNECTED, THIS IS FOR TESTING ONLY


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
    static double _env ;
    static double m_ga ;
    static double m_gr ;
    static double envelope(double sample, double &value, double ga, double gr);

private slots:

    // btns and actions
    void on_actionConnection_Status_triggered(bool checked);
    void on_actionTools_triggered(bool checked);
    void on_actionSettings_triggered(bool checked);
    void doSettingsConfirmed(QString );
    void on_actionLogging_triggered(bool checked);
    void on_ckBscan_clicked(bool checked);
    void on_btnConnect_clicked(bool checked);
    void hideSetting();

    // connected actions
    void logMsg(QString);
    void toggleStatus(bool arg);
    void runAcquisition();
    void stopAcquisition();
    void on_btnRun_clicked(bool checked);
    void on_spinEnvLevel_valueChanged(int arg1);
    void on_ckGates_clicked(bool checked);
    void on_ckDepthAxis_clicked(bool checked);

    void on_ckRectify_clicked(bool checked);
    void updateBScan(const QList<QPointF> &data, bool);
    void on_actionReset_triggered(bool);
    void setRectifyChecked();

    void bScanCustomContext(const QPoint &pos);
    void on_btnCal_clicked();

    void do_bScanSetting(bool, const QList<double> &);

    void updateFs(double);
    // debugging fps
    void do_fps(float);
    void do_ConnectLost();
    // processing
    void threadFinished();
    void setThreshold(double);
    void updateTemp(const float);

    // badsettings
    void do_badSettings();


private:
    QThread socketThread;
    QThread *m_serverThread;
    testServer *m_server;
    QThread processorThread;
    QTimer _tempTimer;
    Ui::MainWindow *ui;
    mTcpClient *m_client;
    TSettings *m_settings;
    TChartViewForm *m_Ascan;

    QCustomPlot *m_Bscan;
    Tlogging *m_logging;
    QLabel *m_status;
    QByteArray m_serverData;
    float m_vel=5890.0;
    bool encoderTriggerMode = false;
    static void resetEnv();
    int m_currentLine=-1;

    // filter param

    int m_order = 4;
    double m_fc = 5;
    double m_fw = 8;
    int m_fs = 125;

    bool use_bscan = 0;
    double m_thres = 0.0;
    // processor
    Processor *m_processor;
    QEventLoop m_quitEvent;

    // status
    bool acquisitionRunning = false;
    bool connected = false;
// private functions
private:
    void resetUI();
    void setConnectionIndicator();
    void set_envelope(float, float);
    void toggleOff(QCheckBox *);
signals:
    void velocitySet(double);
    void mainSendSetting(const QString &);
    void dataReceived(const QList<QPointF> &data);
    void axisTypeChanged(const TChartViewForm::x_AXISTYPE &);
    void axisTypeChanged(const TChartViewForm::y_AXISTYPE &type);
    void acquisitionRun(bool);
    void dataForLogger(const QByteArray &);
    void stopAcqSig();
    void filterParam(const quint8 & order, const quint8 &fs, const float &fc, const float &fw);
};
#endif // MAINWINDOW_H
