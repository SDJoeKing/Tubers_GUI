#include "mainwindow.h"
#include "./ui_mainwindow.h"

double MainWindow::_env=0;
double MainWindow::m_ga = 0;
double MainWindow::m_gr = 0;
static int bscanUpdateOnce = 0;
QElapsedTimer timer;

using namespace Eigen;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
    // setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
    setWindowFlags(  Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint) ;
    ui->log->setEnabled(false);
    ui->ckBscan->setEnabled(false);

    // status bar
    m_status= new QLabel(QString::asprintf("Ultrasound Velocity: %.2f m/s", m_vel), this);
    m_status->setObjectName("m_status");
    ui->statusBar->addPermanentWidget(m_status);
    QLabel *fpsBar = new QLabel(this);
    fpsBar->setObjectName("fpsBar");
    QLabel *plotRateBar = new QLabel(this);
    plotRateBar->setObjectName("plotRateBar");

    ui->statusBar->addWidget(fpsBar);
    ui->statusBar->addWidget(plotRateBar);

    // Graph page & Dock widget
    QMainWindow *graphFrame = new QMainWindow();
    ui->mdiArea->addSubWindow(graphFrame,Qt::Tool|Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    graphFrame->setWindowState(Qt::WindowState::WindowMaximized);
    graphFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    graphFrame->show();

    // A/B scan screen splitter - verticle
    QSplitter *_splitter = new QSplitter(Qt::Orientation::Vertical, graphFrame);
    graphFrame->setCentralWidget(_splitter);
    ui->mdiArea->setViewMode(QMdiArea::SubWindowView);

    // A/B-scan dock
    m_Ascan = new TChartViewForm(_splitter);
    m_Bscan = new QCustomPlot(_splitter);
    // B-scan uses openGL support
    // m_Bscan->setOpenGl(true);

    // configure B-scan
    // m_Bscan->addGraph()
    m_Bscan->yAxis->setRangeReversed(true);
    QCPColorMap *_colorMap = new QCPColorMap(m_Bscan->xAxis, m_Bscan->yAxis);

    m_Bscan->setContextMenuPolicy(Qt::CustomContextMenu);
    // add a color scale:
    QCPColorScale *colorScale = new QCPColorScale(m_Bscan);
    // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    _colorMap->setColorScale(colorScale); // associate the color map with the color scale
    _splitter->addWidget(m_Ascan);

    QWidget *tfmArea = new QWidget(this);
    tfmArea->setObjectName("tfmArea");
    tfmLayout = new QHBoxLayout;
    tfmArea->setLayout(tfmLayout);
    tfmLayout->addWidget(m_Bscan);

    _splitter->addWidget(m_Ascan);
    _splitter->addWidget(tfmArea);
    _splitter->setSizes(QList<int>(height()-30, height()));

    connect(m_Bscan, &QCustomPlot::afterReplot, this, &MainWindow::rescaleBscan);

    // setting dock
    QDockWidget *settingDock = new QDockWidget(graphFrame,Qt::CustomizeWindowHint|Qt::FramelessWindowHint );
    settingDock->setFeatures(QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);
    m_settings = new TSettings();
    settingDock->setWidget(m_settings);
    graphFrame->addDockWidget(Qt::LeftDockWidgetArea, settingDock);


    // logging dock
    QDockWidget *loggingDock = new QDockWidget(graphFrame,Qt::CustomizeWindowHint);
    loggingDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    m_logging = new Tlogging();
    loggingDock->setWidget(m_logging);
    graphFrame->addDockWidget(Qt::RightDockWidgetArea, loggingDock);
    loggingDock->setVisible(false);

    // processor
    m_processor = new Processor();
    m_processor->updateFilter(m_order, m_fs, m_fc, m_fw);
    m_processor->moveToThread(&processorThread);
    connect(&processorThread, &QThread::started, m_processor, &Processor::run);
    // processor class
    connect(ui->ckDepthAxis, &QCheckBox::clicked, m_processor, &Processor::setDepth, Qt::QueuedConnection);
    connect(ui->ckRectify, &QCheckBox::clicked, m_processor, &Processor::setRectified, Qt::QueuedConnection);
    connect(ui->ckFilter, &QCheckBox::clicked, m_processor, &Processor::setFiltering, Qt::QueuedConnection);
    connect(m_processor, &Processor::dataLogger, m_logging, &Tlogging::setData, Qt::QueuedConnection);
    connect(this, &MainWindow::filterParam, m_processor, &Processor::updateFilter, Qt::QueuedConnection);
    connect(this, &MainWindow::velocitySet, m_processor, &Processor::setVel, Qt::QueuedConnection);

    connect(m_processor, &Processor::dataProcessed, m_Ascan, &TChartViewForm::plot, Qt::QueuedConnection);

    connect(m_processor, &Processor::tfmReady, this, &MainWindow::updateBScan);
    connect(this, &MainWindow::sendTfmSettings, m_processor, &Processor::updateTfmSetting);

    connect(m_processor, &Processor::sendHeaderInfo, this, &MainWindow::updateHeaderInfo, Qt::QueuedConnection);
    connect(this, &MainWindow::golayCoding, m_processor, &Processor::updateGolaySetting, Qt::QueuedConnection);

    connect(&processorThread, &QThread::finished, this, &MainWindow::threadFinished);
    processorThread.start();

    // connect settings
    connect(m_settings, &TSettings::settingConfirm, this, &MainWindow::doSettingsConfirmed);
    connect(m_settings, &TSettings::settingHide, this, &MainWindow::hideSetting);

    // acquisition related

    connect(this, &MainWindow::velocitySet, m_Ascan, &TChartViewForm::setVelocity);
    connect(this, qOverload<const TChartViewForm::x_AXISTYPE &>(&MainWindow::axisTypeChanged), m_Ascan,&TChartViewForm::changeXAxisType);
    connect(this, qOverload<const TChartViewForm::y_AXISTYPE &>(&MainWindow::axisTypeChanged), m_Ascan,&TChartViewForm::changeYAxisType);


    // key acquisitionRun or not
    connect(m_settings, &TSettings::fsChanged, this, &MainWindow::updateFs);
    connect(m_settings, &TSettings::fsChanged, m_Ascan, &TChartViewForm::updateFs);
    connect(this, &MainWindow::acquisitionRun, m_Ascan, &TChartViewForm::acquisitionStatus);
    connect(this, &MainWindow::acquisitionRun, m_Ascan, &TChartViewForm::toggleSave);

    connect(ui->ckGates, &QCheckBox::checkStateChanged, m_Ascan, &TChartViewForm::startThickCal);
    connect(ui->ckGates, &QCheckBox::checkStateChanged, ui->btnCal, &QPushButton::setEnabled);
    connect(m_Ascan, &TChartViewForm::scaleSet, m_processor, &Processor::updateScale);
    connect(m_Ascan, &TChartViewForm::setRectifyCheck, this, &MainWindow::setRectifyChecked);
    connect(m_Ascan, &TChartViewForm::calculatedThickness, ui->spinDepth, &QDoubleSpinBox::setValue);
    connect(m_Bscan, &QCustomPlot::customContextMenuRequested, this, &MainWindow::bScanCustomContext);
    connect(m_settings, &TSettings::bScanSetting, this, &MainWindow::do_bScanSetting);

    connect(m_Ascan, &TChartViewForm::sendThreshold, this, &MainWindow::setThreshold);

    // setting/logging related
    connect(m_settings,  &TSettings::badSettings, this, [&](){emit stopAcqSig(); });
    connect(this, &MainWindow::velocitySet, m_settings, &TSettings::updateVel);
    // tcpclient
    m_client = nullptr;

    // test tcpserver
#ifdef TEST_SERVER
    m_server = new testServer();

    // run server
    m_serverThread = new QThread(this);
    m_server->moveToThread(m_serverThread);
    connect(m_serverThread, &QThread::started, m_server, &testServer::run);
    connect(m_serverThread, &QThread::finished, m_server, &testServer::stop);
    m_serverThread->start();

#else
    m_server = nullptr;
#endif

    // timer for updating info
    m_updateTimer.setInterval(500); // this is the status request timer
    timer.start(); // this is the bscan plot update timer

    // final finish
    setConnectionIndicator();
    ui->spinEnvLevel->setValue(0);
    ui->radioTemp->setText(QString::asprintf("Temperature: %.1f \u2103", 0.0));
    ui->radioError->setText(QString("Error Status"));
    resetUI();

}

