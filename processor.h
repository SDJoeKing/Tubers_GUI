#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include "DspFilters/Dsp.h"
#include <QElapsedTimer>
#include <QEventLoop>
#include "mtcpclient.h"
#include <cmath>
#include "tmath.h"

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
    float m_vel;
    float m_fs;
    bool depthAxis;
    bool rectify;
    bool filtering;
    QEventLoop *m_loop;
    std::shared_ptr<std::vector<float>> m_sequenceA;
    std::shared_ptr<std::vector<float>> m_sequenceB;
    bool m_golay;
    float m_golayData[DATA_SIZE/2]{0};
    bool m_golayASeq = true;
    bool m_golayReady = false;
    float m_scale = 1.0;
    const float m_maxValue = 3180;
    quint8 m_errorCode;



signals:
    void dataProcessed(const QList<QPointF> &);
    void dataLogger(const char *);
    void sendHeaderInfo(const float &, const float &, const quint8 &, const QVector<qint16> &imus); // temp. speed
    void requestAcq();
    void thickness(const float &);
    void featuresReading(qint16, float);
public slots:
    void process(const char *, bool headerOnly = false);
    void setVel(const float &vel);
    void setDepth(const bool &depth);
    void setRectified(const bool &rect);
    void setFiltering(const bool &filt);
    void updateFilter(const quint8 & order, const float &fs, const float &fc, const float &fw);
    void updateGolaySetting(bool useGolay, const QString &seq, const float &freq, quint8);
    void updateScale(const float &);


};

#endif // PROCESSOR_H
