#include "tlogging.h"
#include "ui_tlogging.h"

Tlogging::Tlogging(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Tlogging)
{
    ui->setupUi(this);
    ui->btnSingle->setEnabled(false);
    ui->btnContinuous->setEnabled(false);
    connect(this, &Tlogging::logFileSelected, this,  &Tlogging::doFileNameSet);
    m_file.setFileName("");
}

Tlogging::~Tlogging()
{
    m_file.close();
    delete ui;
}

void Tlogging::setData(const QByteArray &data)
{
    m_data = data;
    emit dataReceived();

}

void Tlogging::on_toolButton_clicked()
{

    QString fileName = QFileDialog::getSaveFileName(this, "Select Save Location", QApplication::applicationDirPath(), "Data File(*.dat)");
    if(fileName.isEmpty())
        return;

    emit logFileSelected(fileName);
    ui->lineFileName->setText(fileName);

}


void Tlogging::doFileNameSet(QString str)
{
    m_file.close();
    m_file.setFileName(str);
    //enable btns
    ui->btnSingle->setEnabled(true);
    ui->btnContinuous->setEnabled(true);


}

void Tlogging::reset()
{
    m_file.close();
    ui->lineFileName->clear();
    ui->btnSingle->setEnabled(false);
    ui->btnContinuous->setEnabled(false);
    m_file.setFileName("");
}

void Tlogging::on_btnSingle_clicked()
{
    // open device
    if(!m_file.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        QMessageBox::information(this, "Error", "Cannot open specified path");
        return;
    }

    doWriteData();
    m_file.close();
}

void Tlogging::doWriteData()
{
    if(m_file.isOpen())
    {
        m_file.write(m_data);

    }else
        QMessageBox::information(this, "Error", "File not open, cannot write data");
}

void Tlogging::on_btnContinuous_clicked(bool checked)
{
    if(checked)
    {
        connect(this, &Tlogging::dataReceived, this, &Tlogging::doWriteData);
        // open device
        if(!m_file.open(QIODevice::WriteOnly|QIODevice::Append))
        {
            QMessageBox::information(this, "Error", "Cannot open specified path");
            return;
        }
    }

    else
    {
        disconnect(this, &Tlogging::dataReceived, this, &Tlogging::doWriteData);
        m_file.close();
    }
}

