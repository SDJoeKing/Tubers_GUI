#include "tlabel.h"

TLabel::TLabel(QWidget *parent):QLabel{parent}
{
    qDebug() << m_data.size();
}

void TLabel::setData(const std::vector<qfloat16> &data)
{

}


void TLabel::resizeEvent(QResizeEvent *event)
{
    m_pixmap = m_pixmap.scaled(size(), Qt::IgnoreAspectRatio);
    setPixmap(m_pixmap);
}
