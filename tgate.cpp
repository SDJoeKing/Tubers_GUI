#include "tgate.h"
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
TGate::TGate()
{
    QRectF f1(-25, -5, 2, 10);
    QRectF f2(25, -5, 2, 10);
    QRectF f3(-23, -1, 50, 2);
    QGraphicsRectItem *itemLeft = new QGraphicsRectItem(f1);
    QGraphicsRectItem *itemRight = new QGraphicsRectItem(f2);
    QGraphicsRectItem *itemMid = new QGraphicsRectItem(f3);

    itemLeft->setBrush(QBrush(Qt::red));
    itemLeft->setZValue(1);
    itemMid->setBrush(QBrush(Qt::red));
    itemMid->setZValue(0);
    itemRight->setBrush(QBrush(Qt::red));
    itemRight->setZValue(2);

    addToGroup(itemLeft);
    addToGroup(itemMid);
    addToGroup(itemRight);
//QGraphicsItemGroup::ItemIsNotSelectable
    setFlags(QGraphicsItemGroup::ItemIsSelectable|QGraphicsItemGroup::ItemIsMovable|QGraphicsItemGroup::ItemIsFocusable|QGraphicsItemGroup::ItemSendsGeometryChanges|QGraphicsItemGroup::ItemSendsGeometryChanges);

    auto l = childItems();
    for(auto const &i:l)
        qDebug() << i;


}
bool TGate::groupSelected(QPointF point)
{
    QRectF rectf = sceneBoundingRect();

    return((point.x() >= rectf.left() )&&  (point.x() <= rectf.right())
            && (point.y() >= rectf.top() )&&  (point.y() <= rectf.bottom()));
}
QRectF TGate::posRange()
{

    return(sceneBoundingRect());
}


void TGate::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    auto itemList = childItems();
    auto pos = event->scenePos();

    if((event->button() == Qt::LeftButton) && (itemSelected(itemList.at(1), pos) ||itemSelected(itemList.at(2), pos) ))
    {
        m_moving = true;
    }

    QGraphicsItemGroup::mousePressEvent(event);
}

void TGate::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->buttons()&Qt::LeftButton)
    {

        qDebug() << "Scene pos" <<  event->scenePos();
        qDebug() << "Event pos" << event->pos();
        auto itemList = childItems();

        auto pos = event->pos();
        auto scenePos = event->scenePos();
        if(m_moving)
        {

            // auto itemPos = mapFromScene(pos);
            // auto itemPos = pos;
            // event pos is the item pos
            // scene pos is the parent scene pos

            if(pos.x()>M_XBOUND)
            {
                // // lengthen middle one
                QGraphicsRectItem *midItem = qgraphicsitem_cast<QGraphicsRectItem *>(itemList.at(0));
                QRectF rectCur = midItem->rect();
                midItem->setRect(rectCur.left(), rectCur.top(), -rectCur.left()+pos.x(), 2);

                // // move right one
                QGraphicsRectItem *rightItem = qgraphicsitem_cast<QGraphicsRectItem *>(itemList.at(2));
                rightItem->setRect(pos.x(), -5, 2, 10);

            }else if (pos.x()<-M_XBOUND)
            {
                // // lengthen middle one
                QGraphicsRectItem *midItem = qgraphicsitem_cast<QGraphicsRectItem *>(itemList.at(0));
                QRectF rectCur = midItem->rect();
                midItem->setRect(pos.x(), rectCur.top(), rectCur.right()-pos.x(), 2);

                // // move left one
                QGraphicsRectItem *leftItem = qgraphicsitem_cast<QGraphicsRectItem *>(itemList.at(1));
                leftItem->setRect(pos.x(), -5, 2, 10);

            }
            else{
                // do nothing
                // itemList.at(0)->moveBy(itemPos.x() - itemList.at(2)->x(), 0);
            }



        }
        else
        {
        // just normal dragging
            qDebug() << m_moving << "Dragging";
        auto lastPos = event->lastScenePos();
        this->moveBy(scenePos.x() - lastPos.x(), scenePos.y() - lastPos.y());
        }
    }
    // QGraphicsItemGroup::mouseMoveEvent(event);

}

void TGate::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_moving = false;
    QGraphicsScene *scene = new QGraphicsScene();

    // should update bounding box here?
    qDebug() << sceneBoundingRect().left() <<sceneBoundingRect().right();

    event->accept();
    QGraphicsItemGroup::mouseReleaseEvent(event);
}

bool TGate::itemSelected(QGraphicsItem *item, QPointF point)
{
    QRectF rectf = item->boundingRect();

    auto itemRect = item->mapFromScene(point);
    return((itemRect.x() >= rectf.left() )&&  (itemRect.x() <= rectf.right())
            && (itemRect.y() >= rectf.top() )&&  (itemRect.y() <= rectf.bottom()));
}


QRectF TGate::boundingRect() const
{
    return childrenBoundingRect();
}
