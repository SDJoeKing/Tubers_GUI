#include "processor.h"
#include "mainwindow.h"


const float vel_water = 1480; // m/s
float pulseWidth = 4.0; // mm to skip from one pulse peak to search for another
uint16_t postLockSample = 3;
float ID = 125; // inner diameter mm
float vel_material = 2292.7; // m/s							//INPUT BY CAN BUS USER
const float _thres = 0.05;  										//INPUT BY CAN BUS USER
float targetThick = 5; // mm
AscanFeatures features {0.0};
constexpr auto NUM_OF_ADC_SAMPLES = DATA_SIZE/2;

/////// FUNCTION DECLARATION //////////
float thickCal(const float *arr,  float velocity, float fs, float id, float targetThick, float _thres);
void getAscanFeatures(const float *arr, AscanFeatures* features, float fs, float id );
uint16_t argFirstLarger(const float *arr, uint16_t start, uint16_t end, float threshold);
uint16_t argFirstSmaller(const float *arr, uint16_t start, uint16_t end, float threshold);
uint16_t argmax(const float* arr, uint16_t startPoint, uint16_t endPoint);
///////////////////////////////////////////////////////////////////////////////


QElapsedTimer processTimer;

QMutex mu;
int golayTrack = 0;
QList<QPair<int, int>> _holder;
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

