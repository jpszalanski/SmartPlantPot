#include "aws_iot.h"
#include "config.h"
#include "certs.h"

#include <HTTPClient.h>
#include <Update.h>
#include <cJSON.h>

WiFiClientSecure net;
PubSubClient client(net);

bool waterPumpState = false;
int failedCounter = 0;

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
        Serial.print("Tentando conectar MQTT...");
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            client.subscribe(AWS_SUB_TOPIC_ACCEPTED);
            client.subscribe(AWS_SUB_TOPIC_CONTROLE);
            client.subscribe(AWS_SUB_TOPIC_FIRMWARE);
            client.subscribe(AWS_SUB_TOPIC_FIRMWARE_ACCEPTED);
            failedCounter = 0;
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            failedCounter++;
            if (failedCounter > AWS_FAILED_COUNTER)
            {
                ESP.restart();
            }
            delay(5000);
        }
    }
}

bool publishSensorReadings(SensorDHT &sensorDHT, SensorLDR &sensorLDR, SensorSoilMoisture &sensorSoilMoisture)
{
    String timestamp = getFormattedTime();
    String payload = "{\"state\":{\"reported\":{";
    payload += "\"deviceId\": \"" + String(WiFi.macAddress()) + "\",";
    payload += "\"temperature\": " + String(sensorDHT.getTemperature(), 2) + ",";
    payload += "\"humidity\": " + String(sensorDHT.getHumidity(), 2) + ",";
    payload += "\"lightLevel\": " + String(sensorLDR.getLightLevel()) + ",";
    payload += "\"soilMoisture\": " + String(sensorSoilMoisture.getSoilMoisture()) + ",";
    payload += "\"percentageSoilMoisture\": " + String(sensorSoilMoisture.getPercentageSoilMoisture()) + ",";
    payload += "\"updatedAt\": \"" + timestamp + "\"";
    payload += "}}}";

    if (client.publish(AWS_PUB_TOPIC_UPDATE, payload.c_str()))
    {
        Serial.println("Sensor readings published successfully to AWS IoT:");
        Serial.println(payload);
        return true;
    }
    else
    {
        Serial.println("Failed to publish sensor readings to AWS IoT");
        return false;
    }
}

bool publishSensorReadingsRealTime(SensorDHT &sensorDHT, SensorLDR &sensorLDR, SensorSoilMoisture &sensorSoilMoisture)
{
    String timestamp = getFormattedTime();
    String payload = "{\"state\":{\"reported\":{";
    payload += "\"deviceId\": \"" + String(WiFi.macAddress()) + "\",";
    payload += "\"temperature\": " + String(sensorDHT.getTemperature(), 2) + ",";
    payload += "\"humidity\": " + String(sensorDHT.getHumidity(), 2) + ",";
    payload += "\"lightLevel\": " + String(sensorLDR.getLightLevel()) + ",";
    payload += "\"soilMoisture\": " + String(sensorSoilMoisture.getSoilMoisture()) + ",";
    payload += "\"percentageSoilMoisture\": " + String(sensorSoilMoisture.getPercentageSoilMoisture()) + ",";
    payload += "\"updatedAt\": \"" + timestamp + "\"";
    payload += "}}}";

    if (client.publish(AWS_PUB_TOPIC_REALTIME, payload.c_str()))
    {
        Serial.println("REAL TIME Sensor readings published successfully to AWS IoT:");
        Serial.println(payload);
        return true;
    }
    else
    {
        Serial.println("Failed to publish sensor readings to AWS IoT");
        return false;
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
    }
    else if (String(topic) == AWS_SUB_TOPIC_CONTROLE)
    {
        if (message == "true")
        {
            digitalWrite(WATER_PUMP_PIN, LOW); // Activate the water pump
            waterPumpState = true;
            Serial.println("Water pump activated via MQTT");
        }
        else if (message == "false")
        {
            digitalWrite(WATER_PUMP_PIN, HIGH); // Deactivate the water pump
            waterPumpState = false;
            Serial.println("Water pump deactivated via MQTT");
        }
    }
    else if (String(topic) == AWS_SUB_TOPIC_FIRMWARE)
    {
        Serial.println("Job received");
        client.publish(AWS_SUB_TOPIC_FIRMWARE_ACCEPTED, " ");
        processJob(message);
    }
    else if (String(topic) == AWS_SUB_TOPIC_FIRMWARE_ACCEPTED)
    {
        processJob(message);
    }
}

bool isSendInterval()
{
    String currentTime = getFormattedTime();
    if (currentTime.length() == 0)
    {
        return false; // Falha ao obter o time
    }

    int minute = 99;

    if (SEND_INTERVAL == "H")
    {
        minute = currentTime.substring(14, 16).toInt();
    }
    else if (SEND_INTERVAL == "M")
    {
        minute = currentTime.substring(15, 16).toInt();
    }

    return (minute <= SEND_RETRY);
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

void controlWaterPump(SensorSoilMoisture &sensorSoilMoisture)
{
    // TODO: autoirrigacao
}

void processJob(String jobDocument)
{
    Serial.println("processJob");
    cJSON *json = cJSON_Parse(jobDocument.c_str());
    if (json == NULL)
    {
        Serial.println("Error parsing JSON");
        return;
    }

    cJSON *firmwareUrl = cJSON_GetObjectItemCaseSensitive(json, "firmwareUrl");
    if (cJSON_IsString(firmwareUrl) && (firmwareUrl->valuestring != NULL))
    {
        Serial.println("Downloading firmware...");
        if (downloadFirmware(firmwareUrl->valuestring))
        {
            applyFirmware();
        }
    }
    else
    {
        Serial.println("firmwareUrl not found or invalid");
    }

    cJSON_Delete(json);
}

bool downloadFirmware(const char *url)
{
    Serial.println("downloadFirmware");
    HTTPClient httpClient;
    httpClient.begin(url);
    int httpCode = httpClient.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        int len = httpClient.getSize();
        WiFiClient *stream = httpClient.getStreamPtr();

        if (Update.begin(len))
        {
            size_t written = Update.writeStream(*stream);
            if (written == len)
            {
                return Update.end();
            }
        }
    }
    return false;
}

void applyFirmware()
{
    Serial.println("applyFirmware");
    if (Update.isFinished())
    {
        ESP.restart();
    }
}
