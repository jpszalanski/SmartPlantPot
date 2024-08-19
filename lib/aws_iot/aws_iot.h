#ifndef AWS_IOT_H
#define AWS_IOT_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "sensorDHT/sensorDHT.h"
#include "sensorSoilMoisture/sensorSoilMoisture.h"
#include "sensorLDR/sensorLDR.h"
#include "waterPump/waterPump.h"
#include "Preferences.h"

#include <HTTPClient.h>
#include <Update.h>

// Declaração do cliente MQTT
extern PubSubClient client;

// Funções de configuração e conexão AWS
void setupAWS();
void connectAWS();

// Funções de publicação de leituras dos sensores
bool publishSensorReadings(SensorDHT &sensorDHT, SensorLDR &sensorLDR, SensorSoilMoisture &sensorSoilMoisture);
bool publishSensorReadingsRealTime(SensorDHT &sensorDHT, SensorLDR &sensorLDR, SensorSoilMoisture &sensorSoilMoisture);

// Função de callback do MQTT
void callback(char *topic, byte *payload, unsigned int length);

// Funções de gerenciamento de intervalos e armazenamento de leituras
bool isSendInterval();
// bool isWithinRetryWindow();

// Funções auxiliares
String getFormattedTime();
void controlWaterPump(WaterPump &waterPump);

// Funções de processamento de trabalhos e atualização de firmware
void processJob(String jobDocument);
bool downloadFirmware(const char *url);
String extractFirmwareUrl(String jobDocument);
void applyFirmware();

#endif