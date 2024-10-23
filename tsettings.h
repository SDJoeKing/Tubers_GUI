#ifndef TSETTINGS_H
#define TSETTINGS_H

#include <QDialog>

namespace Ui {
class TSettings;
}

class TSettings : public QDialog
{
    Q_OBJECT

public:
    explicit TSettings(QWidget *parent = nullptr);
    ~TSettings();

    enum GOLAY
    {
        TWO_BIT, FOUR_BIT, EIGHT_BIT, TEN_BIT
    };

public slots:
    void updateHz(QString);
    void updateVel(double);
private slots:
    void on_btnConfirm_clicked();

signals:
    void settingConfirm(const QString &settings);
    void bScanSetting(bool, const QList<double> &settings);
private:
    Ui::TSettings *ui;

};

#endif // TSETTINGS_H
