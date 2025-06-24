#ifndef MTCPCLIENT_H
#define MTCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QtNetwork>
#include <QMetaEnum>
#include <QEventLoop>
#include <QMessageBox>
#include <QMutex>
#include <QMutexLocker>
#include "config.h"
#include <QReadWriteLock>

class mTcpClient : public QObject
{
    Q_OBJECT

public:
    explicit mTcpClient(QObject *parent=nullptr);
    ~mTcpClient();

    bool start(const QString &address, const quint16 port);
    void stop();
    void writeData(QByteArray arr);
    bool isOpen();
    const QByteArray& data();
    void setHostPort(const QString&, const quint16&);

public slots:
    void startAcquisition();
    void stopAcquisition();
    void sendSetting(const QString &);
    void clearData();
    void flush();
    void setChannel(int);
    void setStopAcq();
    void timerOn(bool);
    void setPrf(const int);
    void requestStatus();
private:
    QMutex mu;
    QTcpSocket *m_socket;
    QString m_name="TcpClient:  ";
    QString m_server = "TcpServer:  ";
    QString m_command;
    QString errorToType(int);
    QTcpSocket::SocketState m_state;
    QByteArray m_data;
    QByteArray m_readyData;
    quint16 counter_data=0;
    quint16 m_readSize = 0;
    bool m_commence = 0;
    int m_channel = 16;
    bool m_stopAcq = 0;
    QEventLoop *m_loop;
    QString m_address;
    quint16 m_port;
    bool shutdownLock;
    bool acquisitionRunning;
    QTimer *m_timer;
    int prf;
#ifdef FRAMERATE_CONTROL
    QTimer *m_frameControlTimer;
#endif

    QReadWriteLock wrLock;
private slots:
    void readMessage(); //  signal readyRead, slot readMessage
    void errorOccurred(QAbstractSocket::SocketError socketError);
    void connected();
    void disconnected();
    void notifyServerDown();
    void updateState(QTcpSocket::SocketState);
    bool parseServerMsg(QByteArray &);
    void do_timeout();
    void do_frameRateControl();
signals:
    void clientMessage(const QString &msg);
    void tcpMessage(const QString &msg);
    void tcpMessage(const QByteArray &arr); // overload for ascan data
    void serverReady(bool);
    void settingReady(bool);
    void acquisitionReady();
    void acquisitionStop();
    void dataReady(const char*, bool headerOnly = false);
    void fps(float rate);
    void plotRate(float rate);
    void connectFail();
    void errorOccured();
    void badSettings();
    // QRunnable interface
public:
    void run();
};

#endif // MTCPCLIENT_H
