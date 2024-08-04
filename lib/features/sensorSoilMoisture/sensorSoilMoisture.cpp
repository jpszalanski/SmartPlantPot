#include "sensorSoilMoisture.h"
#include "config.h"
#include <Arduino.h>

SensorSoilMoisture::SensorSoilMoisture(uint8_t soilMoisturePin)
    : soilMoisturePin(soilMoisturePin) {}

void SensorSoilMoisture::begin()
{

    pinMode(soilMoisturePin, INPUT);
}

void SensorSoilMoisture::readSensor()
{

    soilMoisture = analogRead(soilMoisturePin);
    percentageSoilMoisture = map(soilMoisture, WET, DRY, 100, 0);
}

void SensorSoilMoisture::printSensorReadings()
{
    Serial.print("Soil Moisture: ");
    Serial.println(soilMoisture);

    Serial.print("Percentage Soil Moisture: ");
    Serial.print(percentageSoilMoisture);
    Serial.println(" %");
}

int SensorSoilMoisture::getSoilMoisture() const { return soilMoisture; }
int SensorSoilMoisture::getPercentageSoilMoisture() const { return percentageSoilMoisture; }
