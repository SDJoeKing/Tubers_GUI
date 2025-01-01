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



class mTcpClient : public QObject
{
    Q_OBJECT

public:
    explicit mTcpClient(QObject *parent=nullptr);
    ~mTcpClient();

    static const quint16 DATA_SIZE = 16384*2 ;
    static const quint8 HEADER_SIZE = 8;
    static const quint16 DATA_SIZE_RECV = DATA_SIZE + HEADER_SIZE;

    bool start(const QString &address, const quint8 port);
    void stop();
    void writeData(QByteArray arr);
    bool isOpen();
    const QByteArray& data();
    void setHostPort(const QString&, const quint8&);

public slots:
    void startAcquisition();
    void stopAcquisition();
    void sendSetting(const QString &);
    void clearData();
    void flush();
    void setStopAcq();
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
    bool m_stopAcq = 0;
    QEventLoop *m_loop;
    QString m_address;
    quint8 m_port;
private slots:
    void readMessage(); //  signal readyRead, slot readMessage
    void errorOccurred(QAbstractSocket::SocketError socketError);
    void connected();
    void disconnected();
    void notifyServerDown();
    void updateState(QTcpSocket::SocketState);
    bool parseServerMsg(QByteArray &);

signals:
    void clientMessage(const QString &msg);
    void tcpMessage(const QString &msg);
    void tcpMessage(const QByteArray &arr); // overload for ascan data
    void serverReady(bool);
    void settingReady(bool);
    void acquisitionReady();
    void acquisitionStop();
    void dataReady(const char*);
    void fps(float rate);
    void connectFail();

    // QRunnable interface
public:
    void run();
};

#endif // MTCPCLIENT_H
