#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <FS.h>
#include <SPIFFS.h>

// DefiniÃ§Ãµes de pinos
#define DHTPIN 4
#define DHTTYPE DHT22
#define LDRPIN 34
#define SOIL_MOISTURE_PIN 35
#define WATER_PUMP_PIN 5
// ConfiguraÃ§Ãµes WiFi e AWS IoT
const char* ssid = "AlexssJeff";
const char* password = "05122014ja";
const char* aws_endpoint = "a25uug4xbk339z-ats.iot.us-east-1.amazonaws.com"; // Substitua pelo seu endpoint AWS IoT
const int aws_port = 8883;
const char* thing_name = "SmartPlantPot"; // Nome do dispositivo
const char* aws_topic = "$aws/things/SmartPlantPot/shadow/update";


WiFiClientSecure espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

// Função para ler o conteúdo de um arquivo
String readFile(fs::FS &fs, const char * path) {
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.printf("Falha ao abrir o arquivo: %s\n", path);
    return String();
  }
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  return fileContent;
}

// Função para configurar o WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// Função de callback do MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.print(topic);
  Serial.print(". Mensagem: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Função para reconectar ao MQTT se a conexão cair
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LDRPIN, INPUT);
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);

  setup_wifi();

  // Inicializar SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Erro ao montar SPIFFS");
    return;
  }

  // Ler certificados dos arquivos
  String ca_cert = readFile(SPIFFS, "/ca_cert.pem");
  String client_cert = readFile(SPIFFS, "/client_cert.pem");
  String client_key = readFile(SPIFFS, "/client_key.pem");

  espClient.setCACert(ca_cert.c_str());
  espClient.setCertificate(client_cert.c_str());
  espClient.setPrivateKey(client_key.c_str());

  client.setServer(aws_endpoint, aws_port);
  client.setCallback(callback);

  reconnect();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int lightLevel = analogRead(LDRPIN);
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);


h = 9.0;
  t = 8.0;
  lightLevel = 100;
  soilMoisture = 8;


  if (isnan(h) || isnan(t)) {
    Serial.println("Falha ao ler do sensor DHT!");
    return;
  }

  String payload = "{\"state\": {\"reported\": {";
  payload += "\"temperature\": ";
  payload += t;
  payload += ", \"humidity\": ";
  payload += h;
  payload += ", \"light\": ";
  payload += lightLevel;
  payload += ", \"soilMoisture\": ";
  payload += soilMoisture;
  payload += "}}}";

  Serial.print("Enviando payload para o tópico ");
  Serial.print(aws_topic);
  Serial.print(": ");
  Serial.println(payload);

  if (client.publish(aws_topic, payload.c_str())) {
    Serial.println("Mensagem publicada com sucesso");
  } else {
    Serial.println("Falha ao publicar mensagem");
  }

  if (soilMoisture < 500) { // Ajuste o valor conforme necessário
    digitalWrite(WATER_PUMP_PIN, HIGH); // Liga a bomba
  } else {
    digitalWrite(WATER_PUMP_PIN, LOW); // Desliga a bomba
  }

  delay(60000); // Envia dados a cada minuto
}