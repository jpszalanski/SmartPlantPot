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

bool dataSentSuccessfully = false;
unsigned long lastSendAttempt = 0;

void connectAWS();
bool publishSensorReadings();
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

  // sensor.readSensors();
  //  controlWaterPump(sensor);

  String currentTime = getFormattedTime();
  if (currentTime.length() == 0)
  {
    delay(1000); // Retry in a second if time is not available
    return;
  }
  /*
    int minute = currentTime.substring(15, 16).toInt();
    int second = currentTime.substring(17, 19).toInt();
    Serial.print("currentTime: ");
    Serial.println(currentTime);
    Serial.print("second: ");
    Serial.println(second);
    Serial.print("minute: ");
    Serial.println(minute);
    */
  Serial.print("isSendInterval(): ");
  Serial.println(isSendInterval());
  Serial.print("isWithinRetryWindow(): ");
  Serial.println(isWithinRetryWindow());
  Serial.print("dataSentSuccessfully: ");
  Serial.println(dataSentSuccessfully);

  if (isSendInterval() && !dataSentSuccessfully)

    if (isWithinRetryWindow() && !dataSentSuccessfully)
    {
      sensor.readSensors();

      if (storeSensorReadings(sensor, preferences))
      {
        if (publishSensorReadings(sensor, preferences))
        {
          dataSentSuccessfully = true;
        }
      }
      lastSendAttempt = millis();
    }
    else if (!isSendInterval() && isWithinRetryWindow() && !dataSentSuccessfully)
    {
      sensor.readSensors();

      if (storeSensorReadings(sensor, preferences))
      {
        if (publishSensorReadings(sensor, preferences))
        {
          dataSentSuccessfully = true;
        }
      }
    }
    else if (!isSendInterval() && !isWithinRetryWindow())
    {
      dataSentSuccessfully = false; // Reset for the next minute
    }

  delay(1000); // Check every second
}
