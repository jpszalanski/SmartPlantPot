#ifndef AWS_IOT_H
#define AWS_IOT_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "sensor.h"
#include "Preferences.h"


// Declaração do cliente MQTT
extern PubSubClient client;

void setupAWS();
void connectAWS();
void publishSensorReadings(Sensor &sensor, Preferences &preferences);
void callback(char *topic, byte *payload, unsigned int length);

bool isSendInterval();
bool isWithinRetryWindow();
bool storeSensorReadings(Sensor &sensor, Preferences &preferences);
String getFormattedTime();

void controlWaterPump(Sensor &sensor);

#endif
