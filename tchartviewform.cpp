#include "tchartviewform.h"
#include "qgraphicslayout.h"
#include "qlegendmarker.h"
#include "ui_tchartviewform.h"
QElapsedTimer timerV;

QString lengthAxisTitle = "A-scan length [ms]";
QString depthAxisTitle = "Depth [mm]";

TChartViewForm::TChartViewForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TChartViewForm)
{
    ui->setupUi(this);



    m_chartView = ui->graphicsView;
    m_chart = new QChart();
    m_series = new QLineSeries();
    m_ruler = new QLineSeries();

    // m_series->setUseOpenGL(true);
    // m_ruler->setUseOpenGL(true);

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
    // m_chart->legend()->markers(m_ruler).at(0)->setVisible(false);
    // m_chart->legend()->markers(m_series).at(0)->setVisible(false);
    m_chart->legend()->hide();


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
    m_X->setTitleText(lengthAxisTitle);
    m_Y->setTitleText("Amplitude [mV]");
    m_X->setLabelFormat("%.3f");
    m_X->setMinorTickCount(2);
    m_X->applyNiceNumbers();


    // update the value of xMax based on fs selected

    m_X->setRange(xMin, xMax);
    m_Y->setRange(yMin, yMax);

    m_chartView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    // gate
    m_gate1 = new TGate(this);
    m_gate2 = new TGate(this);
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

    //wheel zoom in n out
    m_chartView->installEventFilter(this);

    ui->btnBack->setEnabled(false);
    ui->btnSave->setEnabled(false);
    // chart tight layout
    m_chart->layout()->setContentsMargins(0, 0, 0, 0);
    m_chart->setBackgroundRoundness(0);

    // timer
    m_timer.setInterval(1000);

    // connect
    connect(m_X, &QValueAxis::rangeChanged, ui->btnBack, &QPushButton::setEnabled);
    // connect(m_X, &QValueAxis::rangeChanged, this, &TChartViewForm::backButtonEnabled);
    connect(&m_timer, &QTimer::timeout, this, &TChartViewForm::doThicknessCal);


    // units
    setXUnit("ms");
    setYUnit("mV");
    ui->steps->setValue((m_X->max() - m_X->min()) * 0.05);
}

TChartViewForm::~TChartViewForm()
{
    delete ui;
}


void TChartViewForm::plot(const QList<QPointF> &dataptr, bool p)
{
    Q_UNUSED(p);
    // const QSignalBlocker blocker(m_chartView);

    {
        timerV.restart();
        qDebug() << "datasize: "<< dataptr.size();
        m_series->replace(dataptr);
    }
    qDebug() << "paint " << timerV.durationElapsed();

    m_chartView->update();

}

void TChartViewForm::changeXAxisType(const TChartViewForm::x_AXISTYPE &type)
{

    if(_xAxisType == x_AXISTYPE::TIME && type==x_AXISTYPE::DEPTH)
    {
        _xAxisType = type;

        m_X->setTitleText(depthAxisTitle);
        float _tempMax = timeToDepth(m_X->max());
        float _tempMin = timeToDepth(m_X->min());

        if(!(m_series->points().isEmpty()))
        {
            QList<QPointF> _tempPoints;
            for(auto &point : m_series->points())
                _tempPoints.emplace_back(timeToDepth(point.x()) , point.y());

            m_X->setRange(timeToDepth(xMin), timeToDepth(xMax));
            plot(_tempPoints, true);
        }

        m_X->setRange( _tempMin, _tempMax);
        updateLabelPosition();

        setXUnit("mm");
    }
    else if(_xAxisType == x_AXISTYPE::DEPTH && type == x_AXISTYPE::TIME)
    {
        _xAxisType = type;

        auto _tempMax =depthToTime(m_X->max() );
        float _tempMin = depthToTime(m_X->min());

        m_X->setTitleText(lengthAxisTitle);

        if(!(m_series->points().isEmpty()))
        {
            QList<QPointF> _tempPoints;
            for(auto &point : m_series->points())
                _tempPoints.emplace_back(depthToTime(point.x()) , point.y());
            m_X->setRange(xMin, xMax);
            plot(_tempPoints, true);
        }

        m_X->setRange(_tempMin, _tempMax);
        updateLabelPosition();

        setXUnit("ms");
    }

    // trigger axiscombo change
    if(ui->comboAxis->currentIndex() == 0)
    {
        ui->comboAxis->setCurrentIndex(1);
        ui->comboAxis->setCurrentIndex(0);
    }

    m_gate1->updateAscanValue();
    m_gate2->updateAscanValue();
}