static void dataConversion(quint32 & dest,const QByteArray& src, int ind1, int ind2, int ind3, int ind4)
{
    dest = static_cast<quint32>(((src.at(ind4) << 24) & 0xFF000000 ) | ( (src.at(ind3) << 16)  & 0x00FF0000 ) |
                                ((src.at(ind2) << 8)  & 0x0000FF00) | ((src.at(ind1)  )  & 0xFF));
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

    // features reading;
    qint16 _max;
    dataConversion<decltype(_max)>(_max, serverData, HEADER::featureMax_l, HEADER::featureMax_h);
    quint32 _noise;

    dataConversion(_noise, serverData, HEADER::featureNoise_1, HEADER::featureNoise_2, HEADER::featureNoise_3, HEADER::featureNoise_4);
    float noise = *(reinterpret_cast<float *>(&_noise));
    qDebug() << "max " << _max;
    emit featuresReading(_max, noise);

    quint32 _thick;

    dataConversion(_thick, serverData, HEADER::thick1, HEADER::thick2, HEADER::thick3, HEADER::thick4);

    float _thickness = *(reinterpret_cast<float *>(&_thick));
    qDebug() << "Extracted thick " << _thickness;


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

    // emit dataLogger(reinterpret_cast<const char *>(&_temp));

    // getAscanFeatures(_temp, &features, m_fs*1e6, ID);
    // float _thickness = thickCal(_temp,vel_material, m_fs*1e6, ID, targetThick, _thres);
    emit thickness(_thickness);



    // empty m_golayData
    for(auto &i : m_golayData)
    {
        i = 0;
    }

    emit dataProcessed(calPoint);



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


//////////////////////////////////////////////////////////////////////////////More actions
uint16_t argmax(const float *arr, uint16_t startPoint, uint16_t endPoint)
{
    uint16_t tempMax = startPoint;
    for(size_t i = startPoint+1; i< endPoint; i++)
    {
        if(*(arr+i) >= *(arr+tempMax))
            tempMax = i;
    }

    return tempMax;
}


//////////////////////////////////////////////////////////////////////////////
uint16_t argFirstLarger(const float *arr, uint16_t start, uint16_t end, float threshold){

    uint16_t maxInd = start;

    for(size_t i=start; i<end; i++){
        if((*(arr+i)) >= threshold)
        {
            maxInd = i;
            for(size_t j = i+1; j < i+postLockSample; j++)
            {

                if(j >= end)
                    return 0;

                if( (*(arr+j)) < threshold)
                {
                    maxInd = j;
                    return argFirstLarger(arr, maxInd, end, threshold);
                }
            }
            return maxInd;
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
uint16_t argFirstSmaller(const float *arr, uint16_t start, uint16_t end, float threshold){


    uint16_t minInd = start;

    for(size_t i=start; i<end; i++){
        if( (*(arr+i)) <= threshold)
        {
            minInd = i;
            for(size_t j = i+1; j < i+postLockSample; j++)
            {

                if(j >= end)
                    return 0;

                if( (*(arr+j)) > threshold)
                {
                    minInd = j;
                    return argFirstSmaller(arr, minInd, end, threshold);
                }
            }
            return minInd;
        }

    }
    return 0;

}

//@brief C function to output parameters of input Asan
//@param arr Ptr to the float array containing ABSOLUTE Ascan data; features AscanFeatures struct containing calculated Ascan features

void getAscanFeatures(const float* arr, AscanFeatures* features, float fs, float id )
{


    uint16_t mirrorGap = 18.0 / 1000 / vel_water * fs; // 9 mm mirror distance
    uint16_t waterGap = (id - pulseWidth)/1000 / vel_water * fs;



    if(waterGap >= NUM_OF_ADC_SAMPLES)
        waterGap = NUM_OF_ADC_SAMPLES - 1;


    float _tempMax = 0;
    float _tempMin = 1000;
    float _tempSum = 0;
    float _tempRMS = 0;
    float _tempStd = 0;
    float _noiseAvg = 0;


    for(size_t i = waterGap; i< NUM_OF_ADC_SAMPLES; i++)
    {
        float _tempV = arr[i];

        if(_tempMax <= _tempV)
            _tempMax = _tempV;
        if(_tempMin >= _tempV)
            _tempMin = _tempV;

        _tempRMS+=arr[i]*arr[i];
        _tempSum+=_tempV;

    }

    features->max = _tempMax;
    features->min = _tempMin;
    features->rms = sqrtf(_tempRMS / (NUM_OF_ADC_SAMPLES - waterGap));
    features->mean = _tempSum / (NUM_OF_ADC_SAMPLES - waterGap);

    features->noiseV = features->mean;

    if(waterGap > mirrorGap)
    {
        features->noiseV = 0;
        for(size_t i = mirrorGap; i< waterGap; i++)
        {
            //    	    	uint16_t _tempV = arr[i];
            //
            //    	        if(features->noiseV <= _tempV)
            //    	            features->noiseV =  _tempV;

            _noiseAvg += arr[i];

        }
        features->noiseV = _noiseAvg / (waterGap - mirrorGap);
    }

    for(size_t i=waterGap; i< NUM_OF_ADC_SAMPLES; i++)
    {
        _tempStd += powf(arr[i] - features->mean, 2);
    }

    features->std = sqrtf(_tempStd / (NUM_OF_ADC_SAMPLES - waterGap - 1));

}


// @brief: Function calculating the thickness based on inputs of velocity, sampling rate, pipe inner diameter, norminal thickness and threshold to try in (0-1]
float thickCal(const float *arr,  float velocity, float fs, float id, float targetThick, float _thres)
{


    // 0. preparation calculation
    uint16_t waterGap = (id - pulseWidth * 2)/1000 / vel_water * fs;
    uint16_t targetThickDataPoint = targetThick/1000 * 2 / velocity * fs;

    // ! 5 times noise level to find first peak
    //    float threshold = 5.0 * features.noiseV;
    float threshold = 10.0 * features.noiseV;


    // 1. front wall with waterGap data points margin in the beginning
    // uint16_t _front = argFirstLarger(arr, waterGap, dataLength, threshold);

    uint16_t gateA_f = argFirstLarger(arr, waterGap, NUM_OF_ADC_SAMPLES, threshold);
    //    gateInfo.gateA_f = gateA_f;

    if(gateA_f == 0)
        return -1;

    uint16_t gateA_b = argFirstSmaller(arr, gateA_f, gateA_f + targetThickDataPoint, threshold);
    //    gateInfo.gateA_b = gateA_b;

    if(gateA_b == 0)
        return -2;

    uint16_t gateA = argmax(arr, gateA_f, gateA_b);

    threshold = _thres * arr[gateA];

    // 2. find backwall
    uint16_t margin = gateA_b + (pulseWidth*2/1000 / velocity * fs); // margin of pulse, i.e. minimum resolution of scan

    uint16_t gateB_f = argFirstLarger(arr, margin, margin + targetThickDataPoint, threshold);
    //    gateInfo.gateB_f = gateB_f;

    if(gateB_f == 0)
        return -3;

    uint16_t gateB_b = argFirstSmaller(arr,gateB_f, margin + targetThickDataPoint, threshold);
    //    gateInfo.gateB_b = gateB_b;
    if(gateB_b == 0)
        return -4;

    uint16_t gateB = argmax(arr, gateB_f, gateB_b);

    // 3. return calculated thickness

    return (gateB - gateA) / 2.0 / fs * velocity *1000;
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
