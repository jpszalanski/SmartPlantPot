#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>

// Definições de pinos
#define DHTPIN 4
#define DHTTYPE DHT11
#define LDRPIN 34
#define SOIL_MOISTURE_PIN 35
#define WATER_PUMP_PIN 5

const int dry = 4095; // value for dry sensor
const int wet = 1239; // value for wet sensor
float h = 0;
float t = 0;
int lightLevel = 0;
int soilMoisture = 0;
int percentageSoilMoisture = 0;

// Configurações WiFi e AWS IoT
const char *ssid = "AlexssJeff";
const char *password = "05122014ja";
const char *aws_endpoint = "a25uug4xbk339z-ats.iot.us-east-1.amazonaws.com"; // Substitua pelo seu endpoint AWS IoT
const int aws_port = 8883;
const char *thing_name = "SmartPlantPot"; // Nome do dispositivo
const char *aws_topic = "$aws/things/SmartPlantPot/shadow/update";
unsigned long last_pub_aws = 0;
unsigned long interval_pub = 3600000;

// Certificado CA
const char *ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

// Certificado do Cliente
const char *client_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAMdzkbVzr3/AvIDxvxt6ZiwEKJF/MA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yNDA2MjgxNDQ5
NDdaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDC7vPDt7Q7FDqnSBNr
6sgNKGeOsU+a6RfdRlh2bruLqN1rKROleqRQ1LLYAr2m0dZcsjfLsNPizULs3R3y
rg5sPNY4y/PCi3cCHijjEh6GF8jfuK8Uc4lYmVkAf4n1d/gNpjmB7JbIMGMuBPPt
Nzj5UytPKPtJfzJ8qyGJzHbq2c5h01IvKCQjqxItj7LfyWUNZB8+Y6LqGIPcWCP/
9nYUa+9NKx/Mx+4HqHgFzFnKDqMW7xRUVINqhmiHp/p8kcLrkAvKV8O2CgCuaEEd
vsSq9oKIdkb443xgtaVG7wJ9BKV48Jmo7n6V1X9ziabN2Zn0HIP8di2LGUO+ajXl
u0LhAgMBAAGjYDBeMB8GA1UdIwQYMBaAFLqOhOiqEowxB8qFPrfjMzm17rdxMB0G
A1UdDgQWBBT4+1ukVEMYHKAaNGXDFds2gdgzQDAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAGygrToR1R6WONHm7FQ6iYyuY
nuIESCbDsTcWcx9dizmjKCDdWwwsJnLnHnuzPVQnCRNmRv0AfAccUL7P3GGMXiBJ
alRgXM4dPAInrkptV8MXE7Q9wxOfhoz0+pgrq6clkYxpRAf8d61zDrIEmF2JNbe1
IdkAe+kK+ztBuIlKHB5pTpPaitplgkcyQ3hRRGHsND3+r1lf41VbyyLiupAtaojI
sPHRazsdDsVBjRHfaOIq0Pm+uF70qavA8g7ZosT2qgbfgMlEAEc+ENYFa1Px+UPv
FIpgVXK9FjWhOewvgOx3bv3eYYRs1E74YlRERBMb/GENFqGKbxPmRfgk6hcH+Q==
-----END CERTIFICATE-----
)EOF";

