#include "processor.h"
#include "mainwindow.h"

QElapsedTimer processTimer;
QElapsedTimer fpsTimer;
QMutex mu;
int counter = 0;
int golayTrack = 0;

float lerp(int a, int b, float t);
void correlate(const float * dataArr, quint16 len, std::shared_ptr<std::vector<float>> SEQ, float *);
std::shared_ptr<std::vector<float>> genPulse(std::unique_ptr<std::vector<int>>, quint16 );


Processor::Processor(QObject *parent)
    : QObject{parent}
{
    m_vel = 5890.0;
    depthAxis = false;
    rectify = false;
    filtering = false;

    processTimer.start();
    fpsTimer.start();

    m_sequenceA = genPulse(std::make_unique<std::vector<int>>(std::vector<int>{1,1,-1,1,1,1,-1,1,1,1,-1,1,1,1,-1,1}), 10);

    qDebug() << "Processor on ";
}

Processor::~Processor()
{
    qDebug() << "Processor shutdown";
}

void Processor::setParam(const float &vel, const bool &depth, const bool &rect, const bool &filt)
{
    m_vel = vel;
    depthAxis = depth;
    rectify = rect;
    filtering = filt;
}

void Processor::updateFilter(const quint8 &order, const float &fs, const float &fc, const float &fw)
{

    Dsp::Params params;

    params[0] = fs;
    params[1] = order;
    params[2] = fc;
    params[3] = fw;

    m_filter->setParams(params);
    // m_filter->setup(order, fs, fc, fw);
    m_fs = fs;
}

void Processor::updateGolaySetting(bool useGolay, const QString &seq, const float &freq, quint8 len)
{
    // ensure when restarting acq, golaysettings are reset

    m_golayReady = false;
    m_golayASeq = true;

    m_golay = useGolay;

    if(m_golay)
    {
        std::vector<int> _tempVecA;
        std::vector<int> _tempVecB;

        QString _tempA = seq.sliced(0, len/2);
        QString _tempB = seq.sliced(len/2);

        quint8 counter = _tempA.size() - 1;
        while(_tempA.at(counter).toUpper() == 'C' && counter != 0)
        {
            counter -= 1;
        }

        for(size_t i = 0; i<=counter; i++)
        {
            _tempVecA.emplace_back( (_tempA.at(i).toUpper() == 'P') ? 1 : ((_tempA.at(i).toUpper() == 'N') ? -1 : 0));
            _tempVecB.emplace_back( (_tempB.at(i).toUpper() == 'P') ? 1 : ((_tempB.at(i).toUpper() == 'N') ? -1 : 0));
        }

        m_sequenceA = genPulse(std::make_unique<std::vector<int>>(_tempVecA)  , (int)(100/freq/2));
        m_sequenceB = genPulse(std::make_unique<std::vector<int>>(_tempVecB) , (int)(100/freq/2));
    }

}

void Processor::updateScale(const float & newScale)
{
    m_scale = newScale;
}


void Processor::run()
{
    m_loop = new QEventLoop(this);

    m_loop->exec();
    deleteLater();
}

void Processor::close()
{
    m_loop->quit();
}

