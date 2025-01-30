#ifndef TESTSERVER_H
#define TESTSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QEventLoop>
#include <QTimer>

class testServer : public QObject
{
    Q_OBJECT
public:
    explicit testServer(QWidget *parent=nullptr);
    ~testServer();
// members
private:
    QTcpServer *m_server;
    QTcpSocket *m_socket;
    QEventLoop *m_loop;
    QByteArray m_data;
    bool data_release = false;
    QTimer m_timer;
// public slots:
public slots:
    void run();
    void do_connection();
    void do_connected();
    void do_disconnection();
    void do_readyRead();
    void do_sendData();
    void stop();
};

#endif // TESTSERVER_H
