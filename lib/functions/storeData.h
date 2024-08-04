#ifndef STOREDATA_H
#define STOREDATA_H

#include "sensorDHT/sensorDHT.h"
#include "sensorSoilMoisture/sensorSoilMoisture.h"
#include "sensorLDR/sensorLDR.h"
#include "waterPump/waterPump.h"
#include "Preferences.h"

bool storeSensorReadings(SensorDHT &sensorDHT, SensorLDR &sensorLDR, SensorSoilMoisture &sensorSoilMoisture, Preferences &preferences);

#endif