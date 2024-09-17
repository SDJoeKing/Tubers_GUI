#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // style
    setStyle(QStyleFactory::create("Windows"));

    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
    ui->log->setEnabled(false);


    // status bar
    m_status= new QLabel(QString::asprintf("Ultrasound Velocity: %.2f m/s", 0.0), this);
    ui->statusBar->addPermanentWidget(m_status);

    // TCP Client
    m_client = new mTcpClient(this);


    // Graph page & Dock widget
    QMainWindow *graphFrame = new QMainWindow();
    graphFrame->setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint|Qt::WindowTitleHint );
    ui->mdiArea->addSubWindow(graphFrame,Qt::Tool | Qt::CustomizeWindowHint|Qt::WindowTitleHint );
    graphFrame->setWindowState(Qt::WindowMaximized);
    graphFrame->show();

    // A/B scan screen splitter - verticle
    QSplitter *_splitter = new QSplitter(Qt::Orientation::Vertical, graphFrame);
    graphFrame->setCentralWidget(_splitter);
    ui->mdiArea->setViewMode(QMdiArea::SubWindowView);

    // A/B-scan dock
    _temp = new QChartView(_splitter);
    _temp2 = new QChartView(_splitter);
    _splitter->addWidget(_temp);
    _splitter->addWidget(_temp2);


    // setting dock
    QDockWidget *settingDock = new QDockWidget(graphFrame,Qt::CustomizeWindowHint );
    settingDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    m_settings = new TSettings();
    settingDock->setWidget(m_settings);
    graphFrame->addDockWidget(Qt::LeftDockWidgetArea, settingDock);


    // logging dock
    QDockWidget *loggingDock = new QDockWidget(graphFrame,Qt::CustomizeWindowHint);
    loggingDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    m_logging = new Tlogging(m_client);
    loggingDock->setWidget(m_logging);
    graphFrame->addDockWidget(Qt::LeftDockWidgetArea, loggingDock);
    loggingDock->setVisible(false);


    // connect
    connect(m_settings, &TSettings::settingReady, this, &MainWindow::doSettingsConfirmed);

    // final finish
    setConnectionIndicator();
    resetUI();

}

void MainWindow::resetUI()
{

    setWindowState(Qt::WindowMaximized);
    ui->log->setEnabled(false);

    // action status
    ui->actionConnection_Status->setChecked(true);
    ui->actionSettings->setChecked(true);
    ui->actionTools->setChecked(true);
    ui->actionLogging->setChecked(false);

    // UI elements
    _temp2->setVisible(false);
    ui->log->clear();

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

}

void MainWindow::doSettingsConfirmed(QString str)
{
    ui->actionSettings->trigger();
    //TBC sending to client
    qDebug() << str;
}

void MainWindow::on_actionLogging_triggered(bool checked)
{
    auto dock = static_cast<QDockWidget *>(m_logging->parent());
    dock->setVisible(checked);
}


void MainWindow::on_ckBscan_clicked(bool checked)
{
    _temp2->setVisible(checked);
}

