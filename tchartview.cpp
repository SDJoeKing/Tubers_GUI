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
}
