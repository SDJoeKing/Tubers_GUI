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



// void TChartView::mousePressEvent(QMouseEvent *event)
// {
//     QChartView::mousePressEvent(event);
// }

// void TChartView::mouseReleaseEvent(QMouseEvent *event)
// {
//     emit selectedRubberBand(this->rubberBandRect());
//     QChartView::mouseReleaseEvent(event);
// }

// void TChartView::mouseMoveEvent(QMouseEvent *event)
// {
//     QChartView::mouseMoveEvent(event);
// }


// void TChartView::wheelEvent(QWheelEvent *event)
// {
//     QChartView::wheelEvent(event);
// }

