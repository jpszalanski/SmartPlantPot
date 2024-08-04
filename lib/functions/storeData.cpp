#include "storeData.h"
#include "config.h"
#include <Preferences.h>

const String targetHours[] = {"00", "03", "06", "09", "12", "15", "18", "21"};
const int targetHoursCount = sizeof(targetHours) / sizeof(targetHours[0]);
Preferences preferences;

bool storeSensorReadings(SensorDHT &sensorDHT, SensorLDR &sensorLDR, SensorSoilMoisture &sensorSoilMoisture);
{
    if (isnan(sensor.getTemperature()) || isnan(sensor.getHumidity()) || isnan(sensor.getLightLevel()) || isnan(sensor.getSoilMoisture()))
    {
        Serial.println("Error reading sensors. Data not saved.");
        return false;
    }

    String timestampRead = getFormattedTime();
    String hour = timestamp.substring(11, 13);

    preferences.begin(hour, true);
    preferences.putFloat("temperature", sensor.getTemperature());
    preferences.putFloat("humidity", sensor.getHumidity());
    preferences.putInt("lightLevel", sensor.getLightLevel());
    preferences.putInt("soilMoisture", sensor.getSoilMoisture());
    preferences.putInt("percSoilMoist", sensor.getPercentageSoilMoisture());
    preferences.putString("timestamp", timestampRead);

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

// Função para verificar se a hora está no array
bool isTargetHour(const String &hour)
{
    for (int i = 0; i < targetHoursCount; ++i)
    {
        if (hour == targetHours[i])
        {
            return true;
        }
    }
    return false;
}
