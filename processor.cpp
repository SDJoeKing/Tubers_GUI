#include "processor.h"
#include "mainwindow.h"

QElapsedTimer processTimer;

Processor::Processor(QObject *parent)
    : QObject{parent}
{
    m_vel = 5890.0;
    depthAxis = false;
    rectify = false;
    filtering = false;

    processTimer.start();
    qDebug() << "Processor on";
}

Processor::~Processor()
{
    qDebug() << "Processor shutdown";
}

void Processor::setParam(const float &vel, const bool &depth, const bool &rect, const bool &filt)
{
    m_vel = vel;
    depthAxis = depth;
    rectify = rect;
    filtering = filt;
}

void Processor::updateFilter(const quint8 &order, const float &fs, const float &fc, const float &fw)
{

    Dsp::Params params;

    params[0] = fs;
    params[1] = order;
    params[2] = fc;
    params[3] = fw;

    m_filter->setParams(params);
    // m_filter->setup(order, fs, fc, fw);
    m_fs = fs;
}

void Processor::run()
{
    m_loop = new QEventLoop(this);

    m_loop->exec();
    deleteLater();
}

void Processor::close()
{
    m_loop->quit();
}

void Processor::process(const char *dataptr)
{

    auto serverData = QByteArray::fromRawData(dataptr, mTcpClient::DATA_SIZE_RECV );

    double xpoint=0;
    QList<QPointF> calPoint(mTcpClient::DATA_SIZE/2);
    quint16 temp1;
    quint16 temp2;
    float *dataPoint[1];
    float _temp[mTcpClient::DATA_SIZE/2]{0};
    dataPoint[0] = _temp;

    // QByteArray _arr(mTcpClient::DATA_SIZE/2, Qt::Uninitialized);

    // take into account of header data
    int j= mTcpClient::HEADER_SIZE;
    // get system information from the header data
    quint8 _forward = static_cast<quint8>(serverData.at(4));

    temp1 =(serverData.at(7) << 8) & 0xFF00;

    temp2 = (serverData.at(6)) & 0xFF;

    float _temperature =  (static_cast<quint16>(temp2 | temp1));
    // qDebug() << "------- " << _temperature / 2 / 125e6 *2293.0 << " ------- ";
    _temperature = ((_temperature/65536.0f)/0.00198421639f ) - 273.15f;

    for (int i = 0; i < mTcpClient::DATA_SIZE/2; i++)
    {
        temp1 =(serverData.at(j + 1) << 8) & 0xFF00;
        temp2 = (serverData.at(j)) & 0xFF;
        dataPoint[0][i] = static_cast<qint16>(temp2 | temp1)/ 32768.0  * 3.18 * 1.0 *1000.0;

        j += 2;
    }

    if(filtering)
    {
        m_filter->process(mTcpClient::DATA_SIZE/2, dataPoint);
    }

    // rectified, envelope, depth?
    for (int i = 0; i < mTcpClient::DATA_SIZE/2; i++)
    {

        if(depthAxis)
            xpoint = i / m_fs /2/ 1e6 * m_vel * 1000;
        else
            xpoint = i / m_fs /1e6 *1000;

        if(rectify)
            dataPoint[0][i] = MainWindow::envelope(dataPoint[0][i], MainWindow::_env, MainWindow::m_ga, MainWindow::m_gr);

        calPoint[i] = QPointF(xpoint, dataPoint[0][i]);
        _temp[i] = dataPoint[0][i];
    }

    //reset envelope;
    MainWindow::_env = 0;

    emit dataLogger(reinterpret_cast<const char *>(&_temp));

#ifdef FRAMERATE_CONTROL
    if(processTimer.durationElapsed().count() > 1.0/FRAMERATE * 1e9 ) // 60HZ
    {
        emit dataProcessed(calPoint, _forward == 2 ? false : true);
        processTimer.restart();
    }
#else
    emit dataProcessed(calPoint, _forward == 2 ? false : true);
#endif
    emit sendTemperature(_temperature);
}

void Processor::setVel(const float &vel)
{
    m_vel = vel;
}

void Processor::setDepth(const bool &depth)
{
    depthAxis = depth;

}

void Processor::setRectified(const bool &rect)
{
    rectify = rect;
}

void Processor::setFiltering(const bool &filt)
{
    filtering = filt;
}
