#include "mainwindow.h"
#include "./ui_mainwindow.h"

double MainWindow::_env=0;
double MainWindow::m_ga = 0;
double MainWindow::m_gr = 0;

QElapsedTimer timer;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
    // setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
    setWindowFlags(  Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint) ;
    ui->log->setEnabled(false);
    ui->btnRun->setEnabled(false);
    ui->ckBscan->setEnabled(false);

    // status bar
    m_status= new QLabel(QString::asprintf("Ultrasound Velocity: %.2f m/s", m_vel), this);
    m_status->setObjectName("m_status");
    ui->statusBar->addPermanentWidget(m_status);
    QLabel *fpsBar = new QLabel(this);
    fpsBar->setObjectName("fpsBar");
    ui->statusBar->addWidget(fpsBar);

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
    m_Bscan->setOpenGl(true);

    // configure B-scan
    // m_Bscan->addGraph()
    QCPColorMap *_colorMap = new QCPColorMap(m_Bscan->xAxis, m_Bscan->yAxis);

    m_Bscan->setContextMenuPolicy(Qt::CustomContextMenu);
    // add a color scale:
    QCPColorScale *colorScale = new QCPColorScale(m_Bscan);
    // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    _colorMap->setColorScale(colorScale); // associate the color map with the color scale


    _splitter->addWidget(m_Ascan);
    _splitter->addWidget(m_Bscan);
    _splitter->setSizes(QList<int>(height()-30, height()));

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
    connect(m_processor, &Processor::dataProcessed, this, &MainWindow::updateBScan, Qt::QueuedConnection);
    connect(m_processor, &Processor::dataProcessed, m_Ascan, &TChartViewForm::plot, Qt::QueuedConnection);
    connect(m_processor, &Processor::sendTemperature, this, &MainWindow::updateTemp, Qt::QueuedConnection);

    connect(&processorThread, &QThread::finished, this, &MainWindow::threadFinished);
    processorThread.start();

    // connect settings
    connect(m_settings, &TSettings::settingConfirm, this, &MainWindow::doSettingsConfirmed);
    connect(m_settings, &TSettings::settingHide, this, &MainWindow::hideSetting);

    // acquisition related

    connect(this, &MainWindow::velocitySet, m_Ascan, &TChartViewForm::setVelocity);
    connect(this, &MainWindow::axisTypeChanged, m_Ascan, &TChartViewForm::changeAxisType);

    // key acquisitionRun or not
    connect(m_settings, &TSettings::fsChanged, this, &MainWindow::updateFs);
    connect(m_settings, &TSettings::fsChanged, m_Ascan, &TChartViewForm::updateFs);
    connect(this, &MainWindow::acquisitionRun, m_Ascan, &TChartViewForm::acquisitionStatus);
    connect(this, &MainWindow::acquisitionRun, m_Ascan, &TChartViewForm::toogleSave);
    connect(ui->ckGates, &QCheckBox::checkStateChanged, m_Ascan, &TChartViewForm::startThickCal);
    connect(ui->ckGates, &QCheckBox::checkStateChanged, ui->btnCal, &QPushButton::setEnabled);

    connect(m_Ascan, &TChartViewForm::setRectifyUncheck, this, &MainWindow::setRectifyUnchecked);
    connect(m_Ascan, &TChartViewForm::calculatedThickness, ui->spinDepth, &QDoubleSpinBox::setValue);
    connect(m_Bscan, &QCustomPlot::customContextMenuRequested, this, &MainWindow::bScanCustomContext);
    connect(m_settings, &TSettings::bScanSetting, this, &MainWindow::do_bScanSetting);

    connect(m_Ascan, &TChartViewForm::sendThreshold, this, &MainWindow::setThreshold);

    // setting/logging related
    connect(this, &MainWindow::velocitySet, m_settings, &TSettings::updateVel);


    // final finish
    setConnectionIndicator();
    ui->spinEnvLevel->setValue(0);
    ui->radioTemp->setText(QString::asprintf("Temperature: %.1f \u2103", 0.0));
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

    m_Bscan->setVisible(false);
    ui->log->clear();
    ui->btnRun->setChecked(false);
    ui->btnConnect->setChecked(false);
    ui->spinEnvLevel->setMinimum(0);
    ui->spinEnvLevel->setValue(0);

    m_Ascan->clear();

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

    ui->radioTemp->setStyleSheet(
        "QRadioButton::indicator {"
        "width:                  10px;"
        "height:                 10px;"
        "        border-radius:          7px;"
        "}"
        "QRadioButton::indicator:checked {"
        "background-color:       red;"
        "border:                 2px solid white;"
        "}"
        "QRadioButton::indicator:unchecked {"
        "background-color:       grey;"
        "border:                 2px solid white;"
        "}"
        );
}

