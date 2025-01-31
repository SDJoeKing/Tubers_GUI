#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include "DspFilters/Dsp.h"
#include <QElapsedTimer>
#include <QEventLoop>
#include "mtcpclient.h"

#define FRAMERATE_CONTROL
#ifdef FRAMERATE_CONTROL
#define FRAMERATE 101
#endif


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
    Dsp::Filter *m_filter = new Dsp::FilterDesign<Dsp::Butterworth::Design::BandPass<50>, 1>;
    // Dsp::SimpleFilter<Dsp::Butterworth::BandPass<10>, 1> *m_filter = new Dsp::SimpleFilter<Dsp::Butterworth::BandPass<10>, 1>;
    float m_vel;
    float m_fs;
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
    void updateFilter(const quint8 & order, const float &fs, const float &fc, const float &fw);
};

#endif // PROCESSOR_H
