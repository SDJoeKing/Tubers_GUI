#ifndef TSETTINGS_H
#define TSETTINGS_H

#include <QDialog>
#include <QSizePolicy>
#include <QFocusEvent>
#include <mtcpclient.h>

namespace Ui {
class TSettings;
}

class TSettings : public QDialog
{
    Q_OBJECT



public:
    explicit TSettings(QWidget *parent = nullptr);
    ~TSettings();

public slots:

    void updateVel(double);
    int getAscanIndex();
    void encoderTriggerMode(bool);
    void sendSetting();

private slots:
    void on_btnConfirm_clicked();

    void on_btnHide_clicked();

    void on_comboLength_currentIndexChanged(int index);

    void errorInSettings(const QString &);

    void on_radioManual_toggled(bool checked);

    void on_radioPresetGolay_toggled(bool checked);

    void on_radioManualGolay_toggled(bool checked);

    void manualSeqVisible(bool);

    void presetGolayVisible(bool);

    void manualGolayVisible(bool);

    void on_comboPresetGolayA_currentIndexChanged(int index);

    void on_pulseSequence_textChanged(const QString &arg1);

    void on_pulseSequence_cursorPositionChanged(int arg1, int arg2);

    void do_settingChanged(bool);

    void on_groupBscan_clicked();

signals:
    void settingConfirm(const QString &settings);
    void bScanSetting(bool, const QList<double> &settings);
    void settingHide();
    void fsChanged(const float);
    void badSettings();
    void prf(const int);
private:
    Ui::TSettings *ui;
    void installFilter(QObject *);
    bool encoderMode = false;
    QString m_pulse;
    quint8 m_pulseLength = 32;

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    void disableScroll(bool);
    quint8 pulseLength() const;
};

#endif // TSETTINGS_H
