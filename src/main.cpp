#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include "secrets.h"
#include "defines.h"
#include <time.h> // Para obter o timestamp

float humidity = 0;
float temperature = 0;
int lightLevel = 0;
int soilMoisture = 0;
int percentageSoilMoisture = 0;

unsigned long last_pub_aws = 0;
unsigned long interval_pub = 3600000;

WiFiClientSecure espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800; // Offset de -3 horas em segundos
const int daylightOffset_sec = 3600;

// Configurar o WiFi
void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Mensagem recebida no tópico: ");
  Serial.print(topic);
  Serial.print(". Mensagem: ");
  String message;
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();

  // Verifica se a mensagem recebida é para acionar a bomba de água
  if (message == "water_on")
  {
    // water_on();
  }
  else if (message == "water_off")
  {
    // water_off();
  }
}
void water_on()
{
  digitalWrite(WATER_PUMP_PIN, LOW);
}

void water_off()
{
  digitalWrite(WATER_PUMP_PIN, HIGH);
}
void reconnect()
{
  String clientId = "ESP32Client-" + String(WiFi.macAddress());
  while (!client.connected())
  {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect(clientId.c_str()))
    {
      Serial.println("Conectado");
      client.subscribe("$aws/things/SmartPlantPot/shadow/update/accepted");
      client.subscribe("$aws/things/SmartPlantPot/shadow/update/rejected");
      client.subscribe("$aws/things/SmartPlantPot/control"); // tópico de controle
    }
    else
    {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup_pins()
{
  pinMode(LDRPIN, INPUT);
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);
  digitalWrite(WATER_PUMP_PIN, HIGH); // Iniciar desligado
}

void setup_aws()
{
  espClient.setCACert(AWS_CERT_CA);
  espClient.setCertificate(AWS_CERT_CRT);
  espClient.setPrivateKey(AWS_CERT_PRIVATE);
  client.setServer(AWS_IOT_ENDPOINT, AWS_PORT);
  client.setCallback(callback);
}

void setup_time()
{
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

String getTimestamp()
{
  struct tm timeinfo;
  char timestamp[30];
  char msec[5];

  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Falha ao obter o tempo");
    return "";
  }

  // Obtém o tempo atual com milissegundos
  struct timeval tv;
  gettimeofday(&tv, NULL);

  // Formata o tempo em segundos
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &timeinfo);

  // Formata os milissegundos
  snprintf(msec, sizeof(msec), ".%03d", tv.tv_usec / 1000);

  // Concatena os milissegundos ao timestamp
  strcat(timestamp, msec);

  // Adiciona o sufixo "Z"
  strcat(timestamp, "Z");

  // Debug adicional
  Serial.print("Timestamp gerado: ");
  Serial.println(timestamp);

  return String(timestamp);
}

void setup()
{
  Serial.begin(115200);
  dht.begin();
  setup_pins();
  setup_wifi();
  setup_aws();
  setup_time();
  reconnect();
}

void updateShadow()
{
  String timestamp = getTimestamp();

  String payload = "{\"state\":{\"reported\":{";
  payload += "\"deviceId\": \"" + String(WiFi.macAddress()) + "\",";
  payload += "\"humidity\": " + String(humidity) + ",";
  payload += "\"light\": " + String(lightLevel) + ",";
  payload += "\"soilMoisture\": " + String(soilMoisture) + ",";
  payload += "\"temperature\": " + String(temperature) + ",";
  payload += "\"timestamp\": \"" + timestamp + "\"}}}";

  Serial.print("Atualizando Shadow com payload: ");
  Serial.println(payload);

  if (client.publish("$aws/things/SmartPlantPot/shadow/update", payload.c_str()))
  {
    Serial.println("Shadow atualizado com sucesso");
    last_pub_aws = millis();
  }
  else
  {
    Serial.println("Falha ao atualizar o Shadow");
  }
}

void water_pump()
{
  Serial.print("soilMoisture: ");
  Serial.println(soilMoisture);

  Serial.print("pump_water: ");
  Serial.println(PUMP_WATER);

  Serial.print("soilMoisture - pump_water: ");
  Serial.println(soilMoisture - PUMP_WATER);
  Serial.println(" ");

  if (soilMoisture >= PUMP_WATER)
  {
    water_on();
    Serial.println("WATER ON");
  }
  if (soilMoisture < PUMP_WATER)
  {
    water_off();
    Serial.println("WATER OFF");
  }
}

void read_DHT()
{
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Falha ao ler do sensor DHT!");
    delay(3000);
    return;
  }
}

void read_light_level()
{
  lightLevel = analogRead(LDRPIN);
}

void read_soil_moisture()
{
  soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  percentageSoilMoisture = map(soilMoisture, WET, DRY, 100, 0);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  read_soil_moisture();
  read_light_level();
  read_DHT();
  water_pump();

  if (millis() - last_pub_aws >= interval_pub || last_pub_aws == 0)
  {
    updateShadow();
  }
  delay(5000);
}
