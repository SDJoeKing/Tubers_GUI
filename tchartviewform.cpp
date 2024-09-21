#include "tchartviewform.h"
#include "qlegendmarker.h"
#include "ui_tchartviewform.h"

TChartViewForm::TChartViewForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TChartViewForm)
{
    ui->setupUi(this);

    m_chartView = ui->graphicsView;
    m_chart = new QChart();
    m_series = new QLineSeries();
    m_ruler = new QLineSeries();

    // config m_ruler
    QBrush brush(Qt::red, Qt::SolidPattern);
    QPen pen(brush, 2, Qt::DashDotLine);
    m_ruler->setPen(pen);
    m_ruler->setVisible(false);
    m_ruler->append(QPointF(0, yMin));
    m_ruler->append(QPointF(0, yMax));
    m_ruler->setOpacity(0.3);

    m_X = new QValueAxis();
    m_Y = new QValueAxis();

    m_chart->addSeries(m_series);
    m_chart->addSeries(m_ruler);
    m_chart->legend()->markers(m_ruler).at(0)->setVisible(false);


    m_chart->addAxis(m_X, Qt::AlignBottom);
    m_chart->addAxis(m_Y, Qt::AlignLeft);

    m_series->attachAxis(m_X);
    m_series->attachAxis(m_Y);
    m_ruler->attachAxis(m_X);
    m_ruler->attachAxis(m_Y);

    m_chartView->setChart(m_chart);
    m_chartView->viewport()->setMouseTracking(true);
    m_chartView->setMouseTracking(true);


    // label and axis format
    m_series->setName("A-scan");
    m_X->setTitleText("A-scan data points");
    m_Y->setTitleText("Amplitude [mV]");
    m_X->setLabelFormat("%.2f");

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
    m_dataTip = generateLabel(m_chartView);

    m_dataTip->setVisible(false);
    m_chartView->setDragMode(QGraphicsView::RubberBandDrag);

    // m_chartView->viewport()->installEventFilter(this);

    ui->btnBack->setEnabled(false);


    // connect
    connect(m_X, &QValueAxis::rangeChanged, ui->btnBack, &QPushButton::setEnabled);
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
        float _tempMax = pointToDepth(m_X->max());
        m_X->setRange( pointToDepth(m_X->min()), _tempMax);
        _xAxisType = AXISTYPE::DEPTH;
        updateXRange(_tempMax);

    }else if(type == AXISTYPE::SAMPLE)
    {
        auto _tempMax =depthToPoint(m_X->max() );
        m_X->setTitleText("A-scan data points");
        m_X->setRange(depthToPoint(m_X->min()), _tempMax);
        updateXRange(_tempMax);
        _xAxisType = AXISTYPE::SAMPLE;

    }else if(type == AXISTYPE::ABSOLUTEY)
    {
        m_Y->setRange(0, m_Y->max());
        _yAxisType = AXISTYPE::ABSOLUTEY;
    }else
        m_Y->setRange(-m_Y->max(), m_Y->max());
        _yAxisType = AXISTYPE::FULLY;
}

void TChartViewForm::setVelocity(qfloat16 vel)
{
    m_vel=vel;
}

void TChartViewForm::clear()
{
    m_series->clear();
}

