#include "tchartviewform.h"
#include "ui_tchartviewform.h"

TChartViewForm::TChartViewForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TChartViewForm)
{
    ui->setupUi(this);

    m_chartView = ui->graphicsView;
    m_chart = new QChart();
    m_series = new QLineSeries();

    m_X = new QValueAxis();
    m_Y = new QValueAxis();

    m_chart->addSeries(m_series);


    m_chart->addAxis(m_X, Qt::AlignBottom);
    m_chart->addAxis(m_Y, Qt::AlignLeft);

    m_series->attachAxis(m_X);
    m_series->attachAxis(m_Y);


    m_chartView->setChart(m_chart);
    m_chartView->viewport()->setMouseTracking(true);


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
    m_chartView->scene()->addItem(m_gate1);
    m_chartView->scene()->addItem(m_gate2);
    m_dataTip = new QLabel(this);
    auto font = m_dataTip->font();
    font.setBold(true);
    font.setPointSize(8);

    // palette.setColor(QPalette::Text, Qt::blue);
    m_dataTip->setFont(font);
    m_dataTip->setStyleSheet("QLabel {background-color: white; color:black;}");
    m_dataTip->setVisible(false);
}

TChartViewForm::~TChartViewForm()
{
    delete ui;
}


void TChartViewForm::plot(QList<QPointF> data)
{
    m_series->replace(data);
}

void TChartViewForm::changeAxisType(TChartViewForm::AXISTYPE type)
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

void TChartViewForm::setVelocity(qfloat16 vel)
{
    m_vel=vel;
}

void TChartViewForm::clear()
{
    m_series->clear();
}

void TChartViewForm::toogleGates(bool arg)
{
    m_gate1->setVisible(arg);
    m_gate2->setVisible(arg);

    QRectF sceneRect = m_chartView->sceneRect();
    QPointF gate1ScenePos = m_gate1->scenePos();
    QPointF gate2ScenePos = m_gate2->scenePos();
    m_gate1->moveBy(sceneRect.width()/2 - gate1ScenePos.x() - 30, sceneRect.height()/2 - gate1ScenePos.y());
    m_gate2->moveBy(sceneRect.width()/2- gate2ScenePos.x() + 30, sceneRect.height()/2- gate2ScenePos.y());
}


void TChartViewForm::on_btnDataTip_clicked(bool checked)
{
    // data tip
    if(checked)
    {
        m_chartView->viewport()->installEventFilter(this);
    }
    else
    {
        m_chartView->viewport()->removeEventFilter(this);
    }
}

bool TChartViewForm::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == m_chartView->viewport())
    {

        if(event->type() == QEvent::MouseMove)
        {
            m_dataTip->setVisible(true);
            auto mouse = static_cast<QMouseEvent *>(event);
            auto pos = mouse->position();

            auto position = m_chart->mapFromScene(pos);
            auto value = m_chart->mapToValue(position, m_series);

            m_dataTip->move(pos.toPoint());
            m_dataTip->setText(QString("X: %1, Y: %2").arg(value.x()).arg(value.y()));

        }else if(event->type() == QEvent::Leave)
        {
            m_dataTip->setVisible(false);
        }

        // To implement click data points

    }

    if(watched == m_X)
        qDebug() << "Axis!";

    return QWidget::eventFilter(watched, event);
}

void TChartViewForm::on_btnZoom_clicked(bool checked)
{
    if(checked)
        m_chartView->setDragMode(QGraphicsView::RubberBandDrag);
    else
        m_chartView->setDragMode(QGraphicsView::NoDrag);

}

