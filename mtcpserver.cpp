#include "mtcpserver.h"
#include <qtcpsocket.h>

mTcpServer::mTcpServer(QObject *parent)
    : QTcpServer{parent}
{}

void mTcpServer::incomingConnection(qintptr handle)
{
    qDebug() << "incoming connection";

    QTcpSocket *newSocket = new QTcpSocket;
    newSocket->setSocketDescriptor(handle);
    addPendingConnection(newSocket);

}
