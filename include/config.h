#ifndef CONFIG_H
#define CONFIG_H

#define SOIL_MOISTURE_PIN 35 // 6E
#define DHT_PIN 4            // 13D
#define DHT_TYPE DHT11
#define LDR_PIN 34        // 5E
#define WATER_PUMP_PIN 13 // 15E

// Constantes para o mapeamento do sensor de umidade do solo
#define WET 4095
#define DRY 0

#define PUMP_WATER 2700

// Configurações de tempo para salvar e enviar dados
#define SEND_INTERVAL_SECONDS 60
#define RETRY_WINDOW_SECONDS 10

#define THINGNAME "SmartPlantPot"

const char AWS_IOT_ENDPOINT[] = "a25uug4xbk339z-ats.iot.us-east-1.amazonaws.com"; // endpoint AWS IoT
const int AWS_PORT = 8883;
const char AWS_PUB_TOPIC_UPDATE[] = "$aws/things/" THINGNAME "/shadow/update";
const char AWS_SUB_TOPIC_UPDATE[] = "$aws/things/" THINGNAME "/shadow/update";
const char AWS_SUB_TOPIC_ACCEPTED[] = "$aws/things/" THINGNAME "/shadow/update/accepted";
const char AWS_SUB_TOPIC_REJECTED[] = "$aws/things/" THINGNAME "/shadow/update/rejected";
const char AWS_SUB_TOPIC_CONTROLE[] = "$aws/things/" THINGNAME "/shadow/control";

#endif
