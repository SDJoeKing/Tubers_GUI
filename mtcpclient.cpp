#include "mtcpclient.h"
#include <thread>

static int counter = 0;
static int old_counter = 0;
static int dataEmitCounter = 0;
static int dataEmitTracker = 0;


QByteArray ack = QString("data acknowledged").toUtf8();
QByteArray stopAcq = QString("stop").toUtf8();

mTcpClient::mTcpClient(QObject *parent)
    :QObject{parent}
{
    shutdownLock = true;
    acquisitionRunning = false;

}

mTcpClient::~mTcpClient()
{
    qDebug() << "TCP client destroyed";
    qDebug() << "client is open ? " << isOpen();
    qDebug() << counter << " acquisitions received";
    delete m_socket;

}

bool mTcpClient::start(const QString &address, const quint16 port)
{
    if(isOpen())
        m_socket->disconnectFromHost();

    m_socket->connectToHost(address, port);
    return(m_socket->waitForConnected(3000));

}

void mTcpClient::stop()
{
    if(isOpen())
    {

        disconnected(); // send disconnect info first
        m_socket->disconnectFromHost();
        while(shutdownLock){};
    }
    this->m_socket->readAll();
    emit acquisitionStop();

    m_loop->quit();

}

void mTcpClient::writeData(QByteArray arr)
{
    m_socket->write(arr);
}

void mTcpClient::updateState(QTcpSocket::SocketState state)
{
    m_state = state;
}


void mTcpClient::clearData()
{
    m_data.clear();
    m_readSize = 0;
}

void mTcpClient::flush()
{
    QByteArray temp="000";
    m_socket->flush();
    if(temp.size()!=0)
        temp=m_socket->readAll();
    m_readSize = 0;
}


void mTcpClient::setStopAcq()
{
    m_stopAcq = 1;
}

void mTcpClient::timerOn(bool on)
{
    if(on)
    {
        m_timer->start();
#ifdef FRAMERATE_CONTROL
        m_frameControlTimer->start();
#endif
    }
    else
    {
        m_timer->stop();
#ifdef FRAMERATE_CONTROL
        m_frameControlTimer->stop();
#endif
    }
}

void mTcpClient::setPrf(const int newPrf)
{
    prf = newPrf;
}

bool mTcpClient::isOpen()
{

    return m_state == QTcpSocket::ConnectedState;

}


void mTcpClient::startAcquisition()
{
    m_command = "start";
    writeData(m_command.toUtf8());

}

void mTcpClient::stopAcquisition()
{
    m_command = "stop";
    writeData(m_command.toUtf8());
    qDebug() << "Stopcommand " << m_command;
}

void mTcpClient::sendSetting(const QString & param)
{
    m_command = "settings;"; // ";" is for parameters separation
    writeData((m_command + param).toUtf8());
}

QString mTcpClient::errorToType(int i)
{
    QMetaEnum key = QMetaEnum::fromType<QAbstractSocket::SocketError>();
    return key.valueToKey(i);
}

const QByteArray & mTcpClient::data()
{
    return m_readyData;
}

void mTcpClient::setHostPort(const QString& addr, const quint16& port)
{
    m_address = addr;
    m_port = port;

}

static bool headerFound(const QByteArray &arr)
{

    if(arr.size() > 3)
        return ( (arr.at(0) == 0) && (static_cast<quint8>(arr.at(1)) == 0xFF) && (arr.at(2) == 0) && (static_cast<quint8>(arr.at(3)) == 0xFF) );
    else
    {
        return false;
    }
}


bool mTcpClient::parseServerMsg(QByteArray &arr)
{
    auto size = arr.size();

    if(size<1)
    {
        QMessageBox::warning(nullptr, "Warning!", "Get error reading from server");
        this->stop();
        notifyServerDown();
        acquisitionRunning = false;
        return 1;
    }

    QString msg = QString::fromUtf8(arr, 20);

    if(msg.contains("ematserver"))
    {
        emit serverReady(true);
        emit tcpMessage(m_server + msg.sliced(0, 10));
        arr.slice(10);
        return -1;
    }
    // Settings
    else if(msg.contains("settings ok"))
    {
        if(!acquisitionRunning)
            QTimer::singleShot(500, this, [this](){emit settingReady(true);}); //Note that 500 ms delay is for the hardware to apply the setting;

        emit tcpMessage(m_server + msg.sliced(0, 11));
        arr.slice(11);
        return -1;
    }
    else if(msg.contains("bad settings"))
    {
        emit badSettings();
        return 1;
    }
    // acquisition
    else if(msg.contains("starting acquisition"))
    {
        acquisitionRunning = true;
        emit acquisitionReady();
        emit tcpMessage(m_server + msg.sliced(0, 20));
        arr.slice(20);
        return -1;
    }
    // stop acquisition
    else if(msg.contains("acquisition stopped"))
    {
        acquisitionRunning = false;
        emit acquisitionStop();
        emit tcpMessage(m_server + msg.sliced(0, 19));
        arr.slice(19);
        m_stopAcq = false;
        return -1;
    }
    else if(msg.contains("data acknowledged"))
    {
        arr.slice(17);
        return -1;
    }
    else
    {
        return 1;
    }
}