void TChartViewForm::backButtonEnabled()
{
    // ui->btnBack->setE
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

bool TChartViewForm::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == m_chartView->viewport() && _dataTipOn)
    {

        if(event->type() == QEvent::MouseMove)
        {
            m_dataTip->setVisible(true);
            auto mouse = static_cast<QMouseEvent *>(event);
            auto pos = mouse->position();
            auto value = m_chart->mapToValue(pos, m_series);

            m_dataTip->move(pos.toPoint().x()+20, pos.toPoint().y()-35 );
            m_dataTip->setText(QString("X: %1\nY: %2").arg(value.x(), 0, 'f', 2).arg(value.y(), 0, 'f', 2));

            // draw verticle line
            m_ruler->replace(QList<QPointF>{QPointF(value.x(), m_Y->min()), QPointF(value.x(), m_Y->max())});


        }else if(event->type() == QEvent::Leave)
        {
            m_dataTip->setVisible(false);
        }
        // To implement click data points
        else if(event->type() == QEvent::MouseButtonPress)
        {
            auto mouse =  static_cast<QMouseEvent *>(event);
            if(mouse->button()==Qt::LeftButton && m_series->count() >0)
            {
                // get mouse pos

                auto value = m_chart->mapToValue(mouse->pos(), m_series);

                if(_xAxisType == AXISTYPE::DEPTH)
                    value.setX(depthToPoint(value.x()));

                qreal _Yvalue = m_series->at(value.x()).y();
                if(qAbs(value.y() - _Yvalue)<0.01*(m_Y->max() - m_Y->min()))
                {
                    m_series->selectPoint(value.x());
                    m_series->setSelectedColor(Qt::red);
                    value.setY(m_series->at(value.x()).y());
                    auto labelPos = m_chart->mapToPosition(value, m_series);
                    auto _tempLabel = generateLabel(m_chartView);
                    _tempLabel->setText(QString("X: %1\nY: %2").arg(value.x(), 0, 'f', 2).arg(value.y(), 0, 'f', 2));
                    _tempLabel->move(labelPos.toPoint().x()+20, labelPos.toPoint().y()-35 );
                    _tempLabel->setObjectName(QString("%1").arg(value.x()));
                    _dataTipList.emplaceBack(_tempLabel);
                }

            }
            else if(mouse->button()==Qt::RightButton && m_series->count() >0)
            {
                m_series->deselectAllPoints();
                for(auto &i:_dataTipList)
                    delete i;
                _dataTipList.clear();
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void TChartViewForm::on_btnDataTip_clicked(bool checked)
{
    // data tip
    _dataTipOn = checked;
    m_ruler->setVisible(checked);
    m_chart->legend()->markers(m_ruler).at(0)->setVisible(false);
    if(!checked)
    {
        m_series->deselectAllPoints();
        for(auto &i:_dataTipList)
            delete i;
        _dataTipList.clear();
        m_chartView->viewport()->removeEventFilter(this);
        return;
    }
    auto mouse = m_chartView->mapFromGlobal(QCursor::pos());
    auto value = m_chart->mapToValue(mouse, m_series);
    // draw verticle line
    m_ruler->replace(QList<QPointF>{QPointF(value.x(), yMin), QPointF(value.x(), yMax)});
    m_chartView->viewport()->installEventFilter(this);
}

void TChartViewForm::on_btnZoom_clicked(bool checked)
{
    _zoomOn = checked;
    // m_chartView->rubberBandOn(checked);
    // if(checked)
    // {
    //     connect(m_chartView, &TChartView::selectedRubberBand, this, &TChartViewForm::doZoomInOut);
    // }
    // else
    // {
    //     disconnect(m_chartView, &TChartView::selectedRubberBand, this, &TChartViewForm::doZoomInOut);
    // }
    if(checked)
        m_chartView->setDragMode(QChartView::RubberBandDrag);
    else
        m_chartView->setDragMode(QChartView::NoDrag);
}
void TChartViewForm::doZoomInOut(QRectF rubberband)
{
    if(rubberband.right()-rubberband.left() <20)
        return;

    auto startPos = m_chart->mapToValue(rubberband.topLeft(), m_series);
    auto endPos = m_chart->mapToValue(rubberband.bottomRight(), m_series);
    m_X->setRange(startPos.x(), endPos.x());
    m_Y->setRange(endPos.y(), startPos.y());
    qDebug()<< startPos << endPos << rubberband;
}


void TChartViewForm::on_btnReset_clicked(bool checked)
{
    m_X->setRange(xMin, xMax);
    m_Y->setRange(yMin, yMax);
}

void TChartViewForm::updateXRange(float new_xMax)
{
    xMax = new_xMax;
}

QLabel *TChartViewForm::generateLabel(QWidget *parent)
{
    QLabel *_temp = new QLabel(parent);
    _temp->setGeometry(0,0, 80, 50);
    auto font = _temp->font();
    font.setBold(true);
    font.setPointSize(8);
    _temp->setFont(font);
    _temp->setStyleSheet("QLabel {color:black;}");
    _temp->setVisible(true);
    return _temp;
}

int TChartViewForm::depthToPoint(qfloat16 depth)
{
    return depth* 2 * 125e6 / m_vel / 1000;
}

qfloat16 TChartViewForm::pointToDepth(int point)
{
    return point / 2/ 125e6 * m_vel * 1000;
}

void TChartViewForm::on_btnSave_clicked()
{
    QPixmap pix = m_chartView->grab();
    QString path = QFileDialog::getSaveFileName(this, "Save Figure", QApplication::applicationDirPath(), "Image (*.png *.jpg)");
    bool success = false;
    success = pix.save(path);
    if(!success)
        QMessageBox::warning(this, "Warning", "Not able to save the image");
}

