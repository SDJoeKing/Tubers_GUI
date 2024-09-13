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

private slots:
    void on_btnConfirm_clicked();

signals:
    void settingReady(QString settings);

private:
    Ui::TSettings *ui;
};

#endif // TSETTINGS_H
