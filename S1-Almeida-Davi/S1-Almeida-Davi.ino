#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "DHT.h"

//  CONFIGURAÇÕES DE REDE 
const char* SSID = "FIESC_IOT_EDU";
const char* WIFI_PASS = "8120gv08";

// BROKER MQTT (HiveMQ) 
const char* MQTT_SERVER = "ce8f972bcadf4b7a99acd3e2f2fdf20e.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USER = "s1_davi_rafael";
const char* MQTT_PASS = "S1davirafael";

// TÓPICOS MQTT
const char* TOPICO_PRESENCA     = "S1/presenca";
const char* TOPICO_TEMPERATURA  = "S1/temperatura";
const char* TOPICO_UMIDADE      = "S1/umidade";
const char* TOPICO_LED          = "S1/iluminacao";

//  PINOS 
#define LED_PIN 2
#define TRIG_PIN 9
#define ECHO_PIN 10
#define DHT_PIN 4
#define DHTTYPE DHT11

//  OBJETOS 
WiFiClientSecure client;
PubSubClient mqtt(client);
DHT dht(DHT_PIN, DHTTYPE);

//  VARIÁVEIS 
long duracao;
float distancia;
bool detectou = false;
unsigned long ultimoEnvio = 0;

void conectarWiFi() {
  Serial.print("Conectando ao WiFi ");
  Serial.print(SSID);
  WiFi.begin(SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" conectado!");
}

void conectarMQTT() {
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);
  client.setInsecure();

  Serial.print("Conectando ao HiveMQ...");
  while (!mqtt.connected()) {
    String clientId = "ESP32-S1-";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println(" conectado!");
    } else {
      Serial.print(".");
      delay(1000);
    }
  }
  mqtt.subscribe(TOPICO_LED);
  Serial.println("Assinado no tópico de LED.");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem;
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("Mensagem recebida em ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(mensagem);

  if (String(topic) == TOPICO_LED) {
    if (mensagem.equalsIgnoreCase("acender")) {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ACESO!");
    } else if (mensagem.equalsIgnoreCase("apagar")) {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED APAGADO!");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  conectarWiFi();
  conectarMQTT();
  dht.begin();
}

void loop() {
  if (!mqtt.connected()) {
    conectarMQTT();
  }
  mqtt.loop();

  //  SENSOR ULTRASSÔNICO 
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duracao = pulseIn(ECHO_PIN, HIGH);
  distancia = duracao * 0.0343 / 2;

  if (distancia > 0 && distancia < 10 && !detectou) {
    detectou = true;
    digitalWrite(LED_PIN, HIGH);
    mqtt.publish(TOPICO_PRESENCA, "Objeto detectado!");
    Serial.println("Objeto detectado!");
  } 
  else if (distancia >= 10 && detectou) {
    detectou = false;
    digitalWrite(LED_PIN, LOW);
    mqtt.publish(TOPICO_PRESENCA, "Área livre");
    Serial.println("Área livre");
  }

  //  LEITURA DHT11 (a cada 10s) 
  if (millis() - ultimoEnvio > 10000) {
    float umidade = dht.readHumidity();
    float temperatura = dht.readTemperature();

    if (isnan(umidade) || isnan(temperatura)) {
      Serial.println("Falha ao ler o DHT11");
    } else {
      char msgTemp[10];
      char msgUmi[10];
      dtostrf(temperatura, 4, 2, msgTemp);
      dtostrf(umidade, 4, 2, msgUmi);

      mqtt.publish(TOPICO_TEMPERATURA, msgTemp);
      mqtt.publish(TOPICO_UMIDADE, msgUmi);

      Serial.print("Temperatura: ");
      Serial.print(msgTemp);
      Serial.print(" °C | Umidade: ");
      Serial.print(msgUmi);
      Serial.println(" %");
    }
    ultimoEnvio = millis();
  }

  delay(300);
}
