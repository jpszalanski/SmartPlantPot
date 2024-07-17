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
unsigned long interval_pub = 60000;
unsigned long last_mqtt_command = 0;
const unsigned long mqtt_command_interval = 10000; // 10 segundos

bool mqtt_override = false;

WiFiClientSecure espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3600; // Offset de -3 horas em segundos
const int daylightOffset_sec = 3600;

void setup_wifi();
void callback(char *topic, byte *payload, unsigned int length);
void handleCommand(const String &message);
void water_on();
void water_off();
void reconnect();
void setup_pins();
void setup_aws();
void setup_time();
String getTimestamp();
bool isMultipleOf10Minutes(const String &timestamp);
void updateShadow();
void water_pump();
void read_DHT();
void read_light_level();
void read_soil_moisture();

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
    message += (char)payload[i];
  }
  Serial.println(message);
  handleCommand(message);
}

void handleCommand(const String &message)
{
  mqtt_override = true;
  last_mqtt_command = millis();

  String lowerMessage = message;
  lowerMessage.toLowerCase();

  if (lowerMessage.equals("true"))
  {
    water_on();
    Serial.println("WATER ON - CLOUD");
  }
  else if (lowerMessage.equals("false"))
  {
    updateShadow();
    water_off();
    Serial.println("WATER OFF - CLOUD");
  }
}

void water_on()
{
  updateShadow();
  digitalWrite(WATER_PUMP_PIN, LOW);
}

void water_off()
{
  digitalWrite(WATER_PUMP_PIN, HIGH);
}

void reconnect()
{
  String clientId = String(WiFi.macAddress());
  while (!client.connected())
  {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect(clientId.c_str()))
    {
      Serial.println("Conectado");
      client.subscribe(AWS_SUB_TOPIC_ACCEPTED);
      client.subscribe(AWS_SUB_TOPIC_REJECTED);
      client.subscribe(AWS_SUB_TOPIC_CONTROLE); // tópico de controle
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

  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Falha ao obter o tempo");
    return "erro";
  }

  // Formata o tempo em segundos
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &timeinfo);

  return String(timestamp);
}

bool isMultipleOf10Minutes(const String &timestamp)
{
  // Extrai os minutos do timestamp
  int minutes = timestamp.substring(14, 16).toInt();
  return (minutes % 10) == 0;
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
  // Montando o payload JSON
  String payload = "{\"state\":{\"reported\":{";
  payload += "\"deviceId\": \"" + String(WiFi.macAddress()) + "\",";
  payload += "\"humidity\": " + String(humidity, 2) + ","; // Ajuste para duas casas decimais
  payload += "\"light\": " + String(lightLevel) + ",";
  payload += "\"soilMoisture\": " + String(percentageSoilMoisture) + ",";
  payload += "\"temperature\": " + String(temperature, 2) + "}}}"; // Ajuste para duas casas decimais

  // Debug: Exibindo o payload
  Serial.print("Atualizando Shadow com payload: ");
  Serial.println(payload);

  // Publicando o payload no tópico AWS
  if (client.publish(AWS_PUB_TOPIC_UPDATE, payload.c_str()))
  {
    Serial.println("Shadow atualizado com sucesso");
    last_pub_aws = millis(); // Atualizando o tempo da última publicação
  }
  else
  {
    Serial.println("Falha ao atualizar o Shadow");
  }
}

void water_pump()
{
  Serial.print("temperature: ");
  Serial.println(temperature);

  Serial.print("humidity: ");
  Serial.println(humidity);

  Serial.print("lightLevel: ");
  Serial.println(lightLevel);

  Serial.print("soilMoisture: ");
  Serial.println(soilMoisture);

  Serial.print("pump_water: ");
  Serial.println(PUMP_WATER);

  Serial.print("soilMoisture - pump_water: ");
  Serial.println(soilMoisture - PUMP_WATER);

  Serial.print("percentageSoilMoisture: ");
  Serial.println(percentageSoilMoisture);

  Serial.println(" ");

  if (!mqtt_override)
  {
    if (percentageSoilMoisture < 50)
    {
      water_on();
      Serial.println("WATER ON - sensor");
    }
    if (percentageSoilMoisture >= 50)
    {
      water_off();
      Serial.println("WATER OFF - sensor");
    }
  }
  else
  {
    // Checa se já passaram 10 segundos desde o último comando MQTT
    if (millis() - last_mqtt_command >= mqtt_command_interval)
    {
      mqtt_override = false; // Volta a considerar a umidade do solo
    }
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
  lightLevel = 4095 - analogRead(LDRPIN);

  if (isnan(lightLevel))
  {
    Serial.println("Falha ao ler do sensor LDR!");
    delay(3000);
    return;
  }
}

void read_soil_moisture()
{
  soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  percentageSoilMoisture = map(soilMoisture, WET, DRY, 100, 0);

  if (isnan(soilMoisture))
  {
    Serial.println("Falha ao ler do sensor Soil Moisture");
    delay(3000);
    return;
  }
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

  if (!isnan(lightLevel) || !isnan(humidity) || !isnan(temperature) || !isnan(lightLevel))
  {
    String timestamp = getTimestamp();
    if (timestamp != "erro")
    {
      if (isMultipleOf10Minutes(timestamp))
      {
        if (millis() - last_pub_aws >= interval_pub)
        {
          updateShadow();
        }
      }
      else if (last_pub_aws == 0)
      {
        updateShadow();
      }
    }
  }

  delay(3000);
}