// Chave Privada do Cliente
const char *client_key = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEAwu7zw7e0OxQ6p0gTa+rIDShnjrFPmukX3UZYdm67i6jdaykT
pXqkUNSy2AK9ptHWXLI3y7DT4s1C7N0d8q4ObDzWOMvzwot3Ah4o4xIehhfI37iv
FHOJWJlZAH+J9Xf4DaY5geyWyDBjLgTz7Tc4+VMrTyj7SX8yfKshicx26tnOYdNS
LygkI6sSLY+y38llDWQfPmOi6hiD3Fgj//Z2FGvvTSsfzMfuB6h4BcxZyg6jFu8U
VFSDaoZoh6f6fJHC65ALylfDtgoArmhBHb7EqvaCiHZG+ON8YLWlRu8CfQSlePCZ
qO5+ldV/c4mmzdmZ9ByD/HYtixlDvmo15btC4QIDAQABAoIBAQCNjskmj/kqDf6q
DAVOkSjFpVnOnaSZcjwKTFNhbfhz6yUf9Kx3tyAsMsnNY/AlfKyWlDVAjiDQyw6W
w9xDp3KB14wZosJZvL2npA89FqNj7VHKZWt/Bofu9y1S10twHrbb8qJAX3b/2WOs
v5wD395X8LW6vp/9N4mKTxwD3Z9d25kM96OnYeN7m3U39qyPax+Zm4ckP6BYt92u
0pRyo8zfggpwZ9nTqVi/QRdmbKLcKQX0w0qhFXNT+Qih0lV+bg7X/gLV56V71L/Z
Gi5g0WEusuZobkGwz6ZLIA/Mb9VsxQN2dn4P8yFeMrqE1Cq1SF0tnrmG3K88MtIL
WKh0v++hAoGBAObJmh434pFvFMUK3bF9On0cagRkOAN/S5MsLCvCx92phIcv2PHj
HQ6t/qjSs67bbt8bXAFfI0dc8RAHKfllfyzSnG1VJPPrNYGt5lohsubXj79e5yCE
nNfyCgIalhsPaU1isMS6A2SRFOYC2DGGWrjwDAJ7D9uVYPSTFIJf9U33AoGBANg6
n8n7dYnwtYnV/ZbNy/luGTab/sIZd9SjWL+1leA4j9NAjWFGTJuSJ6++2BGwdt+U
EyRRwTKJBR6FmNw1DA9flbT8WZvVXGpxq/2lgmj+BkyXDTlOs4/vFo6qZCsi7rJZ
q7CUSR7Uv8cknnC1xXBeJ0ZOSWmfeM2FuzzEVx/nAoGAPlVzV/gWLNvIy1Oc/0Ro
mMQk6ly7OE6Ydf63S3aHjxorHmxQ9zw1a/KY1rZaK78+2enE1uYAXFUBEygsMR01
1UBpWapmma8Mu6LhlbSGYo0UWUbjvEe1KSpJg7n0kwh9k5Y1Ul0/b1k0QM/B3taA
CHVleQIODWvyTPywzGnoko8CgYAaZr4P6XYmi49+7vPxr9sR75GAi3eFt5pcCj3R
2pgcMovNHXqlv/GmpSfXW5QCROuezPOxYZIXW4eWfHV3nbqFPgSHfrBqSJjqHksD
w2uy6iFWxLLGpodiMa0tajykz7UZbb166AAtlYh9BkmaNXeQCs781J4+GUyku4lr
GAYTTQKBgDphUeUQqTiZMOL6Ebe2grDeggHgMBzWb+rldRFMEELCcUUSMckCC91L
4RjcYi4Kd+vMlkBS02HKrxLnk/PSfLZuFIiwXQh+d57fuI0eB9OXjDLzVqdf7jqg
cCeebZEp8Ned7Pel3lYYuwnwvU/0ESF6tb94+zHYWcv3DV2tGZfo
-----END RSA PRIVATE KEY-----
)EOF";

WiFiClientSecure espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

// Função para configurar o WiFi
void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

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
    if (client.connect(thing_name))
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

void setup()
{
  Serial.begin(115200);
  dht.begin();
  pinMode(LDRPIN, INPUT);
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);

  setup_wifi();

  // Carregar certificados diretamente no código
  espClient.setCACert(ca_cert);
  espClient.setCertificate(client_cert);
  espClient.setPrivateKey(client_key);

  client.setServer(aws_endpoint, aws_port);
  client.setCallback(callback);

  reconnect();
}

void publishAws()
{

  String payload = "{\"state\": {\"reported\": {";
  payload += "\"temperature\": ";
  payload += t;
  payload += ", \"humidity\": ";
  payload += h;
  payload += ", \"light\": ";
  payload += lightLevel;
  payload += ", \"soilMoisture\": ";
  payload += percentageSoilMoisture;
  payload += "}}}";

  Serial.print("Enviando payload para o tópico ");
  Serial.print(aws_topic);
  Serial.print(": ");
  Serial.println(payload);

  if (client.publish(aws_topic, payload.c_str()))
  {
    Serial.println("Mensagem publicada com sucesso");
    last_pub_aws = millis();
  }
  else
  {
    Serial.println("Falha ao publicar mensagem");
  }

  if (soilMoisture < 500)
  {
    digitalWrite(WATER_PUMP_PIN, HIGH);
  }
  else
  {
    digitalWrite(WATER_PUMP_PIN, LOW);
  }
}
void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  h = dht.readHumidity();
  t = dht.readTemperature();
  lightLevel = analogRead(LDRPIN);
  soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  percentageSoilMoisture = map(soilMoisture, wet, dry, 100, 0);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.println(" C");
  Serial.print("Light: ");
  Serial.print(lightLevel);
  Serial.println(" lx");
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisture);
  Serial.println(" ");
  Serial.print("Soil Moisture: ");
  Serial.print(percentageSoilMoisture);
  Serial.println(" %");

  if (isnan(h) || isnan(t))
  {
    Serial.println("Falha ao ler do sensor DHT!");
    delay(3000);
    return;
  }

  if (millis() - last_pub_aws >= interval_pub || last_pub_aws == 0 )
  {
    publishAws();
  }
  delay(5000);
}

