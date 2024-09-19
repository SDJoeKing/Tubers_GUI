#include "mtcpclient.h"



mTcpClient::mTcpClient(QObject *parent)
    :QObject{parent}
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &mTcpClient::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &mTcpClient::notifyServerDown);
    connect(m_socket, &QTcpSocket::readyRead, this, &mTcpClient::readMessage);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &mTcpClient::errorOccurred);
    connect(m_socket, &QTcpSocket::stateChanged, this, &mTcpClient::updateState);
    // connect(this, &mTcpClient::canStop, this, &mTcpClient::lockRelease);
    m_data = QByteArray(16384, Qt::Uninitialized);
}

mTcpClient::~mTcpClient()
{
    qDebug() << "TCP client destroyed";
    qDebug() << "client is open ? " << isOpen();

    // if(isOpen())
    // {
    //     this->stop();
    // }


}

void mTcpClient::start(const QString &address, const quint8 port)
{
    if(isOpen())
        m_socket->disconnectFromHost();

    m_socket->connectToHost(address, port);
    m_socket->waitForConnected(3000);

}

void mTcpClient::stop()
{
    if(isOpen())
    {
        disconnected(); // send disconnect info first
        m_socket->disconnectFromHost();
        m_socket->waitForDisconnected();
    }
    this->m_socket->readAll();
    emit acquisitionStop();

}

void mTcpClient::writeData(QByteArray arr)
{
    m_socket->write(arr);
}

void mTcpClient::updateState(QTcpSocket::SocketState state)
{
    m_state = state;
}

// void mTcpClient::lockRelease()
// {
//     m_lock = false;
// }

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

bool mTcpClient::isOpen()
{
    // qDebug() << m_state;
    return m_state == QTcpSocket::ConnectedState;

}

void mTcpClient::requestData()
{

    m_command = "data request";
    writeData(m_command.toUtf8());
    // emit clientMessage(m_name + m_command); // too much info

}

void mTcpClient::startAcquisition()
{
    m_command = "start";
    writeData(m_command.toUtf8());

}

void mTcpClient::stopAcquisition()
{

    /*while(getLock()){};*/ // lock the process until no byte data is comming in

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

QByteArray mTcpClient::data() const
{
    return m_data;
}

void mTcpClient::readMessage()
{

    int remainSize = DATA_SIZE - counter_data;
    QByteArray tempData = m_socket->read(remainSize);
    m_readSize = tempData.size();

    if(m_readSize<1)
    {
        QMessageBox::warning(nullptr, "Warning!", "Get error reading from server");
        this->stop();
    }
    QString msg = QString::fromLatin1(tempData, 20);

    // for(int i=0; i<m_readSize; i++)
    //     qDebug() << tempData[i];

    // different cases for commands
    // connect

    if(msg.contains("ematserver"))
    {
        m_readSize = 0;
        emit serverReady(true);
        emit tcpMessage(m_server + msg.sliced(0, 10));
    }
        // Settings
    else if(msg.contains("settings ok"))
    {
        m_readSize = 0;
        emit settingReady(true);
        emit tcpMessage(m_server + msg.sliced(0, 11));}
    // acquisition
    else if(msg.contains("starting acquisition"))
    {
        m_readSize = 0;
        emit acquisitionReady();
        emit tcpMessage(m_server + msg.sliced(0, 20));
    }
    // stop acquisition
    else if(msg.contains("acquisition stopped"))
    {
        m_readSize = 0;
        emit acquisitionStop();
        emit tcpMessage(m_server + msg.sliced(0, 19));
    }
    else
    {
     // read all data routine


    m_data.replace(counter_data, counter_data+m_readSize, tempData, m_readSize);
    counter_data+=m_readSize;
    qDebug() <<"ReadSize: "<< m_readSize << " Counter: "<<counter_data;
    if(counter_data==DATA_SIZE)
    {
        emit dataReady(true);
        counter_data = 0;
        m_readSize = 0;
        qDebug() << "Datasize " << m_data.size();

        // emit canStop();    // open the lock for possible stop commands

    }

    }

}

void mTcpClient::errorOccurred(QAbstractSocket::SocketError socketError)
{
    qDebug() << m_name + errorToType(socketError);
    emit tcpMessage(m_name + errorToType(socketError));
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
}
