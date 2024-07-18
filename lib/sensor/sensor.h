#ifndef SENSOR_H
#define SENSOR_H

#include <DHT.h>
#include "config.h"

class Sensor
{
public:
    Sensor(uint8_t dhtPin, uint8_t dhtType, uint8_t ldrPin, uint8_t soilMoisturePin, uint8_t pumpPin);
    void begin();
    void readSensors();
    void printReadings();
    float getTemperature() const;
    float getHumidity() const;
    int getLightLevel() const;
    int getSoilMoisture() const;
    int getPercentageSoilMoisture() const;

private:
    DHT dht;
    uint8_t ldrPin;
    uint8_t soilMoisturePin;
    uint8_t pumpPin;
    float temperature;
    float humidity;
    int lightLevel;
    int soilMoisture;
    int percentageSoilMoisture;
};

#endif
