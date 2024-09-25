#ifndef TLABEL_H
#define TLABEL_H

#include <QLabel>
#include <QObject>
#include <vector>
#include <QPixmap>

class TLabel : public QLabel
{
    Q_OBJECT
public:
    explicit TLabel(QWidget *parent=nullptr);
    ~TLabel();
    void setData(const QList<QPointF> &data, bool);
private:
    qint16 m_currentLine;
    QImage *m_image;
    QPixmap m_pixmap;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // TLABEL_H
