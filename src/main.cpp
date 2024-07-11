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
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

// configurar o WiFi
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
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect()
{
  String clientId = "ESP32Client-" + String(WiFi.macAddress());
  while (!client.connected())
  {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect(clientId.c_str()))
    {
      Serial.println("conectado");
      client.subscribe(AWS_SUB_TOPIC_ACCEPTED);
      client.subscribe(AWS_SUB_TOPIC_REJECTED);
    }
    else
    {
      Serial.print("falhou, rc=");
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
  digitalWrite(WATER_PUMP_PIN, HIGH); // iniciar desligado
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
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Falha ao obter o tempo");
    return "";
  }
  time(&now);
  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
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
  // client.setServer(AWS_IOT_ENDPOINT, AWS_PORT);
  reconnect();
}

void publishAws()
{
  String timestamp = getTimestamp();
  String payload = "{\"state\":{\"reported\":{";
  payload += "\"deviceId\": \"" + String(WiFi.macAddress()) + "\",";
  payload += "\"createdAt\": \"" + timestamp + "\",";
  payload += "\"updatedAt\": \"" + timestamp + "\",";
  payload += "\"humidity\": " + String(humidity) + ",";
  payload += "\"temperature\": " + String(temperature) + ",";
  payload += "\"light\": " + String(lightLevel) + ",";
  payload += "\"soilMoisture\": " + String(soilMoisture) + ",";
  payload += "\"timestamp\": \"" + timestamp + "\"}}}";

  Serial.print("Enviando payload para o tópico ");
  Serial.print(AWS_PUB_TOPIC);
  Serial.print(": ");
  Serial.println(payload);
  Serial.println("---------------------------------");

  if (client.publish(AWS_PUB_TOPIC, payload.c_str()))
  {
    Serial.println("Mensagem publicada com sucesso");
    last_pub_aws = millis();
  }
  else
  {
    Serial.println("Falha ao publicar mensagem");
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
    publishAws();
  }
  delay(5000);
}