void Processor::process(const char *dataptr)
{

    auto serverData = QByteArray::fromRawData(dataptr,  DATA_SIZE_RECV );

    double xpoint=0;
    QList<QPointF> calPoint( DATA_SIZE/2);
    quint16 temp1;
    quint16 temp2;
    float *dataPoint[1];
    float _temp[ DATA_SIZE/2]{0};


    dataPoint[0] = _temp;

    // QByteArray _arr(mTcpClient::DATA_SIZE/2, Qt::Uninitialized);

    // take into account of header data
    int j=  HEADER_SIZE;
    // get system information from the header data
    quint8 _forward = static_cast<quint8>(serverData.at(HEADER::encoderDirection));
    qDebug() << _forward;
    temp1 =(serverData.at(HEADER::systemTempHigh) << 8) & 0xFF00;

    temp2 = (serverData.at(HEADER::systemTempLow)) & 0xFF;

    float _temperature =  (static_cast<quint16>(temp2 | temp1));
    int linkSpeed = (static_cast<quint8>(serverData.at(HEADER::linkSpeed)) * 10);
    // bool golaySeq = static_cast<quint8>(serverData.at(HEADER::golayCode));
    bool golaySeq = m_golayASeq;

    // qDebug() << "------- " << _temperature / 2 / 125e6 *2293.0 << " ------- ";
    _temperature = ((_temperature/65536.0f)/0.00198421639f ) - 273.15f;

    for (int i = 0; i <  DATA_SIZE/2; i++)
    {
        temp1 =(serverData.at(j + 1) << 8) & 0xFF00;
        temp2 = (serverData.at(j)) & 0xFF;
        dataPoint[0][i] = static_cast<qint16>(temp2 | temp1)/ 32768.0  * 3.18 * 1.0 * 1000.0;

        j += 2;
    }

    if(m_golay)
    {
        QMutexLocker lk(&mu);
        if(golaySeq)
        {

            correlate(dataPoint[0], DATA_SIZE/2,  m_sequenceA, m_golayData);
            m_golayASeq = false;

        }else
        {
            correlate(dataPoint[0],  DATA_SIZE/2,  m_sequenceB, m_golayData);
            m_golayASeq = true;
            for(auto &i : m_golayData)
            {
                i/=2;
            }
            m_golayReady = true;

        }
    }

    if(m_golay && !m_golayReady) // if using golay and the sequence B has not been received, simply skip plotting
        return;
    else if(m_golay && m_golayReady)
        dataPoint[0] = m_golayData;

    m_golayReady = false;
    if(filtering)
    {
        m_filter->process( DATA_SIZE/2, dataPoint);
    }

    // rectified, envelope, depth?

    for (int i = 0; i < DATA_SIZE/2; i++)
    {

        if(rectify)
            dataPoint[0][i] = MainWindow::envelope(dataPoint[0][i], MainWindow::_env, MainWindow::m_ga, MainWindow::m_gr);
    }

    // normalise data to (0, 1]
    for(int i = 0; i < DATA_SIZE/2; i++)
    {

        if(depthAxis)
            xpoint = i / m_fs /2/ 1e6 * m_vel * 1000;
        else
            xpoint = i / m_fs /1e6 *1000;

        dataPoint[0][i] /= (m_maxValue / 100 / m_scale);
        calPoint[i] = QPointF(xpoint, dataPoint[0][i]);
        _temp[i] = dataPoint[0][i];
    }

    //reset envelope;
    MainWindow::_env = 0;

    emit dataLogger(reinterpret_cast<const char *>(&_temp));

    // empty m_golayData
    for(auto &i : m_golayData)
    {
        i = 0;
    }

    if(fpsTimer.hasExpired(1000))
    {
        emit plotRate(counter);
        counter=0;
        fpsTimer.restart();

    }

#ifdef FRAMERATE_CONTROL
    if(processTimer.durationElapsed().count() > (1e9 / FRAMERATE) )
    {
        emit dataProcessed(calPoint, _forward == 2 ? false : true );
        processTimer.restart();
        counter++;
    }

#else
    emit dataProcessed(calPoint, _forward == 2 ? false : true);
    counter++;
#endif
    emit sendTemperatureNLinkSpeed(_temperature, linkSpeed);
}

void Processor::setVel(const float &vel)
{
    m_vel = vel;
}

void Processor::setDepth(const bool &depth)
{
    depthAxis = depth;

}

void Processor::setRectified(const bool &rect)
{
    rectify = rect;
}

void Processor::setFiltering(const bool &filt)
{
    filtering = filt;
}

std::shared_ptr<std::vector<float>> genPulse(std::unique_ptr<std::vector<int>> SEQ, quint16 len)
{
    std::unique_ptr<std::vector<float>> output = std::make_unique<std::vector<float>>();
    SEQ->emplace_back(0);
    float step = 1.0/len;
    for(size_t j = 0; j < SEQ->size() - 1; j++)
    {
        for(size_t i = 0; i < len; i++)
        {
            output->emplace_back(lerp(SEQ->at(j), SEQ->at(j+1), i*step));
        }
    }

    return output;
}

void correlate(const float * dataArr, quint16 len, std::shared_ptr<std::vector<float>> SEQ, float *output)
{


    if(SEQ->size() > len)
        return ;

    quint16 sum_counter = 1;

    for(size_t i=0; i<len; i++)
    {
        float _tempV = 0;
        for(size_t j = 0; j<SEQ->size(); j++)
        {
            if(i+j >= len)
                break;
            _tempV += dataArr[i+j] * SEQ->at(j);
            sum_counter++;
        }
        output[i] += _tempV / sum_counter;
        sum_counter = 1;
    }

}

float lerp(int a, int b, float t)
{
    return a + t * (b - a);
}
