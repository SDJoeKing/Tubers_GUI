#include "tsettings.h"
#include "ui_tsettings.h"

TSettings::TSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TSettings)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Popup| Qt::CustomizeWindowHint);
    setWindowTitle("S");
}

TSettings::~TSettings()
{
    delete ui;
}

void TSettings::updateHz(QString arg)
{
    ui->labelHz->setText(arg);
}

void TSettings::on_btnConfirm_clicked()
{
    QString setting;
    setting+= QString::number(ui->spinCycles->value()) + ";";
    setting+= QString::number(ui->spinFrequency->value()) + ";";
    setting+= QString::number(ui->prf->value()) + ";";
    setting+= QString::number(ui->spinPower->value()) + ";";
    setting+= QString::number(ui->spinGain->value()) + ";";
    setting+= QString::number(ui->spinAvg->value())+ ";";
    setting+= QString::number(ui->spinRefresh->value()) +";";
    setting+= QString::number(ui->spinVel->value());
    emit settingConfirm(setting);
}

