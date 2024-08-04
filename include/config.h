#ifndef CONFIG_H
#define CONFIG_H

#define SOIL_MOISTURE_PIN 35 // 6E
#define DHT_PIN 4            // 13D
#define DHT_TYPE DHT22
#define LDR_PIN 34        // 5E
#define WATER_PUMP_PIN 13 // 15E

// Constantes para o mapeamento do sensor de umidade do solo
#define WET 4095
#define DRY 0

#define AUTO_PUMP_WATER 40

// Configurações de tempo para salvar e enviar dados
#define SEND_INTERVAL "H" // M para enviar a cada 10 minutos fechados ou H para cada hora fechada
#define SEND_RETRY 10 //qtd de vezes que tentara enviar novamente em caso de falha. se SEND_INTERVAL = M, uma vez por segunda, se = H uma vez por minuto.

#define INTERVAL_REAL_TIME 900000

#define THINGNAME "SmartPlantPot"

const char AWS_IOT_ENDPOINT[] = "a25uug4xbk339z-ats.iot.us-east-1.amazonaws.com"; // endpoint AWS IoT
const int AWS_PORT = 8883;
const char AWS_PUB_TOPIC_UPDATE[] = "$aws/things/" THINGNAME "/shadow/update";
const char AWS_PUB_TOPIC_REALTIME[] = "$aws/things/" THINGNAME "/shadow/real";
const char AWS_SUB_TOPIC_UPDATE[] = "$aws/things/" THINGNAME "/shadow/update";
const char AWS_SUB_TOPIC_ACCEPTED[] = "$aws/things/" THINGNAME "/shadow/update/accepted";
const char AWS_SUB_TOPIC_REJECTED[] = "$aws/things/" THINGNAME "/shadow/update/rejected";
const char AWS_SUB_TOPIC_CONTROLE[] = "$aws/things/" THINGNAME "/shadow/control";
const char AWS_SUB_TOPIC_FIRMWARE[] = "$aws/things/" THINGNAME "/jobs/notify-next";
const char AWS_SUB_TOPIC_FIRMWARE_ACCEPTED[] = "$aws/things/" THINGNAME "/jobs/$next/get/accepted";


// Define global variables
const char NTP_SERVER[] = "pool.ntp.org"; // Define here
const long GMT_OFF_SET_SEC = -3600;       // Offset of -3 hours in seconds
const int DAY_LIGHT_OFF_SET_SEC = 3600;

/*
// Define global variables
const char *ntpServer = "pool.ntp.org"; // Define here
const long gmtOffset_sec = -3600;       // Offset of -3 hours in seconds
const int daylightOffset_sec = 3600;

*/
#endif
