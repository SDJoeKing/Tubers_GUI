#include "tlogging.h"
#include "ui_tlogging.h"

Tlogging::Tlogging(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Tlogging)
{
    ui->setupUi(this);
}

Tlogging::~Tlogging()
{
    delete ui;
}
