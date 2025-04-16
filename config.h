#ifndef CONFIG_H
#define CONFIG_H

#include "qtypes.h"
#include <QString>
#include <QMap>
static const quint16 DATA_SIZE = 16000*2 ;
static const quint8 HEADER_SIZE = 24;
static const quint16 DATA_SIZE_RECV = DATA_SIZE + HEADER_SIZE;

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
    maxLow, maxHigh,
    minLow, minHigh,
    meanLow, meanHigh,
    rmsLow, rmsHigh,
    stdLow, stdHigh,
    thick1, thick2, thick3, thick4
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

    pulseWidth,
    postLock,
    ID,
    velocity,
    threshold,
    ratedThickness,

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

static const QString getImuLabel(const QString &axis)  {return QString("IMU - %1: ").arg(axis);}

#endif // CONFIG_H

