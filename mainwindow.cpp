#include "mainwindow.h"
#include "./ui_mainwindow.h"

qfloat16 MainWindow::_env=0;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
    // setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint) ;
    ui->log->setEnabled(false);
    ui->actionTools->setEnabled(false);

    // status bar
    m_status= new QLabel(QString::asprintf("Ultrasound Velocity: %.2f m/s", m_vel), this);
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
    m_Ascan = new TChartViewForm(_splitter);
    m_Bscan = new QCustomPlot(_splitter);

    // configure B-scan
    // m_Bscan->addGraph()
    QCPColorMap *_colorMap = new QCPColorMap(m_Bscan->xAxis, m_Bscan->yAxis);
    m_Bscan->xAxis->setLabel("Scan Length [mm]");
    m_Bscan->yAxis->setLabel("Depth [mm]");
    int nx = 1000;
    int ny = 1000;
    _colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    _colorMap->data()->setRange(QCPRange(0, 10), QCPRange(0, 10)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions
    m_Bscan->setContextMenuPolicy(Qt::CustomContextMenu);

    // add a color scale:
    QCPColorScale *colorScale = new QCPColorScale(m_Bscan);
 // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    _colorMap->setColorScale(colorScale); // associate the color map with the color scale
    // set the color gradient of the color map to one of the presets:
    _colorMap->setGradient(QCPColorGradient::gpJet);
    _splitter->addWidget(m_Ascan);
    _splitter->addWidget(m_Bscan);
    _splitter->setSizes(QList<int>(height(), height()));

    // setting dock
    QDockWidget *settingDock = new QDockWidget(graphFrame,Qt::CustomizeWindowHint );
    settingDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    m_settings = new TSettings();
    settingDock->setWidget(m_settings);
    graphFrame->addDockWidget(Qt::LeftDockWidgetArea, settingDock);

    // TCP Client
    m_client = new mTcpClient(this);

    // logging dock
    QDockWidget *loggingDock = new QDockWidget(graphFrame,Qt::CustomizeWindowHint);
    loggingDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    m_logging = new Tlogging();
    loggingDock->setWidget(m_logging);
    graphFrame->addDockWidget(Qt::RightDockWidgetArea, loggingDock);
    loggingDock->setVisible(false);

    // QTimer for data request
    m_timer = new QTimer(this);
    updateTimer();
    connect(m_timer, &QTimer::timeout, this, &MainWindow::doRequestData);

    // filter
    updateFilter();

    // connect
    connect(m_settings, &TSettings::settingConfirm, this, &MainWindow::doSettingsConfirmed);
    connect(m_client, &mTcpClient::settingReady, ui->frameTools, &QFrame::setEnabled);
    connect(m_client, &mTcpClient::clientMessage, this, &MainWindow::logMsg);
    connect(m_client, qOverload<const QString &>(&mTcpClient::tcpMessage), this, &MainWindow::logMsg);
    connect(m_client, &mTcpClient::serverReady, this, &MainWindow::toogleStatus);

    // acquisition related
    connect(m_client, &mTcpClient::acquisitionReady, this, &MainWindow::runAcquisition);
    connect(m_client, &mTcpClient::acquisitionStop, this, &MainWindow::stopAcquisition);
    connect(this, &MainWindow::dataReceived, m_client, &mTcpClient::clearData);
    connect(m_client, &mTcpClient::dataReady, this, &MainWindow::doDataReady);
    connect(this, &MainWindow::velocitySet, m_Ascan, &TChartViewForm::setVelocity);
    connect(this, &MainWindow::axisTypeChanged, m_Ascan, &TChartViewForm::changeAxisType);

    // key acquisitionRun or not
    connect(this, &MainWindow::acquisitionRun, m_Ascan, &TChartViewForm::acquisitionStatus);
    connect(this, &MainWindow::acquisitionRun, m_Ascan, &TChartViewForm::toogleSave);
    connect(ui->ckGates, &QCheckBox::checkStateChanged, m_Ascan, &TChartViewForm::startThickCal);

    connect(m_Ascan, &TChartViewForm::setRectifyUncheck, this, &MainWindow::setRectifyUnchecked);
    connect(this, &MainWindow::dataReceived, this, &MainWindow::updateBScan);
    connect(m_Ascan, &TChartViewForm::calculatedThickness, ui->spinDepth, &QDoubleSpinBox::setValue);
    connect(m_Bscan, &QCustomPlot::customContextMenuRequested, this, &MainWindow::bScanCustomContext);
    connect(m_settings, &TSettings::bScanSetting, this, &MainWindow::do_bScanSetting);

    // setting/logging related
    connect(this, &MainWindow::velocitySet, m_settings, &TSettings::updateVel);
    connect(this, &MainWindow::dataForLogger, m_logging, &Tlogging::setData);

    // final finish
    setConnectionIndicator();
    ui->spinEnvLevel->setValue(0);
    resetUI();

}

