#ifndef SENSORLDR_H
#define SENSORLDR_H

#include <Adafruit_Sensor.h>
#include "config.h"
#include <Arduino.h>

class SensorLDR
{
public:
    SensorLDR(uint8_t ldrPin);

    void begin();
    void readSensor();
    void printSensorReadings();
    int getLightLevel() const;

private:
    uint8_t ldrPin;
    int lightLevel;
};

#endif
