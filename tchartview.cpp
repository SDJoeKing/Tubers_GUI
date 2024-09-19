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
    auto frameRect = this->geometry();
    this->setSceneRect(frameRect.x()+10, frameRect.y()+10, frameRect.height()-20, frameRect.width()-20);
    qDebug() << this->geometry();

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

}

void TChartView::plot(QList<QPointF> data)
{
    m_series->replace(data);
}

void TChartView::changeAxisType(TChartView::AXISTYPE type)
{
    if(type == AXISTYPE::DEPTH)
    {
        m_X->setTitleText("Depth [mm]");
        m_X->setRange(0, xMax / 2/ 125e6 * m_vel * 1000);

    }else if(type == AXISTYPE::SAMPLE)
    {
        m_X->setTitleText("A-scan data points");
        m_X->setRange(0, xMax);

    }else if(type == AXISTYPE::ABSOLUTEY)
    {
        m_Y->setRange(0, yMax);
    }else
        m_Y->setRange(yMin, yMax);
}

void TChartView::setVelocity(qfloat16 vel)
{
    m_vel=vel;
}

void TChartView::clear()
{
    m_series->clear();
}

void TChartView::toogleGates(bool arg)
{
    m_gate1->setVisible(arg);
    m_gate2->setVisible(arg);

    QRectF sceneRect = this->sceneRect();
    QPointF gate1ScenePos = m_gate1->scenePos();
    QPointF gate2ScenePos = m_gate2->scenePos();
    m_gate1->moveBy(sceneRect.width()/2 - gate1ScenePos.x() - 30, sceneRect.height()/2 - gate1ScenePos.y());
    m_gate2->moveBy(sceneRect.width()/2- gate2ScenePos.x() + 30, sceneRect.height()/2- gate2ScenePos.y());
}


void TChartView::resizeEvent(QResizeEvent *event)
{
    auto frameRect = this->geometry();
    this->setSceneRect(frameRect.x()+10, frameRect.y()+10, frameRect.height()-20, frameRect.width()-20);
    QChartView::resizeEvent(event);
}