void MainWindow::resetUI()
{

    ui->log->setEnabled(false);
    ui->btnCal->setEnabled(false);

    // action status
    ui->actionConnection_Status->setChecked(true);

    if(!ui->actionSettings->isChecked())
        ui->actionSettings->trigger();

    ui->actionTools->setChecked(true);
    ui->frameTools->setVisible(true);

    ui->actionLogging->setChecked(false);

    // UI elements

    if(ui->btnRun->isChecked())
        ui->btnRun->setChecked(false);

    setBscanVisible(false);
    ui->log->clear();
    ui->btnConnect->setChecked(false);
    ui->spinEnvLevel->setMinimum(0);
    ui->spinEnvLevel->setValue(0);

    toggleOff(ui->ckRectify);// so that by default rectify
    ui->ckRectify->click();
    toggleOff(ui->ckFilter);// so that by default rectify
    ui->ckFilter->click();

    toggleOff(ui->ckDepthAxis);
    toggleOff(ui->ckGates);

    ui->spinDepth->setValue(0.00);
    m_settings->disableScroll(false);

    m_Ascan->clear();
    m_Ascan->reset();

    ui->labelSpeed->setText("Ethernet Speed:");
    ui->radioTemp->setText(QString::asprintf("Temperature: %.1f \u2103", 0.0));
    ui->radioError->setText(QString("Error Status"));
    ui->radioError->setChecked(false);
    ui->radioStatus->setText("Not Connected");

    // IMU label:
    ui->btnImuBase->setChecked(false);
    ui->btnImuBase->setEnabled(false);
    ui->label_IMU_X->setText(getImuLabel("X", 0));
    ui->label_IMU_Y->setText(getImuLabel("Y", 0));
    ui->label_IMU_Z->setText(getImuLabel("Z", 0));

    // update timer
    m_updateTimer.stop();
}

