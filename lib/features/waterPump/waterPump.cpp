#include "waterPump.h"
#include "config.h"
#include <Arduino.h>

WaterPump::WaterPump(uint8_t pumpPin)
    : pumpPin(pumpPin), waterPumpStatus(false) {}

void WaterPump::begin()
{
    pinMode(pumpPin, OUTPUT);
    digitalWrite(pumpPin, HIGH); // Pump off initially
}

bool WaterPump::getWaterPumpStatus() const
{
    return waterPumpStatus;
}

void WaterPump::setWaterPumpON()
{
    digitalWrite(pumpPin, LOW);
    waterPumpStatus = true;
}

void WaterPump::setWaterPumpOFF()
{
    digitalWrite(pumpPin, HIGH);
    waterPumpStatus = false;
}
