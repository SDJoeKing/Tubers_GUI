#include "tgate.h"
#include "tchartviewform.h"
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <qwidget.h>
TGate::TGate(QWidget * canvas, QGraphicsItem *parent) : QGraphicsItemGroup{parent}
{
    _canvas = canvas;
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

    auto mtchartviewForm = qobject_cast<TChartViewForm *>(_canvas);
    m_ascanValue = mtchartviewForm->ascanValue(itemMid->scenePos());

}
bool TGate::groupSelected(QPointF point)
{
    QRectF rectf = sceneBoundingRect();

    return((point.x() >= rectf.left() )&&  (point.x() <= rectf.right())
            && (point.y() >= rectf.top() )&&  (point.y() <= rectf.bottom()));
}

QPointF TGate::ascanValue() const
{
    return m_ascanValue;
}

void TGate::updateAscanValue()
{
    auto itemList = childItems();
    QGraphicsRectItem * midItem = qgraphicsitem_cast<QGraphicsRectItem *>(itemList.at(0));
    auto mtchartviewForm = qobject_cast<TChartViewForm *>(_canvas);
    m_ascanValue = mtchartviewForm->ascanValue(midItem->scenePos());
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
        QGraphicsRectItem * leftItem = qgraphicsitem_cast<QGraphicsRectItem *>(itemList.at(1));
        QGraphicsRectItem * midItem = qgraphicsitem_cast<QGraphicsRectItem *>(itemList.at(0));
        QGraphicsRectItem * rightItem = qgraphicsitem_cast<QGraphicsRectItem *>(itemList.at(2));
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

                QRectF rectCur = midItem->rect();
                midItem->setRect(rectCur.left(), rectCur.top(), -rectCur.left()+pos.x(), 2);

                // // move right one

                rightItem->setRect(pos.x(), -5, 2, 10);

            } else if (pos.x()<-M_XBOUND)
            {
                // // lengthen middle one

                QRectF rectCur = midItem->rect();
                midItem->setRect(pos.x(), rectCur.top(), rectCur.right()-pos.x(), 2);

                // // move left one

                leftItem->setRect(pos.x(), -5, 2, 10);

            }
            else{
                // do nothing
            }

        }
        else
        {
        // just normal dragging
            qDebug() << m_moving << "Dragging";
        // limit the scene size so the gate will not go overborder

            auto leftRect = leftItem->mapRectToScene(leftItem->rect());
            auto rightRect = leftItem->mapRectToScene(rightItem->rect());
            auto midRect = midItem->mapRectToScene(midItem->rect());
            auto chartview = qobject_cast<TChartViewForm *>(_canvas);

            if(leftRect.x() <= chartview->chartRect().left())
                this->moveBy(1 - leftRect.x(), 0);
            if(rightRect.x() >= chartview->chartRect().right())
                this->moveBy( chartview->chartRect().right() - rightRect.x(), 0);
            if(midRect.y() >= chartview->chartRect().bottom())
                this->moveBy(0,  chartview->chartRect().bottom() - midRect.y());

            updateAscanValue();


            if(midRect.y() <= chartview->chartRect().top())
            {
                this->moveBy(0,   + 1);
                updateAscanValue();
                return;
            }
            auto lastPos = event->lastScenePos();
            this->moveBy(scenePos.x() - lastPos.x(), scenePos.y() - lastPos.y());
            updateAscanValue();

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
