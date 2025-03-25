#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include "DspFilters/Dsp.h"
#include <QElapsedTimer>
#include <QEventLoop>
#include "mtcpclient.h"
#include <cmath>

#ifdef FRAMERATE_CONTROL
#define FRAMERATE 120
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
    enum HEADER {
        acqMode = 4,
        encoderDirection,
        golayCode,
        systemTempLow,
        systemTempHigh,
        linkSpeed
    };

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
    float m_golayData[mTcpClient::DATA_SIZE/2]{0};
    bool m_golayASeq = true;
    bool m_golayReady = false;
    float m_maxValue = 0;
    const int firstPeakIndex = 200;
    float m_scale = 1.0;
signals:
    void dataProcessed(const QList<QPointF> &, bool );
    void dataLogger(const char *);
    void sendTemperatureNLinkSpeed(const float &, const float &); // temp. speed
    void plotRate(const float &);
public slots:
    void process(const char *);
    void setVel(const float &vel);
    void setDepth(const bool &depth);
    void setRectified(const bool &rect);
    void setFiltering(const bool &filt);
    void updateFilter(const quint8 & order, const float &fs, const float &fc, const float &fw);
    void updateGolaySetting(bool useGolay, const QString &seq, const float &freq, quint8);
    void updateScale(const float &);
};

#endif // PROCESSOR_H
