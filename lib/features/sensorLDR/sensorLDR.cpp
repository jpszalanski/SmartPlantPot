#include "sensorLDR.h"
#include "config.h"
#include <Arduino.h>

SensorLDR::SensorLDR(uint8_t ldrPin)
    : ldrPin(ldrPin) {}

void SensorLDR::begin()
{

    pinMode(ldrPin, INPUT);
}

void SensorLDR::readSensor()
{

    lightLevel = 4095 - analogRead(ldrPin);
}

void SensorLDR::printSensorReadings()
{

    Serial.print("Light Level: ");
    Serial.println(lightLevel);
}

int SensorLDR::getLightLevel() const { return lightLevel; }
