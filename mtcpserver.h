#ifndef MTCPSERVER_H
#define MTCPSERVER_H

#include <QTcpServer>

class mTcpServer : public QTcpServer
{
public:
    explicit mTcpServer(QObject *parent = nullptr);

    // QTcpServer interface
protected:
    virtual void incomingConnection(qintptr handle) override;
};

#endif // MTCPSERVER_H
