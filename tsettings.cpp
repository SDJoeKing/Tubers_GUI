#include "tsettings.h"
#include "ui_tsettings.h"
#include "mtcpclient.h"

static QString encoderModeString = "Trigger Motor Run";
static QString contModeString = "Send Settings";

TSettings::TSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TSettings)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Popup| Qt::CustomizeWindowHint);
    setWindowTitle("S");
    setMouseTracking(false);

    installFilter(ui->scrollAreaWidgetContents_2);

    // initialise ascan length
    QStringList comboLength;

    ui->comboLength->clear();
    for(quint8 i=1;i<9; i++)
        comboLength<< QString("%1 ms").arg(mTcpClient::DATA_SIZE/2/125e3 * i, 3,'f', 3);

    ui->comboLength->addItems(comboLength);
    ui->btnConfirm->setVisible(false);

    m_pulse = ui->pulseSequence->text();
}

TSettings::~TSettings()
{
    delete ui;
}


void TSettings::updateVel(double newVel)
{
    ui->spinVel->setValue(newVel);
}

int TSettings::getAscanIndex()
{
    return ui->comboLength->currentIndex() + 1;
}

void TSettings::encoderTriggerMode(bool okay)
{
    if(okay) // use encoder motor triggering
    {

        ui->btnConfirm->setText(encoderModeString);

    }else
    {

        ui->btnConfirm->setText(contModeString);
    }

    encoderMode = okay;

}

void TSettings::on_btnConfirm_clicked()
{

    QString setting;
    setting+= QString::number(ui->spinTx->value()) + ";"; // txchan
    setting+= QString::number(ui->spinRx->value()) + ";"; // rx
    setting+= QString::number(ui->spinPulseDelay->value()) + ";"; // pulse delay
    QString _input = ui->pulseSequence->text();
    if(_input.size() != m_pulseLength )
    {
        QMessageBox::critical(this, "Error", "Invalid pulse sequence, please double check.");
        ui->pulseSequence->setText(m_pulse);
        emit badSettings();
        return;
    }else{
        for(size_t i=0; i<m_pulseLength; i++)
        {
            if(!QString("PpNnCc").contains(_input.at(i)))
            {
                QMessageBox::critical(this, "Error", "Invalid pulse sequence, please double check.");
                ui->pulseSequence->setText(m_pulse);
                emit badSettings();
                return;
            }
        }
    }
    m_pulse = _input;
    setting+= QString(_input) + ";"; // pulseSequence
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
            power = 75;
            break;
        case 3:
            power = 100;
            break;
        default:
            power = 100;

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

    setting+= QString::number(encoderTrigger)+ ";"; // encoder triggering
    // setting+=QString::number(0) + ";"; // encoder never triggers
    setting+= QString::number(step)+ ";"; // encoder skips

    // motor speed
    setting+= QString::number(ui->motorSpeed->value()) + ";"; // motor speed
    setting+= QString::number(ui->motorAngle->value()) + ";"; // motor angle

    // velocity refreshrate
    setting+= QString::number(ui->spinVel->value()) + ";";

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

void TSettings::sendSetting()
{
    ui->btnConfirm->click();
}

void TSettings::disableScroll(bool on)
{

    ui->frame->setDisabled(on);
    ui->groupBox->setDisabled(on);
        ui->groupBox_3->setDisabled(on);
            ui->groupBscan->setDisabled(on);

    ui->groupBox_2->setEnabled(true);
}

void TSettings::on_btnHide_clicked()
{

     emit settingHide();
}


void TSettings::on_comboLength_currentIndexChanged(int index)
{
    float fs = 125.0 / (ui->comboLength->currentIndex() + 1);
    emit fsChanged(fs);
    ui->label_fs->setText(QString::asprintf("%.2f MHz",fs));
}




