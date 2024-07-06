#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include "secrets.h"

// Definições de pinos

#define SOIL_MOISTURE_PIN 32
#define DHTPIN 4
#define DHTTYPE DHT11
#define LDRPIN 34
#define WATER_PUMP_PIN 15

const int dry = 4095; // value for dry sensor
const int wet = 1350; // value for wet sensor
const int pump_water = 2700;

float humidity = 0;
float temperature = 0;
int lightLevel = 0;
int soilMoisture = 0;
int percentageSoilMoisture = 0;

const int aws_port = 8883;

unsigned long last_pub_aws = 0;
unsigned long interval_pub = 3600000;

WiFiClientSecure espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

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

// Função de callback do MQTT
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

// Função para reconectar ao MQTT se a conexão cair
void reconnect()
{
  // Tente reconectar até que seja bem-sucedido
  while (!client.connected())
  {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect(THINGNAME))
    {
      Serial.println("conectado");
      // Subscreva a um tópico, se necessário
      // client.subscribe("seu/topico");
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
  digitalWrite(WATER_PUMP_PIN, LOW); // iniciar deligado
}

void setup_aws()
{
  // Carregar certificados diretamente no código

  espClient.setCACert(AWS_CERT_CA);
  espClient.setCertificate(AWS_CERT_CRT);
  espClient.setPrivateKey(AWS_CERT_PRIVATE);
  client.setServer(AWS_IOT_ENDPOINT, aws_port);
  client.setCallback(callback);
}

void setup()
{
  Serial.begin(115200);

  dht.begin();
  setup_pins();
  setup_wifi();
  setup_aws();
  reconnect();
}

void publishAws()
{

  String payload = "{\"state\": {\"reported\": {";
  payload += "\"temperature\": ";
  payload += temperature;
  payload += ", \"humidity\": ";
  payload += humidity;
  payload += ", \"light\": ";
  payload += lightLevel;
  payload += ", \"soilMoisture\": ";
  payload += percentageSoilMoisture;
  payload += "}}}";

  Serial.print("Enviando payload para o tópico ");
  Serial.print(AWS_TOPIC);
  Serial.print(": ");
  Serial.println(payload);

  if (client.publish(AWS_TOPIC, payload.c_str()))
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
  Serial.println(pump_water);

  Serial.print("soilMoisture - pump_water: ");
  Serial.println(soilMoisture - pump_water);
  Serial.println(" ");

  if (soilMoisture <= pump_water)
  {
    water_on();
    Serial.println("WATER ON");
  }
  if (soilMoisture > pump_water)
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
  percentageSoilMoisture = map(soilMoisture, wet, dry, 100, 0);
}

void print_data()
{
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.println("   ");
  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.println(" C");
  Serial.println("   ");
  Serial.print("Light: ");
  Serial.print(lightLevel);
  Serial.println(" lx");
  Serial.println("   ");
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisture);
  Serial.print(" | ");
  Serial.print(percentageSoilMoisture);
  Serial.println(" %");
  Serial.println("---------------------------------");
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  read_light_level();
  read_DHT();
  read_soil_moisture();
  water_pump();
  print_data();

  if (millis() - last_pub_aws >= interval_pub || last_pub_aws == 0)
  {
    publishAws();
  }
  delay(5000);
}
