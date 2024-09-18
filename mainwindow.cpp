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
    m_status->setObjectName("m_status");
    ui->statusBar->addPermanentWidget(m_status);

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
    m_Ascan = new QChartView(_splitter);
    m_Bscan = new QChartView(_splitter);
    _splitter->addWidget(m_Ascan);
    _splitter->addWidget(m_Bscan);


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

    // TCP Client
    m_client = new mTcpClient(this);

    // QTimer for data request
    m_timer = new QTimer(this);
    updateTimer();
    connect(m_timer, &QTimer::timeout, this, &MainWindow::doRequestData);

    // connect
    connect(m_settings, &TSettings::settingConfirm, this, &MainWindow::doSettingsConfirmed);
    connect(m_client, &mTcpClient::serverReady, ui->frameTools, &QFrame::setEnabled);
    connect(m_client, &mTcpClient::clientMessage, this, &MainWindow::logMsg);
    connect(m_client, qOverload<const QString &>(&mTcpClient::tcpMessage), this, &MainWindow::logMsg);
    connect(m_client, &mTcpClient::serverReady, this, &MainWindow::toogleStatus);

    connect(m_client, &mTcpClient::acquisitionReady, this, &MainWindow::runAcquisition);
    connect(m_client, &mTcpClient::acquisitionStop, this, &MainWindow::stopAcquisition);
    connect(this, &MainWindow::dataReceived, m_client, &mTcpClient::clearData);
    connect(m_client, &mTcpClient::dataReady, this, &MainWindow::doDataReady);

    // final finish
    setConnectionIndicator();
    ui->spinEnvLevel->setValue(0);
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

    // tool bar
    ui->frameTools->setEnabled(false);

    // UI elements
    m_Bscan->setVisible(false);
    ui->log->clear();
    ui->btnRun->setChecked(false);
    ui->btnConnect->setChecked(false);
    ui->spinEnvLevel->setMinimum(0);
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
    m_client->stop();
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
    quint8 _indexVel = str.lastIndexOf(";");
    quint8 _indexHz = str.sliced(0, _indexVel).lastIndexOf(";");

    // update internal timer logic
    auto list = str.split(";");
    auto avg = list[5].toInt();
    auto prf = list[2].toInt();
    auto _interval = list[7].toInt();

    m_minTimerInterval = ((350*(avg-1) + 600) + (1.0/prf*1e6 + 200) *  avg + 350)/1000/0.9; // 10% safety margin
    m_minTimerInterval = qMax(17, m_minTimerInterval);
    m_settings->updateHz(QString("Refresh rate (max %1 Hz)").arg(1.0/m_minTimerInterval*1000, 0, 'f', 1));
    m_timerInterval = 1.0/_interval * 1000;
    updateTimer();

    auto settings = str.sliced(0, _indexHz);
    m_vel = str.sliced(_indexVel+1, str.length() - _indexVel -1).toFloat();

    auto _status = ui->statusBar->findChild<QLabel *>("m_status");
    if(_status)
    {
        _status->setText(QString("Ultrasound Velocity: %1 m/s").arg(m_vel));
    }

    if(ui->btnRun->isChecked())
    {
        ui->btnRun->click();
        connect(m_client, &mTcpClient::settingReady, ui->btnRun, &QPushButton::click);
        QTimer::singleShot(130, this, [&](){m_client->sendSetting(settings);});
        QTimer::singleShot(200, this, [&](){disconnect(m_client, &mTcpClient::settingReady, ui->btnRun, &QPushButton::click);});
    }else
    {
        m_client->sendSetting(settings);
    }
}

void MainWindow::on_actionLogging_triggered(bool checked)
{
    auto dock = static_cast<QDockWidget *>(m_logging->parent());
    dock->setVisible(checked);
}

void MainWindow::on_ckBscan_clicked(bool checked)
{
    m_Bscan->setVisible(checked);
}

