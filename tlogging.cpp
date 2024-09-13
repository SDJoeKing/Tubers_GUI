#include "tlogging.h"
#include "ui_tlogging.h"

Tlogging::Tlogging(mTcpClient *client, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Tlogging)
{
    ui->setupUi(this);
    ui->btnSingle->setEnabled(false);
    ui->btnContinuous->setEnabled(false);
    connect(this, &Tlogging::logFileSelected, this,  &Tlogging::doFileNameSet);
    m_file.setFileName("");
    m_client=nullptr;
    if(!setDataSource(client))
        QMessageBox::information(this, "Error", "Error initialising the TCP client for Logger");

}

Tlogging::~Tlogging()
{
    m_file.close();
    delete ui;
}

bool Tlogging::setDataSource(mTcpClient *client)
{
    try
    {
        m_client = client;
        return 1;
    }
    catch (...)
    {
        return 0;
    }

}

void Tlogging::on_toolButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Select Save Location", QApplication::applicationDirPath(), "Text File(*.txt)");
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
    doWriteData();
}

void Tlogging::doWriteData()
{
    // open device
    if(!m_file.open(QIODevice::Append))
    {
        QMessageBox::information(this, "Error", "Cannot open specified path");
        return;
    }

    m_file.write(m_client->data());
    m_file.close();
}

void Tlogging::on_btnContinuous_clicked(bool checked)
{
    if(checked)
    {
        connect(m_client, &mTcpClient::dataReady, this, &Tlogging::doWriteData);
        // open device
        if(!m_file.open(QIODevice::Append))
        {
            QMessageBox::information(this, "Error", "Cannot open specified path");
            return;
        }
    }

    else
    {
        disconnect(m_client, &mTcpClient::dataReady, this, &Tlogging::doWriteData);
        m_file.close();
    }
}

