#include "testserver.h"
#include <QHostInfo>
testServer::testServer(QWidget *parent) : QObject(parent)
{
    m_socket=nullptr;
    m_loop = nullptr;
}

testServer::~testServer()
{

    if(m_socket !=nullptr && m_socket->state() == QAbstractSocket::ConnectedState)
        m_socket->disconnectFromHost();

    if(m_server->isListening())
        m_server->close();

    qDebug() << "Test server closed";
}

void testServer::run()
{
    m_loop = new QEventLoop(this);
    m_server = new QTcpServer(this);

    QString hostname(QHostInfo::localHostName());
    qDebug() << hostname;
    QHostInfo info = QHostInfo::fromName(hostname);
    qDebug() << info.addresses();

    m_server->listen(QHostAddress("127.0.1.1"), 5);

    // signal connection
    connect(&m_timer, &QTimer::timeout, this, &testServer::do_sendData);
    connect(m_server, &QTcpServer::newConnection, this, &testServer::do_connection);

    qDebug() << "Test server started listening";
    // exect event loop#
    m_loop->exec();

    deleteLater();
}

void testServer::do_connection()
{
    m_socket = m_server->nextPendingConnection();

    // connect socket signals
    connect(m_socket, &QTcpSocket::connected, this, &testServer::do_connected);
    connect(m_socket, &QTcpSocket::readyRead, this, &testServer::do_readyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &testServer::do_disconnection);
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
        m_socket->write("Settings ok");

    }else if(msg.contains("start"))
    {
        m_socket->write("Settings ok");
        QList list = msg.split(u';');
        m_timer.setInterval(list[6].toInt());
        m_timer.start();

    }else if(msg.contains("stop"))
    {
        m_socket->write("acquisition stopped");

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
        // TBC
    }
}

void testServer::stop()
{
    if(m_loop!=nullptr)
        m_loop->quit();
}
