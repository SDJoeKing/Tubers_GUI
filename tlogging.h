#ifndef TLOGGING_H
#define TLOGGING_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include "mtcpclient.h"
namespace Ui {
class Tlogging;
}

class Tlogging : public QDialog
{
    Q_OBJECT

public:
    explicit Tlogging( mTcpClient *client, QWidget *parent = nullptr);
    ~Tlogging();
    bool setDataSource(mTcpClient *);

private slots:
    void on_toolButton_clicked();
    void on_btnSingle_clicked();
    void doFileNameSet(QString);
    void reset();
    void doWriteData();
    void on_btnContinuous_clicked(bool checked);

signals:
    void logFileSelected(QString);

private:
    Ui::Tlogging *ui;
    QFile m_file;
    mTcpClient *m_client;
};

#endif // TLOGGING_H
