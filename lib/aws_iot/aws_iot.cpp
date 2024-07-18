#include "aws_iot.h"
#include "config.h"
#include "certs.h"

WiFiClientSecure net;
PubSubClient client(net);

bool waterPumpState = false;

void setupAWS()
{
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);
    client.setServer(AWS_IOT_ENDPOINT, AWS_PORT);
    client.setCallback(callback);
    connectAWS();
}

void connectAWS()
{
    String clientId = String(WiFi.macAddress());
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            client.subscribe(AWS_SUB_TOPIC_ACCEPTED);
            client.subscribe(AWS_SUB_TOPIC_CONTROLE);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void publishSensorReadings(Sensor &sensor, Preferences &preferences)
{
    String payload = "{\"state\":{\"reported\":{";
    payload += "\"deviceId\": \"" + String(WiFi.macAddress()) + "\",";
    payload += "\"temperature\": " + String(sensor.getTemperature(), 2) + ",";
    payload += "\"humidity\": " + String(sensor.getHumidity(), 2) + ",";
    payload += "\"lightLevel\": " + String(sensor.getLightLevel()) + ",";
    payload += "\"soilMoisture\": " + String(sensor.getSoilMoisture()) + ",";
    payload += "\"percentageSoilMoisture\": " + String(sensor.getPercentageSoilMoisture()) + ",";
    payload += "\"timestamp\": \"" + preferences.getString("timestamp") + "\"";
    payload += "}}}";

    if (client.publish(AWS_PUB_TOPIC_UPDATE, payload.c_str()))
    {
        Serial.println("Sensor readings published successfully to AWS IoT:");
        Serial.println(payload);
    }
    else
    {
        Serial.println("Failed to publish sensor readings to AWS IoT");
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message received on topic: ");
    Serial.println(topic);
    String message;
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    Serial.println("Message: " + message);

    if (String(topic) == AWS_SUB_TOPIC_ACCEPTED)
    {
        Serial.println("Accepted topic received");
        // Process message if necessary
    }
    else if (String(topic) == AWS_SUB_TOPIC_CONTROLE)
    {
        if (message == "on")
        {
            digitalWrite(WATER_PUMP_PIN, LOW); // Activate the water pump
            waterPumpState = true;
            Serial.println("Water pump activated via MQTT");
        }
        else if (message == "off")
        {
            digitalWrite(WATER_PUMP_PIN, HIGH); // Deactivate the water pump
            waterPumpState = false;
            Serial.println("Water pump deactivated via MQTT");
        }
    }
}

bool isSendInterval()
{
    String currentTime = getFormattedTime();
    return currentTime.endsWith(":00");
}

bool isWithinRetryWindow()
{
    String currentTime = getFormattedTime();
    if (currentTime.length() == 0)
        return false; // Failed to obtain time
    int second = currentTime.substring(17, 19).toInt();
    return (second > 0 && second <= RETRY_WINDOW_SECONDS);

    bool storeSensorReadings(Sensor & sensor, Preferences & preferences)
    {
        if (isnan(sensor.getTemperature()) || isnan(sensor.getHumidity()) || isnan(sensor.getLightLevel()) || isnan(sensor.getSoilMoisture()))
        {
            Serial.println("Error reading sensors. Data not saved.");
            return false;
        }

        String timestamp = getFormattedTime();
        preferences.putFloat("temperature", sensor.getTemperature());
        preferences.putFloat("humidity", sensor.getHumidity());
        preferences.putInt("lightLevel", sensor.getLightLevel());
        preferences.putInt("soilMoisture", sensor.getSoilMoisture());
        preferences.putInt("percSoilMoist", sensor.getPercentageSoilMoisture());
        preferences.putString("timestamp", timestamp);

        Serial.println("Sensor readings stored:");
        Serial.println("Timestamp: " + timestamp);
        sensor.printReadings();
        return true;
    }

    String getFormattedTime()
    {
        time_t now;
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("Failed to obtain time");
            return "";
        }
        char timeString[30];
        strftime(timeString, sizeof(timeString), "%Y-%m-%dT%H:%M:%S", &timeinfo);
        return String(timeString);
    }

    void controlWaterPump(Sensor & sensor)
    {
        if (!waterPumpState && sensor.getPercentageSoilMoisture() < 30)
        {
            digitalWrite(WATER_PUMP_PIN, LOW); // Activate the water pump
            waterPumpState = true;
            Serial.println("Water pump activated automatically by soil moisture level");
        }
        else if (waterPumpState && sensor.getPercentageSoilMoisture() >= 30)
        {
            digitalWrite(WATER_PUMP_PIN, HIGH); // Deactivate the water pump
            waterPumpState = false;
            Serial.println("Water pump deactivated automatically by soil moisture level");
        }
    }
