#ifndef TLOGGING_H
#define TLOGGING_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>

namespace Ui {
class Tlogging;
}

class Tlogging : public QDialog
{
    Q_OBJECT

public:
    explicit Tlogging(QWidget *parent = nullptr);
    ~Tlogging();

public slots:
    void setData(const QByteArray &);

private slots:
    void on_toolButton_clicked();
    void on_btnSingle_clicked();
    void doFileNameSet(QString);
    void reset();
    void doWriteData();
    void on_btnContinuous_clicked(bool checked);
    void updateFileName();
signals:
    void logFileSelected(QString);
    void dataReceived();
private:
    Ui::Tlogging *ui;
    QFile m_file;
    QByteArray m_data;
    QFileDialog *m_dialog;
};

#endif // TLOGGING_H
