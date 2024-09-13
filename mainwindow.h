#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QStyleFactory>
#include <QMessageBox>
#include <QTimer>
#include <QtCharts>
#include <QValueAxis>
#include <QLineSeries>
#include <QInputDialog>
#include <QEvent>
#include <QWheelEvent>

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

private:
    Ui::MainWindow *ui;
    mTcpClient *m_client;
    QTimer *m_timer;
    TSettings *m_settings;
    QChartView *_temp;
    Tlogging *m_logging;
    QLabel *m_status;
// private functions
private:
    void resetUI();
    void setConnectionIndicator();
};
#endif // MAINWINDOW_H
