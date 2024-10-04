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
    digitalWrite(pumpPin, LOW); // Ativa a bomba
    waterPumpStatus = true;

    // Define o temporizador para desligar após 3 segundos
    //pumpTimer.attach(3, staticAutoTurnOff, static_cast<void*>(this)); // Passa `this` como argumento
}

void WaterPump::setWaterPumpOFF()
{
    digitalWrite(pumpPin, HIGH); // Desativa a bomba
    waterPumpStatus = false;

    // Desativa o temporizador para garantir que não haja chamadas adicionais
   // pumpTimer.detach();
}

void WaterPump::autoTurnOff()
{
    setWaterPumpOFF(); // Desliga a bomba após o tempo especificado
    Serial.println("Water pump automatically deactivated after 3 seconds");
}

// Função estática que chama o método membro `autoTurnOff`
void WaterPump::staticAutoTurnOff(void* arg)
{
    WaterPump* instance = static_cast<WaterPump*>(arg);
    if (instance)
    {
        instance->autoTurnOff(); // Chama o método `autoTurnOff` da instância
    }
}
