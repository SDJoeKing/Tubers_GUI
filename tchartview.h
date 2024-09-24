#ifndef TCHARTVIEW_H
#define TCHARTVIEW_H

#include <QChartView>
#include <QObject>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QToolButton>

#include "mtcpclient.h"
#include "tgate.h"

class TChartView : public QChartView
{
    Q_OBJECT
public:
    explicit TChartView(QWidget *parent = nullptr);
    void rubberBandOn(bool);
public slots:
signals:
    void selectedRubberBand(QRectF);
private:
    bool _move=false;
    QPointF _start;
    QPointF _end;

protected:


    // QWidget interface
protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

    // QWidget interface
protected:
    virtual void wheelEvent(QWheelEvent *event) override;
};

#endif // TCHARTVIEW_H