void MainWindow::on_btnConnect_clicked(bool checked)
{
    ui->log->clear();
    if(checked)
    // attemp to make connections to the server
    {
        QString address = ui->ipAddress->text().simplified().replace(" ", "");
        quint8 port = ui->port->text().toInt();
        m_client->start(address, port);
        m_client->flush();
    }
    else
    {
        // disconnect
        m_timer->stop(); //stop timer immediately to avoid further commands
        QTimer::singleShot(100, m_client, &mTcpClient::stopAcquisition);
        QTimer::singleShot(101, m_client, &mTcpClient::stop);
        m_client->flush();
        resetUI();
    }
}

void MainWindow::doRequestData()
{
    m_client->requestData();
}

void MainWindow::updateTimer()
{
    m_timerInterval = qMax(m_timerInterval, m_minTimerInterval);
    m_timer->setInterval( m_timerInterval);
}

void MainWindow::logMsg(QString str)
{
    auto _dateTime = QDateTime::currentDateTime();
    QString _prefix = _dateTime.toString("[dd/MM/yyyy hh:mm:ss]\n");
    ui->log->appendPlainText(_prefix + "    " + str);
}

void MainWindow::toogleStatus(bool arg)
{
    ui->radioStatus->setChecked(arg);

    if(ui->btnConnect->isChecked() && arg)
    {
        ui->btnConnect->setText("Disconnect");
    }else
    {
        ui->btnConnect->setText("Connect");
    }
}

void MainWindow::runAcquisition()
{
    ui->btnRun->setText("Stop");
}

void MainWindow::stopAcquisition()
{
    ui->btnRun->setText("Run");
}

void MainWindow::doDataReady()
{
    m_serverData = QByteArray::fromRawData(m_client->data(), mTcpClient::DATA_SIZE);

    int j=0;
    qfloat16 xpoint=0;
    QList<QPointF> calPoint(mTcpClient::DATA_SIZE/2);
    quint16 temp1;
    quint16 temp2;
    float dataPoint;

    for (int i = 0; i < mTcpClient::DATA_SIZE/2; i++)
    {
        temp1 =(m_serverData[j + 1] << 8) & 0xFF00;
        temp2 = (m_serverData[j]) & 0xFF;
        dataPoint = static_cast<qint16>(temp2 | temp1)/ 32768.0  * 3.18 * 1.0 *1000.0;

        xpoint = i;

        if(ui->ckDepthAxis->isChecked())
            xpoint = i/2.0/125e6 * m_vel * 1000;

        if(ui->ckRectify->isChecked())
        {
            dataPoint = qAbs(dataPoint);
            dataPoint = envelope(dataPoint);
        }
        calPoint[i] = QPointF(xpoint, dataPoint);

        j += 2;
    }

    emit dataReceived();

    // !## Need to implement interface with A-scan and B-scan class;
}

void MainWindow::on_btnRun_clicked(bool checked)
{
    if(checked)
    {
        emit dataReceived(); // clear client data buffer
        m_client->startAcquisition();
        updateTimer();
        m_timer->start();

    }else
    {
        m_timer->stop(); //stop timer immediately to avoid further commands
        QTimer::singleShot(100, m_client, &mTcpClient::stopAcquisition);
        m_client->flush();
    }

}

void MainWindow::set_envelope(float attack, float release)
{
    m_ga = attack < 1e-20 ? 0 : qExp(-1.0 / (attack * 125e6));
    m_gr = attack < 1e-20 ? 0 : qExp(-1.0 / (attack * 125e6));

}

qfloat16 MainWindow::envelope(qfloat16 sample)
{
    auto s = qAbs(sample);
    qfloat16 _env;
    return  _env = _env < s ? m_ga * _env + (1 - m_ga) * s : m_gr * _env + (1 - m_gr) * s;
}

void MainWindow::on_spinEnvLevel_valueChanged(int arg1)
{
    qfloat16 _release = arg1 * 125 /10.0f;
    set_envelope(0.01f, _release );
}

