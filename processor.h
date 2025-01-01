#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include "DspFilters/Dsp.h"
#include <QElapsedTimer>
#include <QEventLoop>
#include "mtcpclient.h"

class Processor : public QObject
{
    Q_OBJECT
public:
    explicit Processor(QObject *parent = nullptr);
    ~Processor();
    void setParam(const float &vel, const bool &depth, const bool &rect, const bool &filt);
    void run();
    void close();

private:
    Dsp::SimpleFilter<Dsp::Butterworth::BandPass <4> , 1> m_filter;
    float m_vel;
    bool depthAxis;
    bool rectify;
    bool filtering;
    QEventLoop *m_loop;

signals:
    void dataProcessed(const QList<QPointF> &, bool);
    void dataLogger(const char *);
    void sendTemperature(const float);
public slots:
    void process(const char *);
    void setVel(const float &vel);
    void setDepth(const bool &depth);
    void setRectified(const bool &rect);
    void setFiltering(const bool &filt);
    void updateFilter(const quint8 & order, const quint8 &fs, const float &fc, const float &fw);
};

#endif // PROCESSOR_H
