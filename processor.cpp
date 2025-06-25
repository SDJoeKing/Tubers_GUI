#include "processor.h"
#include "mainwindow.h"

QElapsedTimer processTimer;

QMutex mu;
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

    tfm_required = false;
    m_sequenceA = genPulse(std::make_unique<std::vector<int>>(std::vector<int>{1,1,-1,1,1,1,-1,1,1,1,-1,1,1,1,-1,1}), 10);
    // calPoint.reserve( DATA_SIZE/2);

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

void Processor::updateTfmSetting(quint8 channels, quint16 rows, quint16 cols, quint16 samples, float pitch, float offsetX, float offsetY, float resolution, bool required)
{
    tfm_required = required;
    m_chan = channels;


    // initialise LookTable
    lookUpTable.clear();
    fmc_data.clear();

    for(int i=0; i<m_chan+1; i++)
    {
        lookUpTable.emplace_back(ArrayXXf::Zero(rows, cols));
        fmc_data.emplace_back(ArrayXXf::Zero(m_chan+1, samples));
    }

    m_chan = TrueSeq(m_chan);
    // calculate the lookTable
    MATH::generateLookTable(lookUpTable, m_vel, pitch, offsetX, offsetY, resolution);
}

void Processor::populateFMC(float * data, quint8 tx, quint8 rx)
{
    // test and optimise THIS !!!

    fmc_data[tx].row(rx) = Map<VectorXf>(data, fmc_data[tx].cols()).transpose().segment(0, fmc_data[tx].cols());

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

template <class T>
static void dataConversion(T& dest,const QByteArray& src, int lowIndex, int highIndex)
{
    dest = static_cast<T>(((src.at(highIndex) << 8) & 0xFF00 ) | ( (src.at(lowIndex)  & 0xFF )));
}

void Processor::process(const char *dataptr, bool headerOnly)
{

    auto serverData = QByteArray();
    serverData.reserve(DATA_SIZE_RECV);

    if(!headerOnly)
        serverData = QByteArray::fromRawData(dataptr,  DATA_SIZE_RECV);
    else
        serverData = QByteArray::fromRawData(dataptr, HEADER_SIZE);



    double xpoint=0;
    quint16 temp1;
    quint16 temp2;
    float *dataPoint[1];
    float _temp[ DATA_SIZE/2]{0};
    // normalise data to (0, 1]

    // serverData
    QList<QPointF>calPoint /*= QList<QPointF>(DATA_SIZE/2, QPointF(1.0f, 1.0f))*/;

    dataPoint[0] = _temp;

    // take into account of header data
    int j=  HEADER_SIZE;

    // get current tx rx
    quint8 tx = static_cast<quint8>(serverData.at(HEADER::TxRx) >> 4 & 0x0F);
    quint8 rx = static_cast<quint8>(serverData.at(HEADER::TxRx) & 0x0F);

    // get system information from the header data
    quint8 _forward = static_cast<quint8>(serverData.at(HEADER::encoderDirection));

    int linkSpeed = (static_cast<quint8>(serverData.at(HEADER::linkSpeed)) * 10);

    if(headerOnly)
    {
        qDebug() << serverData;
        qDebug() << "linkspeed " << serverData.at(HEADER::linkSpeed);
    }
    m_errorCode = static_cast<quint8>(serverData.at(HEADER::errorFlags));

    bool golaySeq = m_golayASeq;

    dataConversion<decltype(temp1)>(temp1, serverData, HEADER::systemTempLow, HEADER::systemTempHigh);
    float _temperature = ((temp1/65536.0f)/0.00198421639f ) - 273.15f;

    // IMU reading;
    qint16 _imu_x, _imu_y, _imu_z;
    dataConversion<decltype(_imu_x)>(_imu_x, serverData, HEADER::imuXLow, HEADER::imuXHigh);
    dataConversion<decltype(_imu_x)>(_imu_y, serverData, HEADER::imuYLow, HEADER::imuYHigh);
    dataConversion<decltype(_imu_x)>(_imu_z, serverData, HEADER::imuZLow, HEADER::imuZHigh);

    if(headerOnly)
    {
        emit sendHeaderInfo(_temperature, linkSpeed, m_errorCode,  QVector<qint16>{_imu_x, _imu_y, _imu_z});
        return;
    }

    for (int i = 0; i <  DATA_SIZE/2; i++)
    {
        temp1 =(serverData.at(j + 1) << 8) & 0xFF00;
        temp2 = (serverData.at(j)) & 0xFF;
        dataPoint[0][i] = static_cast<qint16>(temp2 | temp1)/ 32768.0  * 3.18 * 1.0 * 1000.0;
        // dataPoint[0][i] = static_cast<qint16>(temp2 | temp1);
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



    for(int i = 0; i < DATA_SIZE/2; i++)
    {

        if(depthAxis)
            xpoint = i / m_fs /2/ 1e6 * m_vel * 1000;
        else
            xpoint = i / m_fs /1e6 *1000;

        dataPoint[0][i] /= (m_maxValue / 100 / m_scale);
        calPoint.emplace_back(xpoint, dataPoint[0][i]);
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

    emit dataProcessed(calPoint);

    // populate FMC array

    if(tfm_required)
    {
        populateFMC(_temp, tx, rx);

        if(tx == m_chan && rx== m_chan)
        {   qDebug() << m_chan;

            ArrayXXf tfm_result = ArrayXXf::Zero(lookUpTable[0].rows(), lookUpTable[0].cols());
            MATH::TFM(fmc_data, tfm_result, lookUpTable, m_fs*1e6);

            emit tfmReady(tfm_result);
        }

        // if(tx > m_chan || rx > m_chan)
        //     throw std::runtime_error("Wrong tx/rx channels out of range");
    }

    if(processTimer.elapsed() > 500)
    {
        processTimer.restart();
        emit sendHeaderInfo(_temperature, linkSpeed, m_errorCode,  QVector<qint16>{_imu_x, _imu_y, _imu_z});
    }

    if(!headerOnly)
        emit requestAcq();
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
