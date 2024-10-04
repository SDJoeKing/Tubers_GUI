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

void TSettings::updateVel(qfloat16 newVel)
{
    ui->spinVel->setValue(newVel);
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


    //encoder / bscan setting
    int encoderTrigger = ui->groupBscan->isChecked();
    float thick = ui->spinThick->value();
    float length = ui->spinScanLength->value();
    int step = ui->spinEncoderStep->value();
    float res = ui->spinEncoderRes->value();

    setting+= QString::number(encoderTrigger)+ ";";
    setting+= QString::number(step)+ ";";

    setting+= QString::number(ui->spinVel->value()) + ";";
    setting+= QString::number(ui->spinRefresh->value()) + ";";

    //filter setting
    setting+= QString::number(ui->spinOrder->value())+ ";";
    setting+= QString::number(ui->spinLowCut->value()) + ";";
    setting+= QString::number(ui->spinHighCut->value())+ ";";


    emit settingConfirm(setting);

    QList<qfloat16> bscanSetting;
    bscanSetting.emplaceBack(thick);
    bscanSetting.emplaceBack(length);
    bscanSetting.emplaceBack(step);
    bscanSetting.emplaceBack(res);
    emit bScanSetting(ui->groupBscan->isChecked(), bscanSetting);


}