void mTcpClient::do_timeout()
{
    if(m_timer->isActive())
    {
        emit fps(static_cast<float>(counter-old_counter) / m_timer->interval() * 1000.0);
        old_counter = counter;

        emit plotRate(static_cast<float>(dataEmitCounter-dataEmitTracker) / m_timer->interval() * 1000.0);
        dataEmitTracker = dataEmitCounter;

    }
}

void mTcpClient::do_frameRateControl()
{
#ifdef FRAMERATE_CONTROL
    if(m_frameControlTimer->isActive())
    {
        if(prf > FRAMERATE)
        {
            emit dataReady(m_readyData.constData());
            dataEmitCounter++;
        }
    }
#endif
}

void mTcpClient::run()
{
    m_loop = new QEventLoop(this);
    m_timer = new QTimer(this);

#ifdef FRAMERATE_CONTROL
    m_frameControlTimer = new QTimer(this);
    m_frameControlTimer->setInterval(1000.0/FRAMERATE);
    connect(m_frameControlTimer, &QTimer::timeout, this, &mTcpClient::do_frameRateControl);
#endif

    m_timer->setInterval(1000);
    m_timer->start();

    m_socket = new QTcpSocket(this);
    m_socket->setReadBufferSize(32768);


    m_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    qDebug() << "socket thread: "<< this->thread();
    connect(m_socket, &QTcpSocket::connected, this, &mTcpClient::connected );
    connect(m_socket, &QTcpSocket::disconnected, this, &mTcpClient::notifyServerDown );
    connect(m_socket, &QTcpSocket::readyRead, this, &mTcpClient::readMessage );
    connect(m_socket, &QTcpSocket::errorOccurred, this, &mTcpClient::errorOccurred );
    connect(m_socket, &QTcpSocket::stateChanged, this, &mTcpClient::updateState);
    connect(m_timer, &QTimer::timeout, this, &mTcpClient::do_timeout);

    m_data = QByteArray(DATA_SIZE, Qt::Uninitialized);

    bool success = start(m_address, m_port);

    qDebug() << "Connect success? " << success;
    if(success)
    {
        flush();
        m_loop->exec();

    }else
    {
        emit connectFail();
        auto parent = static_cast<QThread *>(sender());
        parent->quit();
    }

    // delete m_loop;
    deleteLater();
}

void mTcpClient::readMessage()
{


    QByteArray tempData = m_socket->readAll();

    m_readSize = tempData.size();

    if(!parseServerMsg(tempData) || m_readSize == 17)
        return;

    if(headerFound(tempData) && !m_commence)
    {
        m_commence = 1;
        counter_data = 0;
        m_readSize = tempData.size();

    }

    if(m_commence)
    {
        {
            QMutexLocker lk(&mu);
            int size = (counter_data+m_readSize > DATA_SIZE_RECV ? DATA_SIZE_RECV : counter_data+m_readSize);
            m_data.replace(counter_data, size, tempData);
            counter_data+=m_readSize;
        }


        if(counter_data>=DATA_SIZE_RECV )
        {
            m_commence = 0;

            {
                QMutexLocker lk(&mu);
                m_readyData.assign(m_data);
            }

            if(m_stopAcq)
            {
                writeData(stopAcq);

            }else
            {
                // send data acknowledgement
                writeData(ack);
            }
#ifndef FRAMERATE_CONTROL
            emit dataReady(m_readyData.constData());
            dataEmitCounter++;
#else
            if(prf <= FRAMERATE)
            {
                emit dataReady(m_readyData.constData());
                dataEmitCounter++;

            }
#endif
            counter++;
            counter_data = 0;
            m_readSize = 0;
        }
    }

}

void mTcpClient::errorOccurred(QAbstractSocket::SocketError socketError)
{
    qDebug() << m_name + errorToType(socketError);
    emit tcpMessage(m_name + errorToType(socketError));
    emit errorOccured();
}

void mTcpClient::connected()
{
    m_command = "ematclient";
    writeData(m_command.toUtf8());
    emit clientMessage(m_name + m_command);
}

void mTcpClient::disconnected()
{
    m_command = "disconnect";
    writeData(m_command.toUtf8());
    emit clientMessage(m_name + m_command);

}

void mTcpClient::notifyServerDown()
{
    m_socket->flush();
    emit serverReady(false);
    shutdownLock = false;
}
