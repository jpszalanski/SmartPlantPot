#include "sensorDHT.h"
#include "config.h"
#include <Arduino.h>

SensorDHT::SensorDHT(uint8_t dhtPin, uint8_t dhtType)
    : dht(dhtPin, dhtType) {}

void SensorDHT::begin()
{
    dht.begin();
}

void SensorDHT::readSensor()
{
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
}

void SensorDHT::printSensorReadings()
{
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

}

float SensorDHT::getTemperature() const { return temperature; }
float SensorDHT::getHumidity() const { return humidity; }
