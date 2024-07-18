#include <Arduino.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>
#include "config.h"
#include "Sensor.h"
#include "certs.h"
#include "aws_iot.h"

// Define global variables
const char *ntpServer = "pool.ntp.org"; // Define here
const long gmtOffset_sec = -3600;       // Offset of -3 hours in seconds
const int daylightOffset_sec = 3600;

// Declare the instance of the Sensor class
Sensor sensor(DHT_PIN, DHT_TYPE, LDR_PIN, SOIL_MOISTURE_PIN, WATER_PUMP_PIN);

// Instance for NVS storage
Preferences preferences;

void connectAWS();
void publishSensorReadings();
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
  sensor.begin();

  // Initialize preferences (NVS)
  preferences.begin("sensor_readings", false);

  // Configure NTP to get the correct time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

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

  sensor.readSensors();

  if (isSendInterval() || isWithinRetryWindow())
  {
    if (storeSensorReadings(sensor, preferences))
    {
      publishSensorReadings(sensor, preferences);
    }
  }

  delay(1000); // Check every second
}