void TChartViewForm::changeYAxisType(const TChartViewForm::y_AXISTYPE &type)
{

    if(_yAxisType == y_AXISTYPE::FULL && type == y_AXISTYPE::RECTIFY)
    {
        _yAxisType = y_AXISTYPE::RECTIFY;
        auto max = m_Y->max() < yRangeMax ? m_Y->max() : yRangeMax;
        m_Y->setRange(0, max);

        updateLabelPosition();


    }else if((_yAxisType == y_AXISTYPE::RECTIFY && type == y_AXISTYPE::FULL))
    {
        _yAxisType = y_AXISTYPE::FULL;
        auto max = m_Y->max() < yRangeMax ? m_Y->max() : yRangeMax;
        m_Y->setRange(-max, max);

        updateLabelPosition();
    }

}

void TChartViewForm::setVelocity(double vel)
{
    float ratio = vel / m_vel;

    m_vel=vel;

    if(_xAxisType == x_AXISTYPE::DEPTH)
    {
        float _tempMax = m_X->max() * ratio ;
        float _tempMin = m_X->min() * ratio ;
        m_X->setRange( _tempMin, _tempMax);
    }


    updateLabelPosition();
}

void TChartViewForm::clear()
{
    m_series->clear();
}

void TChartViewForm::backButtonEnabled(bool arg)
{
    if(!arg)
        zoomRectTrack.clear();
}

void TChartViewForm::toggleGates(bool arg)
{
    m_gate1->setVisible(arg);
    m_gate2->setVisible(arg);

    QRectF sceneRect = m_chartView->sceneRect();
    QPointF gate1ScenePos = m_gate1->scenePos();
    QPointF gate2ScenePos = m_gate2->scenePos();
    m_gate1->moveBy(sceneRect.width()/2 - gate1ScenePos.x() - 30, sceneRect.height()/2 - gate1ScenePos.y());
    m_gate2->moveBy(sceneRect.width()/2- gate2ScenePos.x() + 30, sceneRect.height()/2- gate2ScenePos.y());

    m_gate1->updateAscanValue();
    m_gate2->updateAscanValue();

}

