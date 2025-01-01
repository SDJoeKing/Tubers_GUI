#ifndef TSETTINGS_H
#define TSETTINGS_H

#include <QDialog>
#include <QSizePolicy>
#include <QFocusEvent>

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
    enum settingParams
    {
        txChannel = 0,
        rxChannel,
        pulseDelayNs,
        pulseSequence,
        pulseFreq,
        prf,
        pulsingPower,
        gain,
        requestedAverages,
        encoderTriggering,
        encoderSkips,
        motorSpeed,
        motorAngle,
        velocity,
        refreshRate,
        order,
        lowCut,
        highCut
    };

public slots:
    void updateHz(QString);
    void updateVel(double);
    int getAscanIndex();
private slots:
    void on_btnConfirm_clicked();

    void on_btnHide_clicked();

    void on_comboLength_currentIndexChanged(int index);

signals:
    void settingConfirm(const QString &settings);
    void bScanSetting(bool, const QList<double> &settings);
    void settingHide();
    void fsChanged(const float);
private:
    Ui::TSettings *ui;
    void installFilter(QObject *);

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // TSETTINGS_H
