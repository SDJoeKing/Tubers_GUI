#include "tchartview.h"

TChartView::TChartView(QWidget *parent) : QChartView{parent}
{
    setDragMode(QChartView::NoDrag);

}

void TChartView::rubberBandOn(bool on)
{
    if(on)
        setDragMode(QChartView::RubberBandDrag);
    else
        setDragMode(QChartView::NoDrag);
}



void TChartView::mousePressEvent(QMouseEvent *event)
{

    QChartView::mousePressEvent(event);
}

void TChartView::mouseReleaseEvent(QMouseEvent *event)
{
    emit selectedRubberBand(this->rubberBandRect());
    QChartView::mouseReleaseEvent(event);
}

void TChartView::mouseMoveEvent(QMouseEvent *event)
{
    // auto pos = event->position();

    // qDebug()<< "MOveevent pos" << pos << sceneRect();
    // if(pos.x() < sceneRect().width() && pos.x()>sceneRect().width()*0.1
    //     && pos.y() < sceneRect().height() && pos.y() > sceneRect().height()*0.9)
    // {if(this->cursor() != Qt::SizeHorCursor) {setCursor(Qt::SizeHorCursor);}}
    // else if(pos.x() < sceneRect().width()*0.1
    //          && pos.y() < sceneRect().height()*0.9)
    // {if(this->cursor() != Qt::SizeVerCursor) {setCursor(Qt::SizeVerCursor);}}
    // else
    // {if(this->cursor() != Qt::ArrowCursor) setCursor(Qt::ArrowCursor);}

    QChartView::mouseMoveEvent(event);
}


void TChartView::wheelEvent(QWheelEvent *event)
{
    QChartView::wheelEvent(event);
}

