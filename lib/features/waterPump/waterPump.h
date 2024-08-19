#ifndef WATERPUMP_H
#define WATERPUMP_H

#include "config.h"
#include <Arduino.h>

class WaterPump
{
public:
    WaterPump(uint8_t pumpPin);
    void begin();
    bool getWaterPumpStatus() const;
    void setWaterPumpON();
    void setWaterPumpOFF();

private:
    uint8_t pumpPin;
    bool waterPumpStatus;
};

#endif