#ifndef CONFIG_H
#define CONFIG_H

#include "qtypes.h"
#include <QString>
#include <QMap>
static const quint16 DATA_SIZE = 16000*2 ;
static const quint8 HEADER_SIZE = 10;
static const quint16 DATA_SIZE_RECV = DATA_SIZE + HEADER_SIZE;

enum HEADER {
    acqMode = 4,
    encoderDirection,
    errorFlags,
    systemTempLow,
    systemTempHigh,
    linkSpeed
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

#endif // CONFIG_H

