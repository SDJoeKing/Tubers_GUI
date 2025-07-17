#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QVector>
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
#include <thread>

#include "tsettings.h"
#include "tlogging.h"
#include "mtcpclient.h"
#include "tchartviewform.h"
#include "qcustomplot.h"
#include "DspFilters/Dsp.h"
#include "processor.h"
#include "testserver.h"
#include "tmath.h"

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

    void updateBScan(float *, bool);

    void on_actionReset_triggered(bool);
    void setRectifyChecked();

    void bScanCustomContext(const QPoint &pos);
    void on_btnCal_clicked();

    void do_bScanSetting(bool, const QList<double> &);

    void updateFs(double);
    // debugging fps
    void do_fps(float);
    void do_plotRate(const float&);
    void do_ConnectLost();
    // processing
    void threadFinished();
    void setThreshold(double);
    void updateHeaderInfo(const float&, const float &, const quint8 &, const QVector<qint16> &imus);

    // badsettings
    void do_badSettings();

    // setting ready
    void do_settingReady();
    void on_btnImuBase_toggled(bool checked);

    // tfm
    void rescaleBscan();
private:
    QThread socketThread;
    QThread *m_serverThread;
    testServer *m_server;
    QThread processorThread;
    QTimer m_updateTimer;
    Ui::MainWindow *ui;
    mTcpClient *m_client;
    TSettings *m_settings;
    TChartViewForm *m_Ascan;

    QCustomPlot *m_Bscan;
    Tlogging *m_logging;
    QLabel *m_status;
    QByteArray m_serverData;
    float m_vel=3250.0;
    float m_velFast = 5890;

    int m_start = 0;
    int m_end = DATA_SIZE /2;

    float m_scanLength = 1.0;
    float m_partThick = 1.0;
    bool encoderTriggerMode = false;
    static void resetEnv();
    int m_currentLine=-1;

    // filter param

    int m_order = 4;
    double m_fc = 5;
    double m_fw = 8;
    float m_fs = 125;

    bool use_bscan = 0;
    double m_thres = 0.0;
    // processor
    Processor *m_processor;
    QEventLoop m_quitEvent;

    // status
    bool acquisitionRunning = false;
    bool connected = false;
    QString m_err {"No Error"};


    // imu
    QVector<qint16 > m_imus{0,0,0};
    qint16 m_imu_x = 0;
    qint16 m_imu_y = 0;
    qint16 m_imu_z = 0;

    quint8 m_chan = 0;

    // tfm

    QHBoxLayout *tfmLayout;
// private functions
private:
    void resetUI();
    void setConnectionIndicator();
    void set_envelope(float, float);
    void toggleOff(QCheckBox *);
    void _rescaleBscan(QCustomPlot *plot, const float &w, const float &h, const float &_w, const float _h);
    void setBscanVisible(bool);
signals:
    void velocitySet(double);
    void mainSendSetting(const QString &);
    void dataReceived(const QList<QPointF> &data);
    void axisTypeChanged(const TChartViewForm::x_AXISTYPE &);
    void axisTypeChanged(const TChartViewForm::y_AXISTYPE &type);
    void acquisitionRun(bool);
    void dataForLogger(const QByteArray &);
    void stopAcqSig();
    void channel(quint8 );
    void filterParam(const quint8 & order, const float &fs, const float &fc, const float &fw);
    void golayCoding(bool, const QString &, const float&, quint8);
    void sendTfmSettings(quint8 channels, quint16 rows, quint16 cols, quint16 samples, float pitch, float offsetX, float offsetY, float resolution, bool required);
};
#endif // MAINWINDOW_H