MainWindow::~MainWindow()
{

    if(ui->btnConnect->isChecked())
    {
        ui->btnConnect->click(); //  manual disconnect

    }

    QTimer::singleShot(0, m_processor, &Processor::close);
    processorThread.quit();

    m_quitEvent.exec();

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
    qDebug() << list;

    m_vel = list[TSettings::velocity].toDouble();
    emit velocitySet(m_vel);

    m_order = list[TSettings::order].toInt();
    m_fc = (list[TSettings::lowCut].toDouble() + list[TSettings::highCut].toDouble() )/ 2.0;
    m_fw= qAbs(list[TSettings::highCut].toDouble() - list[TSettings::lowCut].toDouble());

    emit filterParam(m_order, m_fs, m_fc, m_fw);

    int _size = 0;
    for(int i = TSettings::txChannel ; i< TSettings::motorAngle + 1; i++)
        _size+=list[i].size()+1; // including the separator size

    auto settings = str.sliced(0, _size );
    settings += QString::number(m_settings->getAscanIndex()) + ";";
    qDebug() << settings;
    auto _status = ui->statusBar->findChild<QLabel *>("m_status");
    if(_status)
    {
        _status->setText(QString("Ultrasound Velocity: %1 m/s").arg(m_vel));
    }

    if(ui->btnRun->isChecked())
    {
        ui->btnRun->click();
        connect(m_client, &mTcpClient::settingReady, ui->btnRun, &QPushButton::click, Qt::QueuedConnection);
        QTimer::singleShot(120, this, [this, settings](){emit mainSendSetting(settings); });
        QTimer::singleShot(130, this, [this](){ disconnect(m_client, &mTcpClient::settingReady, ui->btnRun, &QPushButton::click);});
    }
    else
    {
        emit mainSendSetting(settings);
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

        // TCP Client
        m_client = new mTcpClient();

        connect(m_client, &mTcpClient::settingReady, ui->btnRun, &QPushButton::setEnabled, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::clientMessage, this, &MainWindow::logMsg, Qt::QueuedConnection);
        connect(m_client, qOverload<const QString &>(&mTcpClient::tcpMessage), this, &MainWindow::logMsg, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::serverReady, this, &MainWindow::toogleStatus, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::serverReady, ui->radioStatus, &QRadioButton::setChecked, Qt::QueuedConnection);
        connect(this, &MainWindow::stopAcqSig, m_client, &mTcpClient::setStopAcq, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::acquisitionReady, this, &MainWindow::runAcquisition, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::acquisitionStop, this, &MainWindow::stopAcquisition, Qt::QueuedConnection);
        connect(this, &MainWindow::dataReceived, m_client, &mTcpClient::clearData, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::dataReady, m_processor, &Processor::process, Qt::QueuedConnection);
        connect(m_client, &mTcpClient::connectFail, this, [this](){ui->btnConnect->setChecked(false);
                QMessageBox::information(this, "Error", "Unable to make connection to server. Please check connection.");}, Qt::QueuedConnection);
        connect(this, &MainWindow::mainSendSetting, m_client, &mTcpClient::sendSetting, Qt::QueuedConnection);
        // connect fps
        connect(m_client, &mTcpClient::fps, this, &MainWindow::do_fps, Qt::QueuedConnection);


        QString address = ui->ipAddress->text().simplified().replace(" ", "");
        quint8 port = ui->port->text().toInt();
        m_client->setHostPort(address, port);


        m_client->moveToThread(&socketThread);
        qDebug() << "m_client thread: "<< m_client->thread();
        connect(&socketThread, &QThread::started, m_client, &mTcpClient::run);
        socketThread.start();
    }
    else
    {
        // disconnect
        ui->radioStatus->setChecked(false);
        QTimer::singleShot(0, m_client, &mTcpClient::stopAcquisition);
        QTimer::singleShot(100, m_client, &mTcpClient::stop);

        resetUI();
        ui->btnRun->setDisabled(true);
    }
}

void MainWindow::logMsg(QString str)
{
    auto _dateTime = QDateTime::currentDateTime();
    QString _prefix = _dateTime.toString("[dd/MM/yyyy hh:mm:ss]\n");
    ui->log->append(_prefix + "    " + str);
}

