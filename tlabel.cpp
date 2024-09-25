#include "tlabel.h"

TLabel::TLabel(QWidget *parent):QLabel{parent}
{
    m_currentLine = -1;
    m_image = new QImage(5000, 1000, QImage::Format_RGB32);

}

TLabel::~TLabel()
{
    delete m_image;
}

void TLabel::setData(const QList<QPointF> &data, bool forward)
{

    if(forward)
    {
        m_currentLine+=1;
        // emplace back latest data
        for(int i=0; i<1000; i++)
        {
            m_image->setPixel(m_currentLine, i,qRgb(data[i].y(), 0, 0));
        }

    }else
    {
        m_currentLine <= 0 ? m_currentLine=0 : m_currentLine-=1;
        for(int i=0; i<1000; i++)
        {
            m_image->setPixel(m_currentLine, i, qRgb(data[i].y(), 0, 0));
        }
    }

    m_pixmap = QPixmap::fromImage(*m_image);
    setPixmap(m_pixmap);
    show();
}


void TLabel::resizeEvent(QResizeEvent *event)
{
    m_pixmap = m_pixmap.scaled(size(), Qt::IgnoreAspectRatio);
    setPixmap(m_pixmap);
}
