#ifndef WATERPUMP_H
#define WATERPUMP_H

#include "config.h"
#include <Arduino.h>
#include <Ticker.h> // Inclua a biblioteca correta

class WaterPump
{
public:
    WaterPump(uint8_t pumpPin);
    void begin();
    bool getWaterPumpStatus() const;
    void setWaterPumpON();
    void setWaterPumpOFF();

    // Função estática para ser usada como callback do Ticker
    static void staticAutoTurnOff(void* arg);

private:
    uint8_t pumpPin;
    bool waterPumpStatus;
    Ticker pumpTimer; // Instância do Ticker
    void autoTurnOff(); // Função para desligar a bomba
};

#endif
