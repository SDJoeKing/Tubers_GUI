#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
    // action status
    ui->actionConnection_Status->setChecked(true);
    ui->actionSettings->setChecked(true);
    ui->actionTools->setChecked(true);
    ui->actionLogging->setChecked(false);

    // Graph page & Dock widget
    QMainWindow *graphFrame = new QMainWindow();
    graphFrame->setWindowFlags(Qt::FramelessWindowHint);
    ui->mdiArea->addSubWindow(graphFrame, Qt::CustomizeWindowHint);

    _temp = new QChartView(graphFrame);
    graphFrame->setCentralWidget(_temp);
    graphFrame->setWindowTitle("A-scan Monitor");

    ui->mdiArea->setViewMode(QMdiArea::TabbedView);
    ui->mdiArea->setTabsClosable(false);

    // setting dock
    QDockWidget *settingDock = new QDockWidget(graphFrame,Qt::CustomizeWindowHint );
    settingDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    m_settings = new TSettings();
    settingDock->setWidget(m_settings);
    graphFrame->addDockWidget(Qt::LeftDockWidgetArea, settingDock);
    m_settings->setVisible(true);

    // logging dock
    QDockWidget *loggingDock = new QDockWidget(graphFrame,Qt::CustomizeWindowHint);
    loggingDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    m_logging = new Tlogging();
    settingDock->setWidget(m_logging);
    graphFrame->addDockWidget(Qt::RightDockWidgetArea, settingDock);
    m_logging->setVisible(false);

    // connect
    connect(m_settings, &TSettings::settingReady, this, &MainWindow::doSettingsConfirmed);

    // final finish
    setConnectionIndicator();
    resetUI();

}

void MainWindow::resetUI()
{

}
void MainWindow::setConnectionIndicator()
{
    ui->radioStatus->setStyleSheet(
        "QRadioButton::indicator {"
        "width:                  10px;"
        "height:                 10px;"
        "        border-radius:          7px;"
        "}"
        "QRadioButton::indicator:checked {"
        "background-color:       green;"
        "border:                 2px solid white;"
        "}"
        "QRadioButton::indicator:unchecked {"
        "background-color:       red;"
        "border:                 2px solid white;"
        "}"
        );
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionConnection_Status_triggered(bool checked)
{
    ui->frameConnect->setVisible(checked);
}

void MainWindow::on_actionTools_triggered(bool checked)
{
    ui->frameTools->setVisible(checked);
}





void MainWindow::on_actionSettings_triggered(bool checked)
{
    auto dock = static_cast<QDockWidget *>(m_settings->parent());
    dock->setVisible(checked);
    m_settings->setVisible(checked);
}

void MainWindow::doSettingsConfirmed(QString str)
{
    ui->actionSettings->trigger();
    //TBC sending to client
    qDebug() << str;
}