void MainWindow::resetUI()
{

    ui->log->setEnabled(false);

    // action status
    ui->actionConnection_Status->setChecked(true);
    ui->actionSettings->setChecked(true);
    ui->actionTools->setChecked(false);
    ui->frameTools->setVisible(false);

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
    if(!ui->frameTools->isVisible())
    {
        ui->actionTools->setEnabled(true);
        ui->actionTools->trigger();
    }
    /*
     cycle // pulseFreq // prf
     power // gain // avg
     trigger // skip // vel
     Hz // ord // l // h

    */

    // update internal timer logic
    auto list = str.split(";");
    qDebug() << list;
    auto avg = list[5].toInt();
    auto prf = list[2].toInt();
    auto _interval = list[9].toInt();
    m_vel = list[8].toFloat();
    m_order = list[10].toInt();
    m_fc = (list[12].toFloat() + list[11].toFloat() )/ 2.0;
    m_fw= list[12].toFloat() - list[11].toFloat();

    updateFilter();

    m_minTimerInterval = ((350*(avg-1) + 600) + (1.0/prf*1e6 + 200) *  avg + 350)/1000/0.9; // 10% safety margin
    m_minTimerInterval = qMax(17, m_minTimerInterval);
    m_settings->updateHz(QString("Refresh rate (max %1 Hz)").arg(1.0/m_minTimerInterval*1000, 0, 'f', 1));

    m_timerInterval = 1.0/_interval * 1000;
    updateTimer();

    int _size = 0;
    for(int i = 0 ; i< 8; i++)
        _size+=list[i].size();

    auto settings = str.sliced(0, _size+7);

    auto _status = ui->statusBar->findChild<QLabel *>("m_status");
    if(_status)
    {
        _status->setText(QString("Ultrasound Velocity: %1 m/s").arg(m_vel));
    }

    if(ui->btnRun->isChecked())
    {
        ui->btnRun->click();
        connect(m_client, &mTcpClient::settingReady, ui->btnRun, &QPushButton::click);
        QTimer::singleShot(120, this, [this, settings](){qDebug() << settings; m_client->sendSetting(settings); });
        QTimer::singleShot(130, this, [this](){ disconnect(m_client, &mTcpClient::settingReady, ui->btnRun, &QPushButton::click);});
    }
    else
    {

        m_client->sendSetting(settings);
        qDebug() << settings;
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
        bool success = m_client->start(address, port);
        if(!success)
        {
            ui->btnConnect->setChecked(false);
            QMessageBox::information(this, "Error", "Unable to make connection to server. Please check connection.");
            return;
        }

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
    ui->log->append(_prefix + "    " + str);
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
    float *dataPoint[1];
    float _temp[mTcpClient::DATA_SIZE/2]{0};
    dataPoint[0] = _temp;
    QByteArray _arr;
    bool _tempDepthFlag = false;
    if(ui->ckDepthAxis->isChecked())
        _tempDepthFlag = true;

    for (int i = 0; i < mTcpClient::DATA_SIZE/2; i++)
    {
        temp1 =(m_serverData[j + 1] << 8) & 0xFF00;
        temp2 = (m_serverData[j]) & 0xFF;
        dataPoint[0][i] = static_cast<qint16>(temp2 | temp1)/ 32768.0  * 3.18 * 1.0 *1000.0;

        j += 2;
    }

    if(ui->ckFilter->isChecked())
        m_filter.process(mTcpClient::DATA_SIZE/2, dataPoint);

    // rectified, envelope, depth?
    for (int i = 0; i < mTcpClient::DATA_SIZE/2; i++)
    {
        xpoint = i;
        if(_tempDepthFlag)
            xpoint = i / 2/ 125e6 * m_vel * 1000;

        if(ui->ckRectify->isChecked())
            dataPoint[0][i] = envelope(dataPoint[0][i]);

        calPoint[i] = QPointF(xpoint, dataPoint[0][i]);
        _arr.append(reinterpret_cast<const char *>(&dataPoint[0][i]), sizeof(dataPoint[0][i]));
    }

    resetEnv();
    m_Ascan->plot(calPoint);

    emit dataForLogger(_arr);
    emit dataReceived(calPoint, true); // sent for Bscan & clear tcp client data buffer
}

void MainWindow::on_btnRun_clicked(bool checked)
{
    if(checked)
    { // clear client data buffer
        m_client->startAcquisition();
        updateTimer();
        m_timer->start();
        emit acquisitionRun(true);
    }else
    {
        m_timer->stop(); //stop timer immediately to avoid further commands
        QTimer::singleShot(100, m_client, &mTcpClient::stopAcquisition);
        m_client->flush();
        emit acquisitionRun(false);
    }

}

void MainWindow::set_envelope(float attack, float release)
{
    m_ga = attack < 1e-20 ? 0 :1 - qExp(-1.0 / (attack * 125e6));
    m_gr = attack < 1e-20 ? 0 :1 - qExp(-1.0 / (release * 125e6));
}

qfloat16 MainWindow::envelope(qfloat16 sample)
{
    auto s = qAbs(sample);
    _env += (s - _env) * (s > _env ? m_ga : m_gr);
    return  _env;
}



void MainWindow::on_spinEnvLevel_valueChanged(int arg1)
{
    qfloat16 _release = arg1 * 1e-7;
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

void MainWindow::updateFilter()
{
    m_filter.setup(m_order, 125, m_fc, m_fw);
}

void MainWindow::on_ckRectify_clicked(bool checked)
{

    if(checked)
        emit axisTypeChanged(TChartViewForm::ABSOLUTEY);
    else
        emit axisTypeChanged(TChartViewForm::FULLY);
}

void MainWindow::updateBScan(const QList<QPointF> &data, bool forward)
{
    if(!use_bscan)
        return;

    auto _colorMap = static_cast<QCPColorMap *>(m_Bscan->plottable());
    int valueSize = _colorMap->data()->valueSize();
    if(forward)
    {
        m_currentLine >= _colorMap->data()->keySize() ? m_currentLine=0: m_currentLine++;

        // configure the colormap
        for(int i=0; i<valueSize; i++)
        {
            _colorMap->data()->setCell(m_currentLine, i, data[i].y());
        }
    }else
    {
        m_currentLine <= 0 ? m_currentLine=0 : m_currentLine-=1;
        for(int i=0; i<valueSize; i++)
        {
            _colorMap->data()->setCell(m_currentLine, i, data[i].y());
        }
    }
    _colorMap->rescaleDataRange();
    _colorMap->rescaleAxes();
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
    auto newDepth = QInputDialog::getDouble(this, "Please input true thickness", "Thickness (mm): ", 0, 0, 5000.0);
    auto oldDepth = ui->spinDepth->value();
    auto distance = oldDepth * 2 / 1000 * 125e6 / m_vel;
    m_vel = newDepth * 2/1000 * 125e6 / distance;

    auto _status = ui->statusBar->findChild<QLabel *>("m_status");

    if(_status)
    {
        _status->setText(QString("Ultrasound Velocity: %1 m/s").arg(m_vel));
    }

    emit velocitySet(m_vel);
}

void MainWindow::do_bScanSetting(bool arg, const QList<qfloat16> &settings)
{
    use_bscan = arg;
    ui->ckBscan->setEnabled(use_bscan);

    if(use_bscan)
    {
        QCPColorMap * _map = static_cast<QCPColorMap *>(m_Bscan->plottable());
        if(_map)
        {
            float thick = settings[0];
            float length = settings[1];
            int step = settings[2];

            // x/y axis array size
            int nx = length / ((step+1)* 0.01);
            int ny = thick * 2;

            _map->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
            _map->data()->setRange(QCPRange(0, length), QCPRange(0, 2 * thick));
            _map->rescaleDataRange();
            _map->rescaleAxes();
            m_Bscan->replot();
        }
    }
}

