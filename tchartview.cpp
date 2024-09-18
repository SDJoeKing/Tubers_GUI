#include "tchartview.h"

TChartView::TChartView(QWidget *parent) : QChartView{parent}
{
    m_chart = new QChart();
    m_series = new QLineSeries(this);
    m_X = new QValueAxis();
    m_Y = new QValueAxis();

    m_chart->addSeries(m_series);
    m_chart->addAxis(m_X, Qt::AlignBottom);
    m_chart->addAxis(m_Y, Qt::AlignLeft);
    m_series->attachAxis(m_X);
    m_series->attachAxis(m_Y);
    this->setChart(m_chart);

    this->setContextMenuPolicy(Qt::CustomContextMenu);

    // chart scene
    // auto frameRect = this->rect();
    // this->setSceneRect(frameRect.x()+10, frameRect.y()+10, frameRect.height()-20, frameRect.width()-20);

    // label and axis format
    m_series->setName("A-scan");
    m_X->setTitleText("A-scan data points");
    m_Y->setTitleText("Amplitude [mV]");
    m_X->setLabelFormat("%d");
    m_X->setRange(xMin, xMax);
    m_Y->setRange(yMin, yMax);

    // gate
    m_gate1 = new TGate();
    m_gate2 = new TGate();
    m_gate1->setZValue(100);
    m_gate2->setZValue(100);
    m_gate1->setVisible(false);
    m_gate2->setVisible(false);
    m_gate1->setFlags(m_gate1->flags()| QGraphicsItem::ItemIsSelectable);
    m_gate2->setFlags(m_gate2->flags()| QGraphicsItem::ItemIsSelectable);

    // add gate to scene
    this->scene()->addItem(m_gate1);
    this->scene()->addItem(m_gate2);
    m_gate1->moveBy(-10, 0);
    m_gate2->moveBy(10, 0);

}

void TChartView::plot(QList<QPointF> data, bool axisIsDepth)
{

}

// void TChartView::toogleGates(bool arg)
// {
//     m_gate1->setVisible(arg);
//     m_gate2->setVisible(arg);
// }


// void TChartView::resizeEvent(QResizeEvent *event)
// {
//     auto frameRect = this->rect();
//     this->setSceneRect(frameRect.x()+10, frameRect.y()+10, frameRect.height()-20, frameRect.width()-20);
//     QChartView::resizeEvent(event);
// }
