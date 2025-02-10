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
    ui->radioManual->click();
    ui->radioManual->toggled(true);
    ui->labelPresetGolayB->setVisible(false);
    ui->comboPresetGolayB->setVisible(false);
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
    QString _input;

    if(ui->radioManual->isChecked())
    {
        _input = ui->pulseSequence->text();
        _input += _input;
    }else if(ui->radioPresetGolay->isChecked())
    {
        quint8 _index = ui->comboPresetGolayA->currentIndex();
        QString seqA;
        switch(_index)
        {
            case 0:
                seqA = "PP";
                break;
            case 1:
                seqA = "PPNP";
                break;
            case 2:
                seqA = "PPPNPPNP";
                break;
            case 3:
                seqA = "NNPNNNNPNPPPNPNN";
                break;
            default:
                seqA = "PP";

        }
        auto seqB = ui->comboPresetGolayB->currentText().simplified().replace(" ", "");

        while(seqA.size() != m_pulseLength / 2)
        {
            seqA += 'C';
            seqB += 'C';
        }
        _input = seqA + seqB;
    }
    else
        _input = ui->lineGolayA->text() + ui->lineGolayB->text();


    if(_input.size() != m_pulseLength )
    {
        if(ui->radioManual->isChecked())
            ui->pulseSequence->setText(m_pulse);
        else
        {
            ui->lineGolayA->setText(m_pulse.sliced(0, m_pulseLength/2));
            ui->lineGolayB->setText(m_pulse.sliced(m_pulseLength/2));
        }

        errorInSettings("Invalid pulse sequence, please double check.");
        return;

    }else{
        for(size_t i=0; i<m_pulseLength; i++)
        {
            if(!QString("PpNnCc").contains(_input.at(i)))
            {
                if(ui->radioManual->isChecked())
                    ui->pulseSequence->setText(m_pulse);
                else
                {
                    ui->lineGolayA->setText(m_pulse.sliced(0, m_pulseLength/2));
                    ui->lineGolayB->setText(m_pulse.sliced(m_pulseLength/2));
                }
                errorInSettings("Invalid pulse sequence, please double check.");
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
    if(ui->spinHighCut->value() <= ui->spinLowCut->value())
    {
        errorInSettings("Filter high cutoff frequency must be larger than low cutoff frequency");
        return;
    }

    setting+= QString::number( (ui->radioManual->isChecked() ? 0 : 1) ) + ";";

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

quint8 TSettings::pulseLength() const
{
    return m_pulseLength;
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

void TSettings::errorInSettings(const QString &msg)
{
    QMessageBox::critical(this, "Error", msg);
    emit badSettings();
}





void TSettings::on_radioManual_toggled(bool checked)
{
    manualSeqVisible(checked);
    m_pulse = ui->pulseSequence->text();

}


void TSettings::on_radioPresetGolay_toggled(bool checked)
{
    presetGolayVisible(checked);

}


void TSettings::on_radioManualGolay_toggled(bool checked)
{
    manualGolayVisible(checked);
    m_pulse = ui->lineGolayA->text() + ui->lineGolayB->text();

}

void TSettings::manualSeqVisible(bool vis)
{
    ui->labelSeq->setVisible(vis);
    ui->pulseSequence->setVisible(vis);

    if(vis == true)
    {
        presetGolayVisible(!vis);
        manualGolayVisible(!vis);
    }


}

void TSettings::presetGolayVisible(bool vis)
{
    ui->labelPresetGolayA->setVisible(vis);
    ui->comboPresetGolayA->setVisible(vis);
    if(vis == true)
    {
        manualSeqVisible(!vis);
        manualGolayVisible(!vis);
    }

}

void TSettings::manualGolayVisible(bool vis)
{
    ui->labelGolayA->setVisible(vis);
    ui->labelGolayB->setVisible(vis);
    ui->lineGolayA->setVisible(vis);
    ui->lineGolayB->setVisible(vis);

    if(vis == true)
    {
        presetGolayVisible(!vis);
        manualSeqVisible(!vis);
    }


}



void TSettings::on_comboPresetGolayA_currentIndexChanged(int index)
{
    ui->comboPresetGolayB->setCurrentIndex(index);
}

