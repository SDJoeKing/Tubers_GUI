#include "tsettings.h"
#include "ui_tsettings.h"

TSettings::TSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TSettings)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Popup| Qt::CustomizeWindowHint);
    setWindowTitle("S");
    setMouseTracking(false);

    installFilter(ui->scrollAreaWidgetContents_2);

}

TSettings::~TSettings()
{
    delete ui;
}

void TSettings::updateHz(QString arg)
{
    ui->labelHz->setText(arg);
}

void TSettings::updateVel(double newVel)
{
    ui->spinVel->setValue(newVel);
}

void TSettings::on_btnConfirm_clicked()
{

    QString setting;
    setting+= QString::number(ui->spinTx->value()) + ";"; // txchan
    setting+= QString::number(ui->spinRx->value()) + ";"; // rx
    setting+= QString::number(ui->spinPulseDelay->value()) + ";"; // pulse delay
    setting+= QString(ui->pulseSequence->text()) + ";"; // pulseSequence
    setting+= QString::number(ui->spinFrequency->value()) + ";"; // pulse freq
    setting+= QString::number(ui->prf->value()) + ";"; // prf

    int power = 0;
    switch(ui->powerOutput->currentIndex())
    {
        case 0:
            power = 25;
            break;
        case 1:
            power = 50;
            break;
        case 2:
            power = 100;
            break;
        default:
            power = 50;
    }

    setting+= QString::number(power) + ";"; // power output
    setting+= QString::number(ui->spinGain->value()) + ";"; // gain
    setting+= QString::number(ui->spinAvg->value())+ ";"; // avg


    //encoder / bscan setting
    int encoderTrigger = ui->groupBscan->isChecked();
    float thick = ui->spinThick->value();
    float length = ui->spinScanLength->value();
    int step = ui->spinEncoderStep->value();
    float res = ui->spinEncoderRes->value();

    // setting+= QString::number(encoderTrigger)+ ";"; // encoder triggering
    setting+=QString::number(0) + ";";
    setting+= QString::number(step)+ ";"; // encoder skips

    // motor speed
    setting+= QString::number(ui->motorSpeed->value()) + ";"; // motor speed
    setting+= QString::number(ui->motorAngle->value()) + ";"; // motor angle

    // velocity refreshrate
    setting+= QString::number(ui->spinVel->value()) + ";";
    setting+= QString::number(ui->spinRefresh->value()) + ";";

    //filter setting
    setting+= QString::number(ui->spinOrder->value())+ ";";
    setting+= QString::number(ui->spinLowCut->value()) + ";";
    setting+= QString::number(ui->spinHighCut->value())+ ";";


    emit settingConfirm(setting);

    QList<double> bscanSetting;
    bscanSetting.emplaceBack(thick);
    bscanSetting.emplaceBack(length);
    bscanSetting.emplaceBack(step);
    bscanSetting.emplaceBack(res);
    emit bScanSetting(ui->groupBscan->isChecked(), bscanSetting);


}

void TSettings::installFilter(QObject *obj)
{
    for(QObject* child : obj->children())
    {
        installFilter(child);

        if(qobject_cast<QAbstractSpinBox *>(child) || qobject_cast<QComboBox *>(child))
            child->installEventFilter(this);
    }
}



bool TSettings::eventFilter(QObject *watched, QEvent *event)
{


    if(event->type() == QEvent::Wheel)
    {
        if (!static_cast<QWidget *>(watched)->hasFocus())
        {
            event->ignore();
            return 1;
        }
    }


    return QDialog::eventFilter(watched, event);
}
