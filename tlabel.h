#ifndef TLABEL_H
#define TLABEL_H

#include <QLabel>
#include <QObject>
#include <vector>

class TLabel : public QLabel
{
    Q_OBJECT
public:
    explicit TLabel(QWidget *parent=nullptr);
    void setData(const std::vector<qfloat16> &data);
private:
    QPixmap m_pixmap;
    std::vector<std::vector<qfloat16>> m_data{8192, std::vector<qfloat16>(100)};
    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // TLABEL_H
