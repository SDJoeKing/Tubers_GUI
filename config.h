#ifndef CONFIG_H
#define CONFIG_H

#include "qtypes.h"
#include <QString>
#include <QMap>
static const quint16 DATA_SIZE = 10000*2 ;
static const quint8 HEADER_SIZE = 17;
static const quint16 DATA_SIZE_RECV = DATA_SIZE + HEADER_SIZE;

static const QByteArray ack = QString("data acknowledged").toUtf8();
static const QByteArray stopAcq = QString("stop").toUtf8();

#ifdef FRAMERATE_CONTROL
#define FRAMERATE 120
#endif

enum HEADER {
    acqMode = 4,
    encoderDirection,
    errorFlags,
    systemTempLow,
    systemTempHigh,
    linkSpeed,
    TxRx,
    imuXLow,
    imuXHigh,
    imuYLow,
    imuYHigh,
    imuZLow,
    imuZHigh
};
enum GOLAY
{
    TWO_BIT, FOUR_BIT, EIGHT_BIT, TEN_BIT
};
enum SETTINGS
{
    txChannel = 0,
    rxChannel,
    pulseDelayNs,
    pulseSequence,
    pulseFreq,
    prf,
    pulsingPower,
    gain,
    requestedAverages,
    encoderTriggering,
    encoderSkips,
    motorSpeed,
    motorAngle,
    adcClkDiv,
    velocity,
    order,
    lowCut,
    highCut,
    golay
};
static const QString LED_NETCONNECTED_STYLE {"QRadioButton::indicator {"
                               "width:                  10px;"
                               "height:                 10px;"
                               "        border-radius:          7px;"
                               "}"
                               "QRadioButton::indicator:checked {"
                               "background-color:       green;"
                               "border:                 2px solid white;"
                               "}"
                               "QRadioButton::indicator:unchecked {"
                               "background-color:       red;"
                               "border:                 2px solid white;"
                               "}"};


static const QString LED_CONNECTED_STYLE {"QRadioButton::indicator {"
                            "width:                  10px;"
                            "height:                 10px;"
                            "        border-radius:          7px;"
                            "}"
                            "QRadioButton::indicator:checked {"
                            "background-color:       red;"
                            "border:                 2px solid white;"
                            "}"
                            "QRadioButton::indicator:unchecked {"
                            "background-color:       green;"
                            "border:                 2px solid white;"
                            "}"};

static const QString LED_NONCONNECT_STYLE {"QRadioButton::indicator {"
                             "width:                  10px;"
                             "height:                 10px;"
                             "        border-radius:          7px;"
                             "}"
                             "QRadioButton::indicator:checked {"
                             "background-color:       red;"
                             "border:                 2px solid white;"
                             "}"
                             "QRadioButton::indicator:unchecked {"
                             "background-color:       grey;"
                             "border:                 2px solid white;"
                             "}"};
template <class T>
static const QString getImuLabel(const QString &axis, const T &v)  {return QString("IMU - %1: %2 milli-G").arg(axis).arg(v);}

#endif // CONFIG_H

