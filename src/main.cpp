#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

// Definições de pinos
#define DHTPIN 4      // Pino do sensor DHT22 (temperatura e umidade)
#define DHTTYPE DHT22 // Tipo do sensor DHT
#define LDRPIN 34     // Pino do sensor de luminosidade
#define SOIL_MOISTURE_PIN 35 // Pino do sensor de umidade do solo
#define WATER_PUMP_PIN 5 // Pino para controlar a bomba de água

// Configurações WiFi e MQTT
const char* ssid = "AlexssJeff";
const char* password = "05122014ja";
const char* mqtt_server = "SEU_SERVIDOR_MQTT";
const char* mqtt_topic = "sua/topico/planta";

// Inicialização das bibliotecas
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

// Função para conectar ao WiFi
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

// Setup inicial
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LDRPIN, INPUT);
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

// Função principal de loop
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int lightLevel = analogRead(LDRPIN);
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);

  if (isnan(h) || isnan(t)) {
    Serial.println("Falha ao ler do sensor DHT!");
    return;
  }

  String payload = "{";
  payload += "\"temperature\": ";
  payload += t;
  payload += ", \"humidity\": ";
  payload += h;
  payload += ", \"light\": ";
  payload += lightLevel;
  payload += ", \"soilMoisture\": ";
  payload += soilMoisture;
  payload += "}";

  Serial.print("Enviando payload: ");
  Serial.println(payload);

  client.publish(mqtt_topic, payload.cstr());

  // Controle da bomba de água com base na umidade do solo
  if (soilMoisture < 500) { // Ajuste o valor conforme necessário
    digitalWrite(WATER_PUMP_PIN, HIGH); // Liga a bomba
  } else {
    digitalWrite(WATER_PUMP_PIN, LOW); // Desliga a bomba
  }

  delay(60000); // Envia dados a cada minuto
}
