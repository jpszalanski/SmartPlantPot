#include <Arduino.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>
#include <Update.h>
#include "config.h"
#include "certs.h"

#include "sensorLDR/sensorLDR.h"
#include "sensorDHT/sensorDHT.h"
#include "sensorSoilMoisture/sensorSoilMoisture.h"
#include "waterPump/waterPump.h"
#include "OTAUpdate.h"
#include "aws_iot.h"
#include "nvs_flash.h"

SensorDHT sensorDHT(DHT_PIN, DHT_TYPE);
SensorLDR sensorLDR(LDR_PIN);
SensorSoilMoisture sensorSoilMoisture(SOIL_MOISTURE_PIN);
WaterPump waterPump(WATER_PUMP_PIN);

bool dataSentSuccessfully = false;
unsigned long lastSendAttempt = 0;
unsigned long lastSend = 0;
unsigned long lastPrint = 0;
bool isReboot = true;

OTAUpdate ota(OTA_PASSWORD);


void connectAWS();
bool publishSensorReadings();
bool publishSensorReadingsRealTime();
void callback(char *topic, byte *payload, unsigned int length);
void setupAWS();

void setup()
{
  Serial.begin(115200);

  // WiFi setup using WiFiManager
  WiFiManager wifiManager;

  // Definir o hostname antes de conectar
  //String hostname = String(THINGNAME) + "-" + WiFi.macAddress();
  String hostname = String(THINGNAME);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE); // Necessário para fixar o hostname
  WiFi.setHostname(hostname.c_str());                       // Define o hostname

  // Try to connect to WiFi, if it fails, enter AP mode for configuration
  if (!wifiManager.autoConnect(THINGNAME "-AP", AP_MANAGER_PASSWORD)) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
  }
    if (MDNS.begin(hostname.c_str())) {
    Serial.printf("mDNS iniciado com hostname: %s\n", hostname.c_str());
  } else {
    Serial.println("Erro ao configurar mDNS");
  }

  Serial.println("Connected to WiFi!");

  // Initialize sensors
  sensorDHT.begin();
  sensorLDR.begin();
  sensorSoilMoisture.begin();
  waterPump.begin();

  // Configure NTP to get the correct time
  configTime(GMT_OFF_SET_SEC, DAY_LIGHT_OFF_SET_SEC, NTP_SERVER);
  // Inicializa OTA após o Wi-Fi estar conectado
  ota.begin();

  // AWS IoT setup
  setupAWS();
}

void loop() {
  ota.handle();

  if (!client.connected()) {
    connectAWS();
  }
  client.loop();

  String currentTime = getFormattedTime();
  if (currentTime.length() == 0) {
    delay(1000); // Retry in a second if time is not available
    return;
  }

  if (!isReboot && (millis() - lastPrint >= 30000)) {
    sensorDHT.readSensor();
    sensorDHT.printSensorReadings();

    sensorLDR.readSensor();
    sensorLDR.printSensorReadings();

    sensorSoilMoisture.readSensor();
    sensorSoilMoisture.printSensorReadings();

    lastPrint = millis();
  }

  if (isSendInterval() && !dataSentSuccessfully) {
    sensorDHT.readSensor();
    sensorLDR.readSensor();
    sensorSoilMoisture.readSensor();

    if (publishSensorReadings(sensorDHT, sensorLDR, sensorSoilMoisture))
    {
      dataSentSuccessfully = true;
      delay(1000);
    }
  }
  if (!isSendInterval() && dataSentSuccessfully) {
    dataSentSuccessfully = false;
  }

  if (isReboot || (millis() - lastSend >= INTERVAL_REAL_TIME)) {
    sensorDHT.readSensor();
    sensorLDR.readSensor();
    sensorSoilMoisture.readSensor();
    publishSensorReadingsRealTime(sensorDHT, sensorLDR, sensorSoilMoisture);
    lastSend = millis();
    isReboot = false;
  }

  delay(1000);
}
