#ifndef TLOGGING_H
#define TLOGGING_H

#include <QDialog>

namespace Ui {
class Tlogging;
}

class Tlogging : public QDialog
{
    Q_OBJECT

public:
    explicit Tlogging(QWidget *parent = nullptr);
    ~Tlogging();

private:
    Ui::Tlogging *ui;
};

#endif // TLOGGING_H