bool TChartViewForm::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == m_chartView->viewport() && _dataTipOn)
    {

        if(event->type() == QEvent::MouseMove)
        {

            auto mouse = static_cast<QMouseEvent *>(event);
            auto pos = mouse->position();
            auto value = m_chart->mapToValue(pos, m_series);
            // draw verticle line
            m_ruler->replace(QList<QPointF>{QPointF(value.x(), m_Y->min()), QPointF(value.x(), m_Y->max())});

            m_dataTip->move(pos.toPoint().x()+20, pos.toPoint().y()-35 );
            m_dataTip->setText(QString("X: %1\nY: %2").arg(value.x(), 0, 'f', 4).arg(value.y(), 0, 'f', 4));
            m_dataTip->setVisible(true);

        }else if(event->type() == QEvent::Leave)
        {
            m_dataTip->setVisible(false);
        }
        // To implement click data points
        else if(event->type() == QEvent::MouseButtonPress && !acquisitionRunning)
        {   qDebug() << "mouse click triggered";
            auto mouse =  static_cast<QMouseEvent *>(event);
            if(mouse->button()==Qt::LeftButton && m_series->count() >0)
            {
                // get mouse pos

                auto value = m_chart->mapToValue(mouse->pos(), m_series);
                auto xposition = _xAxisType == x_AXISTYPE::TIME ? value.x() : depthToTime(value.x());  // this should be time
                int index = static_cast<int>(xposition / xMax * mTcpClient::DATA_SIZE/2);
                qreal _Yvalue = m_series->at(index).y();

                if(qAbs(value.y() - _Yvalue)<0.05*(m_Y->max() - m_Y->min()))
                {
                    m_series->selectPoint(index);
                    // qDebug() << "index select " << index;
                    m_series->setSelectedColor(Qt::red);
                    value.setY(m_series->at(index).y());

                    auto _tempLabel = generateLabel(m_chartView);

                    _tempLabel->setText(QString("X: %1\nY: %2").arg(value.x(), 0, 'f', 4).arg(value.y(), 0, 'f', 4));

                    // object name is always the index as x
                    _tempLabel->setObjectName(QString("%1;%2").arg(xposition).arg(value.y())); // xposition always means time, saved as time
                    _dataTipList.emplaceBack(_tempLabel);

                    updateLabelPosition();
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

    // wheel event
    if(watched == m_chartView && event->type()==QEvent::Wheel)
    {
        float _min = m_Y->min();
        float _max = m_Y->max();

        QWheelEvent *wheel = static_cast<QWheelEvent *>(event);
        if(wheel->angleDelta().y()>0)
        {
            qDebug() << wheel->angleDelta().y();
            if(_min < 0)
                _min*=1.1;
            else
                _min /= 1.1;

            if(_max < 0)
                _max/=1.1;
            else
                _max *=1.1;
        }
        else
        {
            qDebug() << wheel->angleDelta().y();
            if(_min < 0)
                _min/=1.1;
            else
                _min *= 1.1;

            if(_max < 0)
                _max*=1.1;
            else
                _max /=1.1;
        }
        if(_yAxisType == y_AXISTYPE::RECTIFY && _min<0)
            _min = 0;
        auto vpp = _max - _min;
        if(vpp < yRangeMax*1.1 && vpp > yRangeMin*1.1)
            m_Y->setRange(_min, _max);

        updateLabelPosition();
    }

    return QWidget::eventFilter(watched, event);
}

void TChartViewForm::on_btnDataTip_clicked(bool checked)
{

    if(ui->btnZoom->isChecked() && checked)
        ui->btnZoom->click();

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
    if(ui->btnDataTip->isChecked() && checked)
        ui->btnDataTip->click();

    _zoomOn = checked;
    m_chartView->rubberBandOn(checked);
    if(checked)
    {
        connect(m_chartView, &TChartView::selectedRubberBand, this, &TChartViewForm::doZoomInOut);
    }
    else
    {
        disconnect(m_chartView, &TChartView::selectedRubberBand, this, &TChartViewForm::doZoomInOut);
    }

}
void TChartViewForm::doZoomInOut(QRectF rubberband)
{
    if(rubberband.right()-rubberband.left() <20)
        return;

    // push the rubberband into track queue
    QRectF currentFrame(QPointF(m_X->min(), m_Y->max()), QPointF(m_X->max(), m_Y->min()));
    zoomRectTrack.push(QPair<QRectF, x_AXISTYPE>(currentFrame, _xAxisType));

    auto startPos = m_chart->mapToValue(rubberband.topLeft(), m_series);
    auto endPos = m_chart->mapToValue(rubberband.bottomRight(), m_series);


    if(_yAxisType == y_AXISTYPE::RECTIFY)
        endPos.ry() < 0 ? endPos.ry() = 0 : endPos.ry();
    startPos.rx() < 0 ? startPos.rx() = 0: startPos.rx();

    m_X->setRange(startPos.x(), endPos.x());
    m_Y->setRange(endPos.y(), startPos.y());

    updateLabelPosition();

    qDebug()<< startPos << endPos << rubberband;
}

void TChartViewForm::acquisitionStatus(bool isRunning)
{
    acquisitionRunning = isRunning;
}

void TChartViewForm::toggleSave(bool arg)
{
    ui->btnSave->setEnabled(!arg);
}

void TChartViewForm::startThickCal(bool arg)
{
    if(arg)
        m_timer.start();
    else
        m_timer.stop();
}

void TChartViewForm::updateFs(const float newFs)
// newFs comes in without e6
{
    fs = newFs*1e6;
    updateXMax(fs);
}

void TChartViewForm::reset()
{
    if(ui->btnDataTip->isChecked())
        ui->btnDataTip->click();
    m_series->clear();
    ui->btnReset->click();
}

QRectF TChartViewForm::chartRect() const
{
    return this->m_chart->rect();
}

QPointF TChartViewForm::ascanValue(const QPointF & pos) const
{
    return m_chart->mapToValue(pos, m_series);
}


void TChartViewForm::on_btnReset_clicked(bool checked)
{
    m_X->setRange(xMin, xMax);
    m_Y->setRange(0, yMax);
    auto currentPosition = m_ruler->points();
    m_ruler->replace(QList<QPointF>{QPointF(currentPosition.at(0).x(), yMin), QPointF(currentPosition.at(0).x(), yMax)});
    updateLabelPosition();
    if(_yAxisType==y_AXISTYPE::FULL)
    {
        emit setRectifyCheck();
    }
    if(_xAxisType == x_AXISTYPE::DEPTH)
    {
        m_X->setRange(timeToDepth(xMin), timeToDepth(xMax));
    }

    // empty any residual zoomRectTrack
    zoomRectTrack.clear();
    ui->btnBack->setEnabled(false);
}


QLabel *TChartViewForm::generateLabel(QWidget *parent)
{
    QLabel *_temp = new QLabel(parent);
    _temp->setGeometry(0,0, 80, 50);
    auto font = _temp->font();
    font.setBold(true);
    font.setPointSize(8);
    _temp->setFont(font);
    _temp->setStyleSheet("QLabel {color:black; background-color: transparent}");
    _temp->setVisible(true);
    return _temp;
}

double TChartViewForm::depthToTime(const double &depth)
{
    return depth* 2 / m_vel  ;
}

double TChartViewForm::timeToDepth(const double &time)
{
    return time * m_vel / 2  ;
}

void TChartViewForm::on_btnSave_clicked()
{
    QPixmap pix = m_chartView->grab();
    QString path = QFileDialog::getSaveFileName(this, "Save Figure", QApplication::applicationDirPath(), "Image (*.png *.jpg)");
    bool success = false;
    success = pix.save(path);
    if(!success && !path.isEmpty())
        QMessageBox::warning(this, "Warning", "Not able to save the image");
}

void TChartViewForm::updateXMax(const float &newFs)
{
    // reset paint
    m_series->clear();
    qDebug() << newFs;
    xMax = mTcpClient::DATA_SIZE/2/newFs * 1e3;
    m_X->setRange(xMin, xMax);
}

void TChartViewForm::updateLabelPosition()
{

    qDebug() << "in UpdateLabelPosition" <<_yAxisType;
    if(!_dataTipList.empty())
    {
        for(auto &_tempLabel : _dataTipList)
        {
            QString strValue = _tempLabel->objectName();
            QPointF value(strValue.split(";").at(0).toFloat(), strValue.split(";").at(1).toFloat());

            if(_xAxisType == x_AXISTYPE::DEPTH)
                value.setX(timeToDepth(value.x()));

            _tempLabel->setVisible(true);

            if(!inRange(value, m_X, m_Y))
            {
                _tempLabel->hide();
                continue;
            }
            auto labelPos = m_chart->mapToPosition(value, m_series);
            _tempLabel->move(labelPos.toPoint().x()+20, labelPos.toPoint().y()-35 );
            _tempLabel->setText(QString("X: %1\nY: %2").arg(value.x(), 0, 'f', 4).arg(value.y(), 0, 'f', 4));
        }
    }
}

bool TChartViewForm::inRange(const QPointF &a, QValueAxis *xaxis, QValueAxis *yaxis)
{
    return ((a.x() < xaxis->max()) && (a.x() > xaxis->min() )&& (a.y() < yaxis->max()) && (a.y() > yaxis->min()));
}

void TChartViewForm::on_btnBack_clicked()
{
    auto _pair = zoomRectTrack.pop();
    auto rect = _pair.first;
    x_AXISTYPE _oldAxisType = _pair.second;

    auto startPos = rect.topLeft();
    auto endPos = rect.bottomRight();

    if(_xAxisType == x_AXISTYPE::TIME && _oldAxisType==x_AXISTYPE::DEPTH)
    {
        startPos.setX(depthToTime(startPos.x()));
        endPos.setX(depthToTime(endPos.x()));
    }else if(_xAxisType==x_AXISTYPE::DEPTH && _oldAxisType == x_AXISTYPE::TIME)
    {
        startPos.setX(timeToDepth(startPos.x()));
        endPos.setX(timeToDepth(endPos.x()));
    }

    m_X->setRange(startPos.x(), endPos.x());
    if(_yAxisType==y_AXISTYPE::FULL )
        m_Y->setRange(endPos.y(), startPos.y());
    else
        m_Y->setRange(qAbs(endPos.y()), qAbs(startPos.y()));


    updateLabelPosition();

    if(zoomRectTrack.isEmpty())
        ui->btnBack->setEnabled(false);
}

void TChartViewForm::doThicknessCal()
{
    // locate two gate position
    QRectF gate1_range = m_gate1->posRange();
    QRectF gate2_range = m_gate2->posRange();

    int indGate1 = maxInd(gate1_range, true);
    int indGate2 = maxInd(gate2_range, false);

    qDebug() << "gate1 " << indGate1;
    qDebug() << "gate2 " << indGate2;
    if(indGate1 == -1 || indGate2 == -1)
        return;

    emit calculatedThickness(timeToDepth(qAbs(indGate2 - indGate1) / fs * m_xUnit.second )); // tbc
}

qreal TChartViewForm::maxInd(const QRectF &rect, bool thres)
{
    // if acquisition is not started or the plot is empty;
    if(m_series->count() <= 0)
        return -1;

    // convert to series position
    QPointF value_left = m_chart->mapToValue(rect.topLeft(), m_series);
    QPointF value_right = m_chart->mapToValue(rect.bottomRight(), m_series);
    qreal _left = value_left.x();
    qreal _right = value_right.x();

    qDebug() << "left: "<< _left;
    qDebug() << "right: "<< _right;

    if(_xAxisType == x_AXISTYPE::DEPTH)
    {
        _left = depthToTime(value_left.x());
        _right = depthToTime(value_right.x());
    }

    int leftInd = (_left / xMax) * mTcpClient::DATA_SIZE /2;
    int rightInd = (_right/ xMax) * mTcpClient::DATA_SIZE /2;
    double threshold = qAbs(value_left.y() + value_right.y()) / 2; // absolute thres

    if(thres)
        emit sendThreshold(threshold);

    leftInd < 0 ? leftInd =0 : leftInd;
    rightInd < 0 ? rightInd =0 : rightInd;
    leftInd = leftInd > mTcpClient::DATA_SIZE/2 ? mTcpClient::DATA_SIZE/2 : leftInd;
    rightInd = rightInd > mTcpClient::DATA_SIZE/2 ? mTcpClient::DATA_SIZE/2 : rightInd;

    int _tempMax = leftInd;

    for(int i = leftInd+1; i< rightInd; i++)
    {
        double value = qAbs(m_series->at(i).y()); // absolute value
        if((value > qAbs(m_series->at(_tempMax).y())) && value >threshold )
            _tempMax = i;
    }

    if(m_series->at(_tempMax).y() < threshold)
        return -1;

    return _tempMax;
}

void TChartViewForm::setYUnit(const QString &newYUnit)
{
    m_yUnit.first = newYUnit;
    if(newYUnit == QString("mV"))
        m_yUnit.second = 1000;
    else if(newYUnit == QString("V"))
        m_yUnit.second = 1;
    else if(newYUnit == QString("uV"))
        m_yUnit.second = 1e6;
    else
        m_yUnit.second = 1000;

    if(ui->comboAxis->currentIndex())
        ui->labelUnit->setText(m_yUnit.first);
    else
        ui->labelUnit->setText(m_xUnit.first);
}

void TChartViewForm::resizeEvent(QResizeEvent *event)
{


    auto newPosGate1 = m_chart->mapToPosition(m_gate1->ascanValue(), m_series);
    auto newPosGate2 = m_chart->mapToPosition(m_gate2->ascanValue(), m_series);

    m_gate1->moveBy(newPosGate1.x() - m_gate1->scenePos().x(), newPosGate1.y() - m_gate1->scenePos().y());
    m_gate2->moveBy(newPosGate2.x() - m_gate2->scenePos().x(), newPosGate2.y() - m_gate2->scenePos().y());

    m_gate1->updateAscanValue();
    m_gate2->updateAscanValue();

    // qreal heightRatio =  event->size().toSizeF().height() / event->oldSize().toSizeF().height();
    // qreal widthRatio =  event->size().toSizeF().width() / event->oldSize().toSizeF().width();
    // qDebug() << heightRatio << widthRatio;
    // m_gate1->moveBy( (widthRatio - 1) * m_gate1->scenePos().x(), (heightRatio - 1)* m_gate1->scenePos().y() );
    // m_gate2->moveBy( (widthRatio - 1) * m_gate2->scenePos().x(), (heightRatio - 1)* m_gate2->scenePos().y() );

    QWidget::resizeEvent(event);
}

void TChartViewForm::setXUnit(const QString &newXUnit)
{
    m_xUnit.first = newXUnit;
    if(newXUnit == QString("mm"))
        m_xUnit.second = 1000;
    else if(newXUnit == QString("m"))
        m_xUnit.second = 1;
    else if(newXUnit == QString("um"))
        m_xUnit.second = 1e6;
    else if(newXUnit == QString("nm"))
        m_xUnit.second = 1e9;
    else
        m_xUnit.second = 1000;
    if(ui->comboAxis->currentIndex())
        ui->labelUnit->setText(m_yUnit.first);
    else
        ui->labelUnit->setText(m_xUnit.first);
}

void TChartViewForm::setXRange(const float &min, const float &max)
{
    float _min = 0;
    float _max = (_xAxisType == x_AXISTYPE::TIME) ? xMax : xMax*m_vel/2;

    if(min <=0)
        _min = 0;
    else
        _min = min;

    if(max <= _max)
        _max = max;

    m_X->setRange(_min, _max);
}

void TChartViewForm::setYRange(const float &min, const float &max)
{
    auto vpp = max - min;
    if(!(vpp > yRangeMax || vpp < yRangeMin))
        m_Y->setRange(min, max);

}

void TChartViewForm::updateSteps()
{

}


void TChartViewForm::on_btnIncr_clicked()
{

    float step = ui->steps->value();

    auto axis = m_X; // default to x axis control
    if(ui->comboAxis->currentIndex() == 1) // Y axis
        axis = m_Y;

    //  control logic - if axis is X
    if(axis == m_X)
    {
        setXRange(m_X->min(), m_X->max() + step);

    }else
    {
        qreal ymin = m_Y->min()-step;
        if(ymin < 0.0 && _yAxisType==y_AXISTYPE::RECTIFY)
            ymin = 0.0;
        setYRange(ymin, m_Y->max() + step);
    }

    updateLabelPosition();
}


void TChartViewForm::on_btnDecr_clicked()
{
    float step = ui->steps->value();

    auto axis = m_X; // default to x axis control
    if(ui->comboAxis->currentIndex() == 1) // Y axis
        axis = m_Y;

    //  control logic - if axis is X
    if(axis == m_X)
    {
        if(m_X->max() - step <= (m_X->max() - m_X->min()) * 0.05)
            return;

        setXRange(m_X->min(), m_X->max() - step);

    }else
    {
        if(_yAxisType == y_AXISTYPE::FULL)
            setYRange(m_Y->min()+step, m_Y->max() - step);
        else
            setYRange(m_Y->min(), m_Y->max() - step);
    }

    updateLabelPosition();
}





void TChartViewForm::on_comboAxis_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0: // X selected
        ui->labelUnit->setText(m_xUnit.first);
        ui->steps->setMaximum((_xAxisType == x_AXISTYPE::TIME) ? xMax : xMax*m_vel/2);
        if(m_X)
            ui->steps->setValue((m_X->max() - m_X->min()) * 0.05);
        break;
    case 1: // Y seleceted
        ui->labelUnit->setText(m_yUnit.first);
        ui->steps->setMaximum(2000);
        if(m_Y)
            ui->steps->setValue((m_Y->max() - m_Y->min()) * 0.05);
        break;
    default:
        break;
    }
}

