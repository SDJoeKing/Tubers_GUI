#ifndef CONFIG_H
#define CONFIG_H

#include "qtypes.h"

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
#endif // CONFIG_H

