#include "sensor.h"
#include "config.h"
#include <Arduino.h>

Sensor::Sensor(uint8_t dhtPin, uint8_t dhtType, uint8_t ldrPin, uint8_t soilMoisturePin, uint8_t pumpPin)
    : dht(dhtPin, dhtType), ldrPin(ldrPin), soilMoisturePin(soilMoisturePin), pumpPin(pumpPin) {}

void Sensor::begin()
{
    dht.begin();
    pinMode(ldrPin, INPUT);
    pinMode(soilMoisturePin, INPUT);
    pinMode(pumpPin, OUTPUT);
    digitalWrite(pumpPin, HIGH); // Pump off initially
}

void Sensor::readSensors()
{
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    lightLevel = 4095 - analogRead(ldrPin);
    soilMoisture = analogRead(soilMoisturePin);
    percentageSoilMoisture = map(soilMoisture, WET, DRY, 100, 0);
}

void Sensor::printReadings()
{
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Light Level: ");
    Serial.println(lightLevel);

    Serial.print("Soil Moisture: ");
    Serial.println(soilMoisture);

    Serial.print("Percentage Soil Moisture: ");
    Serial.print(percentageSoilMoisture);
    Serial.println(" %");
}

float Sensor::getTemperature() const { return temperature; }
float Sensor::getHumidity() const { return humidity; }
int Sensor::getLightLevel() const { return lightLevel; }
int Sensor::getSoilMoisture() const { return soilMoisture; }
int Sensor::getPercentageSoilMoisture() const { return percentageSoilMoisture; }
