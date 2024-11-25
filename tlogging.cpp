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

    m_dialog = new QFileDialog(this, "Select Save File", QApplication::applicationDirPath(), "DAT File(*.dat)");
    m_dialog->setAcceptMode(QFileDialog::AcceptSave);
    m_dialog->setModal(false);
    m_dialog->setFileMode(QFileDialog::AnyFile);
    m_dialog->setViewMode(QFileDialog::ViewMode::Detail);
    m_dialog->setOptions(QFileDialog::DontConfirmOverwrite | QFileDialog::DontUseNativeDialog);
    connect(m_dialog, &QFileDialog::accepted, this, &Tlogging::updateFileName);
}

Tlogging::~Tlogging()
{
    m_file.close();
    delete ui;
}

void Tlogging::setData(const char* dataptr)
{
    m_data = QByteArray::fromRawData(dataptr, mTcpClient::DATA_SIZE/2);
    emit dataReceived();

}

void Tlogging::on_toolButton_clicked()
{
    m_dialog->show();
}

void Tlogging::updateFileName()
{

    QString fileName = m_dialog->selectedFiles().at(0);

    if(fileName.isEmpty())
        return;

    QFileInfo info(fileName);
    if(info.suffix()!="dat")
        fileName+=".dat";

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

