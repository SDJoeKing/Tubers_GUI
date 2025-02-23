#ifndef TGATE_H
#define TGATE_H

#include <QGraphicsItemGroup>
#include <QGraphicsPathItem>

class TGate : public QGraphicsItemGroup
{
public:
    explicit TGate(QWidget * canvas, QGraphicsItem *parent = nullptr);
    QRectF posRange();
    bool groupSelected(QPointF point);
    QPointF ascanValue() const;
    void updateAscanValue();
    // QGraphicsItem interface
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
private:
    bool itemSelected(QGraphicsItem *item, QPointF point);
    bool m_moving=false;
    const float M_XBOUND=25;
    QWidget *_canvas;
    QPointF m_ascanValue;

    // QGraphicsItem interface
public:
    // virtual QRectF boundingRect() const override;

    // QGraphicsItem interface
public:
    virtual QRectF boundingRect() const override;


};

#endif // TGATE_H