void MainWindow::toggleOff(QCheckBox *widget)
{
    if(widget->isChecked())
        widget->click();
}

void MainWindow::setConnectionIndicator()
{
    ui->radioStatus->setStyleSheet(LED_NETCONNECTED_STYLE);

    ui->radioTemp->setStyleSheet(LED_NONCONNECT_STYLE);

    ui->radioError->setStyleSheet(LED_NONCONNECT_STYLE);
}

MainWindow::~MainWindow()
{

    if(ui->btnConnect->isChecked())
    {
        m_client->flush();
        m_client->stop();
        socketThread.quit();
    }

    QTimer::singleShot(0, m_processor, &Processor::close);
    processorThread.quit();

    m_quitEvent.exec();

#ifdef TEST_SERVER
    m_serverThread->quit();
#endif

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

void MainWindow::hideSetting()
{
    ui->actionSettings->trigger();
}

void MainWindow::doSettingsConfirmed(QString str)
{

    // update internal ->s logic

    auto list = str.split(";");

    m_vel = list[SETTINGS::velocity].toDouble();
    emit velocitySet(m_vel);

    m_order = list[SETTINGS::order].toInt();
    m_fc = (list[SETTINGS::lowCut].toDouble() + list[SETTINGS::highCut].toDouble() )/ 2.0;
    m_fw= qAbs(list[SETTINGS::highCut].toDouble() - list[SETTINGS::lowCut].toDouble());

    emit filterParam(m_order, m_fs, m_fc, m_fw);


    int _size = 0;
    for(int i = SETTINGS::txChannel ; i< SETTINGS::adcClkDiv + 1; i++)
        _size+=list[i].size()+1; // including the separator size

    auto settings = str.sliced(0, _size );

    qDebug() << settings;
    auto _status = ui->statusBar->findChild<QLabel *>("m_status");
    if(_status)
    {
        _status->setText(QString("Ultrasound Velocity: %1 m/s").arg(m_vel));
    }

    // encoder mode?
    encoderTriggerMode = list[SETTINGS::encoderTriggering].toUInt();
    m_settings->encoderTriggerMode(encoderTriggerMode);

    // for motor testing only
    if(encoderTriggerMode && acquisitionRunning)
    {

        ui->btnRun->click();
        QTimer::singleShot(50, this, [&](){ui->btnRun->click();});
        return;
    }

    emit golayCoding(list[SETTINGS::golay].toInt(), list[SETTINGS::pulseSequence], list[SETTINGS::pulseFreq].toFloat(), m_settings->pulseLength());

    emit mainSendSetting(settings);


}

void MainWindow::on_actionLogging_triggered(bool checked)
{
    auto dock = static_cast<QDockWidget *>(m_logging->parent());
    dock->setVisible(checked);
}

void MainWindow::on_ckBscan_clicked(bool checked)
{
    setBscanVisible(checked);
}

void MainWindow::on_btnConnect_clicked(bool checked)
{
    ui->log->clear();
    if(checked)
    // attemp to make connections to the server
    {

        // TCP Client
        m_client = new mTcpClient();

        connect(m_client, &mTcpClient::clientMessage, this, &MainWindow::logMsg, Qt::QueuedConnection);
        connect(m_client, qOverload<const QString &>(&mTcpClient::tcpMessage), this, &MainWindow::logMsg, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::serverReady, this, &MainWindow::toggleStatus, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::serverReady, ui->radioStatus, &QRadioButton::setChecked, Qt::QueuedConnection);
        connect(this, &MainWindow::stopAcqSig, m_client, &mTcpClient::setStopAcq, Qt::QueuedConnection);
        connect(this, &MainWindow::channel, m_client, &mTcpClient::setChannel, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::acquisitionReady, this, &MainWindow::runAcquisition, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::acquisitionStop, this, &MainWindow::stopAcquisition, Qt::QueuedConnection);
        connect(this, &MainWindow::dataReceived, m_client, &mTcpClient::clearData, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::dataReady, m_processor, &Processor::process, Qt::QueuedConnection);
        connect(  m_processor, &Processor::requestAcq, m_client, &mTcpClient::writeAcq, Qt::QueuedConnection);

        connect(m_client, &mTcpClient::connectFail, this, [this](){ui->btnConnect->setChecked(false); connected=false;
                QMessageBox::information(this, "Error", "Unable to make connection to server. Please check connection.");}, Qt::QueuedConnection);
        connect(this, &MainWindow::mainSendSetting, m_client, &mTcpClient::sendSetting, Qt::QueuedConnection);
        // connect fps
        connect(m_client, &mTcpClient::fps, this, &MainWindow::do_fps, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::plotRate, this, &MainWindow::do_plotRate, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::settingReady, this, &MainWindow::do_settingReady, Qt::QueuedConnection);
        connect(m_settings, &TSettings::prf, m_client, &mTcpClient::setPrf);
        // connect error handling
        connect(m_client, &mTcpClient::errorOccured, this, &MainWindow::do_ConnectLost, Qt::QueuedConnection);
        connect(m_client,  &mTcpClient::badSettings, this, &MainWindow::do_badSettings);

        QString address = ui->ipAddress->text().simplified().replace(" ", "");
        quint16 port = ui->port->text().toInt();

#ifdef TEST_SERVER
        m_client->setHostPort("127.0.0.1", 5000);

#else
        m_client->setHostPort(address, port);

#endif
        m_client->moveToThread(&socketThread);
        qDebug() << "m_client thread: "<< m_client->thread();
        connect(&socketThread, &QThread::started, m_client, &mTcpClient::run);
        socketThread.start();
        ui->btnImuBase->setEnabled(true);
    }
    else
    {

        // disconnect

        ui->radioStatus->setChecked(false);
        QTimer::singleShot(0, m_client, &mTcpClient::stopAcquisition);
        QTimer::singleShot(100, m_client, &mTcpClient::stop);

        resetUI();
    }
}

void MainWindow::logMsg(QString str)
{
    auto _dateTime = QDateTime::currentDateTime();
    QString _prefix = _dateTime.toString("[dd/MM/yyyy hh:mm:ss]\n");
    ui->log->append(_prefix + "    " + str);
}

void MainWindow::toggleStatus(bool arg)
{

    qDebug() << "radio status: " << arg;
    if(ui->btnConnect->isChecked() && arg)
    {
        connected=true;
        ui->btnConnect->setText("Disconnect");
        ui->radioError->setStyleSheet(LED_CONNECTED_STYLE);
        ui->radioTemp->setStyleSheet(LED_CONNECTED_STYLE);
        ui->radioStatus->setText("Connected");
        //UPDATE TIMER
        m_updateTimer.start();
        connect(&m_updateTimer, &QTimer::timeout, m_client, &mTcpClient::requestStatus, Qt::QueuedConnection);

    }else
    {
        connected=false;
        ui->btnConnect->setText("Connect");
        ui->radioError->setStyleSheet(LED_NONCONNECT_STYLE);
        ui->radioTemp->setStyleSheet(LED_NONCONNECT_STYLE);
        ui->radioStatus->setText("Not Connected");
        socketThread.quit();
    }
}

void MainWindow::runAcquisition()
{
    emit acquisitionRun(true);
    acquisitionRunning = true;
    // m_settings->disableScroll(true);

    ui->btnRun->setText("Stop");

}

void MainWindow::stopAcquisition()
{
    emit acquisitionRun(false);
    acquisitionRunning = false;

    ui->btnRun->setText("Run");
    ui->btnRun->setChecked(false);
}

double MainWindow::envelope(double sample, double &value, double ga, double gr)
{
    auto s = qAbs(sample);
    value += (s - value) * (s > value ?  ga :  gr);
    return value;
}

void MainWindow::on_btnRun_clicked(bool checked)
{

    ui->btnRun->setChecked(false);

    if(connected)
    {
        if(checked && !acquisitionRunning)
        {
            // send current settings to hardware
            ui->btnRun->setChecked(true);
            m_settings->sendSetting();

        }else
        {

            if(encoderTriggerMode)
            {
                QTimer::singleShot(10, m_client, [&](){m_client->writeData("stop");});
                return;
            }
            emit stopAcqSig();
            QTimer::singleShot(0, m_client, [&](){m_client->flush();});
        }
    }

}



void MainWindow::set_envelope(float attack, float release)
{
    m_ga = attack < 1e-20 ? 0 :1 - qExp(-1.0 / (attack * 125e6));
    m_gr = attack < 1e-20 ? 0 :1 - qExp(-1.0 / (release * 125e6));
}

void MainWindow::on_spinEnvLevel_valueChanged(int arg1)
{
    double _release = arg1 * 1e-7;
    set_envelope(1e-7, _release );
}


void MainWindow::on_ckGates_clicked(bool checked)
{
    m_Ascan->toggleGates(checked);
}


void MainWindow::on_ckDepthAxis_clicked(bool checked)
{

    if(checked)
        emit axisTypeChanged(TChartViewForm::DEPTH);
    else
        emit axisTypeChanged(TChartViewForm::TIME);
}


void MainWindow::resetEnv()
{
    _env=0;
}

void MainWindow::on_ckRectify_clicked(bool checked)
{

    if(checked)
    {
        emit axisTypeChanged(TChartViewForm::RECTIFY);
        ui->spinEnvLevel->setValue(1);
    }
    else
        emit axisTypeChanged(TChartViewForm::FULL);

}

int findFrontWall(const QList<QPointF> &data, int start, int end)
{
    int _max = start; // in water less than 2mm
    int _end = DATA_SIZE/2 < end ? DATA_SIZE/2 : end;

    double value = 0;
    for(int i = _max; i<_end; i++)
    {
        auto _v = qAbs(data[i].y());
        if(_v >= value)
        {
            value = _v;
            _max = i;
        }
    }
    return _max;
}


void MainWindow::updateBScan(const ArrayXXf &tfm_result)
{


    if(!use_bscan)
        return;



    auto _colorMap = static_cast<QCPColorMap *>(m_Bscan->plottable());
    int valueSize = _colorMap->data()->valueSize();
    // functions to find the first front wall peaks

    int _start = 0;



    _colorMap->rescaleAxes();
    _colorMap->setGradient(QCPColorGradient::gpJet);

    auto nx = tfm_result.cols();
    auto ny = tfm_result.rows();

    QCPColorMap *_map = static_cast<QCPColorMap *>(m_Bscan->plottable());

    auto key = _map->data()->keySize();
    auto value = _map->data()->valueSize();

    if(key != nx || value != ny)
    {
        ui->btnConnect->click(); // disconnect
        QMessageBox::critical(this,  "Critical Error", QString("The TFM array shape (%1, %2) does not match ColorMap (%3, %4)").arg(ny).arg(nx).arg(value).arg( key));
    }

    for(int row = 0; row< value; row++)
    {
        for(int col = 0; col < key; col++)
            _map->data()->setCell(col, row, qAbs(tfm_result(row, col)));
    }
    _map->setGradient(QCPColorGradient::gpJet);
    _map->rescaleDataRange();
    _map->rescaleAxes();
    m_Bscan->replot(QCustomPlot::rpQueuedRefresh);


    // if(timer.hasExpired(30))
    //     {
    //         timer.restart();
    //         m_Bscan->replot(QCustomPlot::rpQueuedRefresh);
    //     }


    // m_Bscan->setUpdatesEnabled(false);
    // auto f = QtConcurrent::run(QThreadPool::globalInstance(), &QCustomPlot::replot, m_Bscan, QCustomPlot::rpQueuedRefresh );
    // f.waitForFinished();
    // m_Bscan->setUpdatesEnabled(true);

    QTimer::singleShot(0, m_client, [&](){m_client->writeData(ack);}); // resume acquisition

}


void MainWindow::on_actionReset_triggered(bool checked)
{
    resetUI();
}

void MainWindow::setRectifyChecked()
{
    ui->ckRectify->click();
}

void MainWindow::bScanCustomContext(const QPoint &pos)
{
    QMenu _tempMenu(this);
    QAction _tempAction("Save B-Scan", this);
    _tempMenu.addAction(&_tempAction);
    connect(&_tempAction, &QAction::triggered, this, [this]()\
    {
        QPixmap _bscan = m_Bscan->grab();
        QString path = QFileDialog::getSaveFileName(this, "Save Figure", QApplication::applicationDirPath(), "Image (*.png *.jpg)");
        bool success = false;
        success = _bscan.save(path);
        if(!success && !path.isEmpty())
            QMessageBox::warning(this, "Warning", "Not able to save the image");
    });

    _tempMenu.exec(m_Bscan->mapToGlobal(pos));
}



void MainWindow::on_btnCal_clicked()
{
    bool ok = false;
    // ui->btnRun->toggle();
    auto newDepth = QInputDialog::getDouble(this, "Please input true thickness", "Thickness (mm): ", 0, 0, 5000.0, 2, &ok);
    qDebug() << "\n\n\n Old Depth " << newDepth;
    if(ok)
    {
        auto oldDepth = ui->spinDepth->value();

        if(oldDepth == 0)
            return;

        auto distance = oldDepth * 2 / 1000 * 125e6 / m_vel;
        qDebug() << "\n\n\n New Depth " << distance; // tbc
        m_vel = newDepth * 2/1000 * 125e6 / distance;

        if(m_vel<=0)
            return;

        auto _status = ui->statusBar->findChild<QLabel *>("m_status");

        if(_status)
        {
            _status->setText(QString("Ultrasound Velocity: %1 m/s").arg(m_vel));
        }

        emit velocitySet(m_vel);
    }
}

void MainWindow::do_bScanSetting(bool arg, const QList<double> &settings)
{
    use_bscan = arg;
    ui->ckBscan->setEnabled(use_bscan);

    if(use_bscan)
    {
        setBscanVisible(true);
        ui->ckBscan->setCheckState(Qt::Checked);
    }else
    {
        setBscanVisible(false);
        ui->ckBscan->setCheckState(Qt::Unchecked);
    }

    if(use_bscan)
    {


        m_Bscan->xAxis->setLabel("TFM Width [mm]");
        m_Bscan->yAxis->setLabel("TFM Height [mm]");

        QCPColorMap * _map = static_cast<QCPColorMap *>(m_Bscan->plottable());

        // clear graph for replot;
        _map->data()->fill(0);
        bscanUpdateOnce = 0;
        // reset front head to -1
        m_currentLine = -1;
        if(_map)
        {
            float pitch = settings[0];
            float width = settings[1];
            float height = settings[2];
            float offsetX = settings[3];
            float offsetY = settings[4];
            int samples = static_cast<int>(settings[5]);
            float resolution = settings[6];
            int channels = settings[7];
            bool required = static_cast<int>(settings[8]);
            m_chan = channels - 1;

            emit channel(m_chan);

            // x/y axis array size

            int nx = width / resolution;
            int ny = height/ resolution;

            tfmHeight = ny;
            tfmWidth = nx;

            emit sendTfmSettings(m_chan, ny, nx, samples, pitch, offsetX, offsetY, resolution, required);

            _map->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
            _map->data()->setRange(QCPRange(0, width), QCPRange(0, height));
            _map->setGradient(QCPColorGradient::gpJet);
            _map->rescaleDataRange();
            _map->rescaleAxes();
            m_Bscan->replot(QCustomPlot::rpImmediateRefresh);

        }
    }
}

void MainWindow::setThreshold(double thres)
{
    m_thres = thres;
}


QString ErrorMsg(quint8 code)
{
    QString msg = "";
    QMap<quint8, QString> checkTable
        {
            {0, "Pulser OverTemp"},
            {1, "FPGA OverTemp"},
            {2, "Motor Fault"},
            {3, "Over PRF"},
            {4, "Comm Err"},
            {5, "Setup Err"},
            {6, "Acquisition Err"},
            {7, "HV supply Err"}
        };

    if(code != 0)
    {
        for(quint8 i = 0; i<8; i++)
        {
            if( ((code>>i) & 1) )
            {
                msg+=checkTable[i] + "|";
            }
        }

        return msg.slice(0, msg.size()-1);
    }
    return "No Error";
}

void MainWindow::updateHeaderInfo(const float &temp, const float &speed, const quint8  &errorCode, const QVector<qint16>& imus)
{
    ui->radioTemp->setText(QString::asprintf("Temperature: %.1f \u2103", temp));
    ui->labelSpeed->setText(QString("Ethernet Speed: %1 Mbits/s").arg(speed));

    m_imus = imus;

    ui->label_IMU_X->setText(getImuLabel("X", imus.at(0) - m_imu_x));
    ui->label_IMU_Y->setText(getImuLabel("Y", imus.at(1) - m_imu_y));
    ui->label_IMU_Z->setText(getImuLabel("Z", imus.at(2) - m_imu_z));

    QString errMessage = ErrorMsg(errorCode);
    ui->radioError->setText(QString("Error: %1").arg(errMessage));

    if(m_err!=errMessage)
    {
        logMsg(QString("Error Status: ") + errMessage);
        m_err = errMessage;
    }

    if(temp > 70.0 )
        ui->radioTemp->setChecked(true);
    else
        ui->radioTemp->setChecked(false);

    if(errorCode != 0)
        ui->radioError->setChecked(true);
    else
    {
        ui->radioError->setChecked(false);
        ui->radioError->setText(QString("No Error"));
    }
}

void MainWindow::do_badSettings()
{
    stopAcquisition();
}

void MainWindow::do_settingReady()
{
    qDebug() << ui->btnRun->isChecked() << acquisitionRunning;
    if(ui->btnRun->isChecked() && !acquisitionRunning)
        QTimer::singleShot(0, m_client, [&](){m_client->startAcquisition();});
}


void MainWindow::updateFs(double newFs)
{
    m_fs = newFs;
    emit filterParam(m_order, m_fs, m_fc, m_fw);
}

void MainWindow::do_fps(float fps)
{
    auto fpsBar = ui->statusBar->findChild<QLabel *>("fpsBar");
    if(fpsBar)
    {
        fpsBar->setText(QString::asprintf("Data Rate: %.1f", fps));
    }
}

void MainWindow::do_plotRate(const float &fps)
{
    auto plotRateBar = ui->statusBar->findChild<QLabel *>("plotRateBar");
    if(plotRateBar)
    {
        plotRateBar->setText(QString::asprintf("Plot FPS: %.1f", fps));
    }
}

void MainWindow::do_ConnectLost()
{
    // disconnect
    ui->radioStatus->setChecked(false);
    stopAcquisition();
    resetUI();
}

void MainWindow::threadFinished()
{
    m_quitEvent.quit();
}


void MainWindow::on_btnImuBase_toggled(bool checked)
{
    if(checked)
    {
        ui->btnImuBase->setText("Reset");
        m_imu_x = m_imus.at(0);
        m_imu_y = m_imus.at(1);
        m_imu_z = m_imus.at(2);
    }
    else
    {
        ui->btnImuBase->setText("Set Baseline");
        m_imu_x = 0;
        m_imu_y = 0;
        m_imu_z = 0;
    }
}

void MainWindow::_rescaleBscan(QCustomPlot *plot, const float &w, const float &h, const float &_w, const float _h)
{
    // calculate bscan new area and move to center
    float widthRatio = _w/w;
    float heightRatio = _h/h;

    if(int(heightRatio * 100) == int( widthRatio * 100))
        return;

    // prioritise y axis
    if(heightRatio > widthRatio)
    {

        float targetWidth = h * _w / _h;
        tfmLayout->setContentsMargins(w/2 - targetWidth / 2, 0, w/2 - targetWidth/2, 0);
    }
    // prioritise x axis
    else
    {
        float targetHeight = w * _h / _w ;
        tfmLayout->setContentsMargins(0, h/2 - targetHeight/2, 0, h/2 - targetHeight/2);
    }

    // m_Bscan->replot(QCustomPlot::rpQueuedReplot);
}

void MainWindow::setBscanVisible(bool visible)
{
    m_Bscan->setVisible(visible);
    QWidget *widget = qobject_cast<QSplitter*>(m_Ascan->parent())->findChild<QWidget *>("tfmArea");
    if(widget != nullptr)
        widget->setVisible(visible);
}

void MainWindow::rescaleBscan()
{

    // compare if the bscan has already been rendered (size not 0)
    auto totalRect = qobject_cast<QSplitter*>(m_Ascan->parent())->childrenRect();
    auto ascanRect = m_Ascan->rect();

    if(ascanRect.height() == totalRect.height()) // bscan has not been rendered yet
        return;

    float height = totalRect.height() - ascanRect.height();
    float width  = totalRect.width();

    _rescaleBscan(m_Bscan, width, height, tfmWidth, tfmHeight);

}




