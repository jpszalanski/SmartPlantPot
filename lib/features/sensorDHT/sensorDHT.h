#ifndef SENSORDHT_H
#define SENSORDHT_H

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "config.h"
#include <Arduino.h>

class SensorDHT
{
public:
    SensorDHT(uint8_t dhtPin, uint8_t dhtType);
    void begin();
    void readSensor();
    void printSensorReadings();
    float getTemperature() const;
    float getHumidity() const;

private:
    DHT dht;
    float temperature;
    float humidity;
};

#endif
