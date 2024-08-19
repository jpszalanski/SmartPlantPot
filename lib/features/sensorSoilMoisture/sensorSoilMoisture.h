#ifndef SENSORSOILMOISTURE_H
#define SENSORSOILMOISTURE_H

#include "config.h"
#include <Arduino.h>

class SensorSoilMoisture
{
public:
    SensorSoilMoisture(uint8_t soilMoisturePin);
    void begin();
    void readSensor();
    void printSensorReadings();
    int getSoilMoisture() const;
    int getPercentageSoilMoisture() const;

private:
    uint8_t soilMoisturePin;
    int soilMoisture;
    int percentageSoilMoisture;
};

#endif
