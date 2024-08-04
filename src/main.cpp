#include <Arduino.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>
#include "config.h"
#include "certs.h"

#include "sensorLDR/sensorLDR.h"
#include "sensorDHT/sensorDHT.h"
#include "sensorSoilMoisture/sensorSoilMoisture.h"
#include "waterPump/waterPump.h"
#include "aws_iot.h"

SensorDHT sensorDHT(DHT_PIN, DHT_TYPE);
SensorLDR sensorLDR(LDR_PIN);
SensorSoilMoisture sensorSoilMoisture(SOIL_MOISTURE_PIN);
WaterPump waterPump(WATER_PUMP_PIN);

// Instance for NVS storage
// Preferences preferences;

bool dataSentSuccessfully = false;
unsigned long lastSendAttempt = 0;
unsigned long lastSend = 0;
bool isReboot = true;

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

  // Try to connect to WiFi, if it fails, enter AP mode for configuration
  if (!wifiManager.autoConnect(THINGNAME "-AP"))
  {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Connected to WiFi!");

  // Initialize sensors
  sensorDHT.begin();
  sensorLDR.begin();
  sensorSoilMoisture.begin();

  // Initialize preferences (NVS)
  // preferences.begin("sensor_readings", false);

  // Configure NTP to get the correct time
  configTime(GMT_OFF_SET_SEC, DAY_LIGHT_OFF_SET_SEC, NTP_SERVER);

  // AWS IoT setup
  setupAWS();
}

void loop()
{
  if (!client.connected())
  {
    connectAWS();
  }
  client.loop();

  String currentTime = getFormattedTime();
  if (currentTime.length() == 0)
  {
    delay(1000); // Retry in a second if time is not available
    return;
  }

  if (isSendInterval() && !dataSentSuccessfully)
  {
    sensorDHT.readSensor();
    sensorLDR.readSensor();
    sensorSoilMoisture.readSensor();

    // if (storeSensorReadings(sensor, preferences))
    // {
    if (publishSensorReadings(sensorDHT, sensorLDR, sensorSoilMoisture))
    {
      dataSentSuccessfully = true;
      delay(1000); // Check every second
      //  }
    }
  }
  if (!isSendInterval() && dataSentSuccessfully)
  {
    dataSentSuccessfully = false;
  }

  if (isReboot || (millis() - lastSend >= INTERVAL_REAL_TIME))
  {
    sensorDHT.readSensor();
    sensorLDR.readSensor();
    sensorSoilMoisture.readSensor();
    publishSensorReadingsRealTime(sensorDHT, sensorLDR, sensorSoilMoisture);
    lastSend = millis();
    isReboot = false;
  }

  delay(1000);
}
