#ifndef MTCPCLIENT_H
#define MTCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QtNetwork>
#include <QMetaEnum>
#include <QEventLoop>
#include <QMessageBox>

class mTcpClient : public QObject
{
    Q_OBJECT

public:
    explicit mTcpClient(QObject *parent=nullptr);
    ~mTcpClient();
    static const quint16 DATA_SIZE = 16384;
    void start(const QString &address, const quint8 port);
    void stop();
    void writeData(QByteArray arr);
    bool isOpen();
    QByteArray data() const;

public slots:
    void requestData(); // to work with timer of refresh rate
    void startAcquisition();
    void stopAcquisition();
    void sendSetting(const QString &);
    void clearData();
    void flush();

private:

    QTcpSocket *m_socket;
    QString m_name="TcpClient:  ";
    QString m_server = "TcpServer:  ";
    QString m_command;
    QString errorToType(int);
    QTcpSocket::SocketState m_state;
    QByteArray m_data;
    bool m_lock;
    bool getLock(){return m_lock;};
    quint16 counter_data=0;
    quint16 m_readSize = 0;

private slots:
    void readMessage(); //  signal readyRead, slot readMessage
    void errorOccurred(QAbstractSocket::SocketError socketError);
    void connected();
    void disconnected();
    void notifyServerDown();
    void updateState(QTcpSocket::SocketState);
    // void lockRelease();

signals:
    void clientMessage(const QString &msg);
    void tcpMessage(const QString &msg);
    void tcpMessage(QByteArray arr); // overload for ascan data
    void serverReady(bool);
    void settingReady(bool);
    void acquisitionReady();
    void acquisitionStop();
    void dataReady(bool);
    // void canStop();

};

#endif // MTCPCLIENT_H