void MainWindow::toogleStatus(bool arg)
{

    qDebug() << "radio status: " << arg;
    if(ui->btnConnect->isChecked() && arg)
    {
        ui->btnConnect->setText("Disconnect");
    }else
    {
        ui->btnConnect->setText("Connect");
        socketThread.quit();
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

double MainWindow::envelope(double sample, double &value, double ga, double gr)
{
    auto s = qAbs(sample);
    value += (s - value) * (s > value ?  ga :  gr);
    return value;
}

void MainWindow::on_btnRun_clicked(bool checked)
{
    if(checked)
    { // clear client data buffer
        QTimer::singleShot(0, m_client, [&](){m_client->startAcquisition();});
        emit acquisitionRun(true);
    }else
    {
        emit stopAcqSig();

        QTimer::singleShot(0, m_client, [&](){m_client->flush();});
        emit acquisitionRun(false);
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
    m_Ascan->toogleGates(checked);
}


void MainWindow::on_ckDepthAxis_clicked(bool checked)
{

    if(checked)
        emit axisTypeChanged(TChartViewForm::DEPTH);
    else
        emit axisTypeChanged(TChartViewForm::SAMPLE);
}


void MainWindow::resetEnv()
{
    _env=0;
}

void MainWindow::on_ckRectify_clicked(bool checked)
{

    if(checked)
    {
        emit axisTypeChanged(TChartViewForm::ABSOLUTEY);
        ui->spinEnvLevel->setValue(1);
    }
    else
        emit axisTypeChanged(TChartViewForm::FULLY);

}

int findFrontWall(const QList<QPointF> &data)
{
    int _max = 3000; // in water less than 2mm
    double value = 0;
    for(int i = _max; i< mTcpClient::DATA_SIZE/2; i++)
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


void MainWindow::updateBScan(const QList<QPointF> &data, bool forward)
{
    if(!use_bscan)
        return;

    auto _colorMap = static_cast<QCPColorMap *>(m_Bscan->plottable());
    int valueSize = _colorMap->data()->valueSize();
    // functions to find the first front wall peaks
    int _start = findFrontWall(data)-50;

    if((valueSize + _start) > data.size())
        valueSize = data.size() - _start;

    forward = true;
    if(forward)
    {
        m_currentLine >= _colorMap->data()->keySize() ? m_currentLine=0: m_currentLine++;

        // configure the colormap
        for(int i=_start; i<valueSize + _start; i++)
        {
            auto value = data[i].y() > m_thres ? data[i].y() : 0;
            auto plotValue = value > 0 ? value / data[_start+50].y() : 0;
            _colorMap->data()->setCell(m_currentLine, i - _start, plotValue);
        }
    }else
    {
        //  remove current front line

        m_currentLine <= 0 ? m_currentLine=0 : m_currentLine-=1;
        for(int i=_start; i<valueSize + _start; i++)
        {
            auto value = data[i].y() > m_thres ? data[i].y() : 0;
            auto plotValue = value > 0 ? value / data[_start+50].y() : 0;
            _colorMap->data()->setCell(m_currentLine+1, i - _start, 0);
            _colorMap->data()->setCell(m_currentLine, i - _start, plotValue);
        }
    }
    _colorMap->rescaleDataRange();
    _colorMap->rescaleAxes();
    _colorMap->setGradient(QCPColorGradient::gpJet);
    m_Bscan->replot(QCustomPlot::rpQueuedRefresh);
}


void MainWindow::on_actionReset_triggered(bool checked)
{
    resetUI();
}

void MainWindow::setRectifyUnchecked()
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
    auto newDepth = QInputDialog::getDouble(this, "Please input true thickness", "Thickness (mm): ", 0, 0, 5000.0, 2, &ok);

    if(ok)
    {
        auto oldDepth = ui->spinDepth->value();

        if(oldDepth == 0)
            return;

        auto distance = oldDepth * 2 / 1000 * 125e6 / m_vel;
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
        m_Bscan->setVisible(true);
        ui->ckBscan->setCheckState(Qt::Checked);
    }else
    {
        m_Bscan->setVisible(false);
        ui->ckBscan->setCheckState(Qt::Unchecked);
    }

    if(use_bscan)
    {


        m_Bscan->xAxis->setLabel("Scan Length [mm]");
        m_Bscan->yAxis->setLabel("Depth [mm]");

        QCPColorMap * _map = static_cast<QCPColorMap *>(m_Bscan->plottable());

        // clear graph for replot;
        _map->data()->fill(0);

        // reset front head to -1
        m_currentLine = -1;
        if(_map)
        {
            float thick = settings[0];
            float length = settings[1];
            int step = settings[2];
            float encoder_res = settings[3];
            // x/y axis array size
            int nx = length / ((step+1)* encoder_res)+1;
            int ny = 4 * thick / 1000.0 / m_vel * 125e6;

            _map->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
            _map->data()->setRange(QCPRange(0, length), QCPRange(0, 2 * thick));
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

void MainWindow::updateTemp(const float temp)
{
    ui->radioTemp->setText(QString::asprintf("Temperature: %.1f \u2103", temp));
    if(temp > 70.0)
        ui->radioTemp->setChecked(true);
    else
        ui->radioTemp->setChecked(false);
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
        fpsBar->setText(QString::asprintf("FPS: %.1f", fps));
    }
}

void MainWindow::threadFinished()
{
    m_quitEvent.quit();
}



