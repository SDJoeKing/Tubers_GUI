#include "testserver.h"
int sent = 0;
int tx = 0;
int rx = 0;
#include <QHostInfo>

static void dataGen(char * byteArr, uint8_t * arr, float scale)
{

    float *_df = reinterpret_cast<float *>(byteArr);
    quint16 j =  HEADER_SIZE;
    for(size_t i=0; i< DATA_SIZE/2;i++)
    {
        // bool _t = (i < 1000 && QRandomGenerator::global()->bounded(0, 10000) > 9998) ? 1 : 0;
        bool _t = 0;

        qint16 _temp = static_cast<qint16>( _t ?  32768*1.8 : _df[i]*scale / 1000 / 3.18 * 32768 );
        // qint16 _temp = static_cast<qint16>( _t ?  32768*1.8 : _df[i]);
        arr[j] = (_temp) & 0x00FF;
        arr[j+1] = (_temp >>8) &0x00FF;
        j+=2;

    }
}


testServer::testServer(QWidget *parent) : QObject(parent)
{
    m_socket=nullptr;
    m_loop = nullptr;


     _data[0] = 0x0F;
     _data[1] = 0x0F;
     _data[2] = 0x0F;
     _data[3] = 0x0F;
     _data[6] = 5;


     m_file.setFileName("../../test_data");

     if(m_file.open(QIODevice::ReadOnly))
     {
         m_arr =  m_file.readAll();
         char * _d = m_arr.data();
         dataGen(_d, _data, m_scale);
     }else
     {
         qDebug()<< "Open file not successful";
     }

     // log_file.setFileName("log.dat");
     // log_file.open(QIODevice::WriteOnly | QIODevice::Append);

}

testServer::~testServer()
{

    if(m_socket !=nullptr && m_socket->state() == QAbstractSocket::ConnectedState)
        m_socket->disconnectFromHost();

    if(m_server->isListening())
        m_server->close();

    qDebug() << "Test server closed, " << sent << " acquisitions sent";

    if(m_file.isOpen())
        m_file.close();
    log_file.close();
}

void testServer::run()
{
    m_loop = new QEventLoop(this);
    m_server = new QTcpServer(this);
    m_timer = new QTimer(this);

    QString hostname(QHostInfo::localHostName());
    qDebug() << hostname;
    QHostInfo info = QHostInfo::fromName(hostname);
    qDebug() << info.addresses();

    // m_server->listen(QHostAddress("127.0.1.1"), 5);
    m_server->listen(QHostAddress::AnyIPv4, 5000);

    // signal connection
    connect(m_timer, &QTimer::timeout, this, &testServer::do_sendData);
    connect(m_server, &QTcpServer::newConnection, this, &testServer::do_connection);

    qDebug() << "Test server started listening";
    // exect event loop#
    m_loop->exec();

    deleteLater();
}

void testServer::do_connection()
{
    qDebug() << "Connect signal";
    m_socket = m_server->nextPendingConnection();

    // connect socket signals
    connect(m_socket, &QTcpSocket::connected, this, &testServer::do_connected, Qt::QueuedConnection);
    connect(m_socket, &QTcpSocket::readyRead, this, &testServer::do_readyRead, Qt::QueuedConnection);
    connect(m_socket, &QTcpSocket::disconnected, this, &testServer::do_disconnection, Qt::QueuedConnection);
}

void testServer::do_connected()
{
    qDebug() << "Test server connected";
}

void testServer::do_disconnection()
{
    m_socket->write(QString("disconnected").toLatin1());
    m_socket->deleteLater();

}

void testServer::do_readyRead()
{

    QString msg = m_socket->readAll();

    // logics to different commands:
    if(msg.contains("ematclient"))
    {
        m_socket->write("ematserver");

    }else if(msg.contains("settings"))
    {
        m_socket->write("settings ok");
        QList list = msg.split(u';');
        m_timer->setInterval(1000.0/list[6].toInt());
        m_scale = powf(10, (list[8].toInt() - 20) / 20.0);

    }else if(msg.contains("start"))
    {
        m_socket->write("starting acquisition");
        m_timer->start();
        data_release = true;

    }else if(msg.contains("stop"))
    {
        m_timer->stop();
        m_socket->write("acquisition stopped");
        data_release = false;

    }else if(msg.contains("data acknowledged"))
    {
        // m_socket->write("data acknowledged");
        data_release = true;
    }

}

void testServer::do_sendData()
{

    if(data_release)
    {
        data_release = false;

        dataGen(m_arr.data(), _data, m_scale);
        // datasending logic
        m_socket->write(reinterpret_cast<const char *>(_data), DATA_SIZE_RECV);
        sent++;
    }
}

void testServer::stop()
{
    if(m_loop!=nullptr)
        m_loop->quit();
}
