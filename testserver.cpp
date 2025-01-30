#include "testserver.h"
int sent = 0;

#include <QHostInfo>
testServer::testServer(QWidget *parent) : QObject(parent)
{
    m_socket=nullptr;
    m_loop = nullptr;


     _data[0] = 0;
     _data[1] = 0x00FF;
     _data[2] = 0;
     _data[3] = 0x00FF;



    // for(size_t i=8; i<32000+8; i++)
    //     m_data[i] = 0;

}

testServer::~testServer()
{

    if(m_socket !=nullptr && m_socket->state() == QAbstractSocket::ConnectedState)
        m_socket->disconnectFromHost();

    if(m_server->isListening())
        m_server->close();

    qDebug() << "Test server closed, " << sent << " acquisitions sent";

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
    m_server->listen(QHostAddress::Any, 1234);

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
        m_socket->write("data acknowledged");
        data_release = true;
    }

}

void testServer::do_sendData()
{

    if(data_release)
    {
        data_release = false;
        // datasending logic
        m_socket->write(reinterpret_cast<const char *>(_data), 32008);
        sent++;
    }
}

void testServer::stop()
{
    if(m_loop!=nullptr)
        m_loop->quit();
}
