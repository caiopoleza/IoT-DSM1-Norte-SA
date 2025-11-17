#include <WiFi.h>               // Biblioteca para ESP32
#include <WiFiClientSecure.h>    // Biblioteca para cliente seguro (SSL)
#include <PubSubClient.h>        // Biblioteca para MQTT
#include <ESP32Servo.h>          // Biblioteca para controle de servos

// ----- OBJETOS -----
WiFiClientSecure client;   // Cliente SSL para conexão segura com o broker
PubSubClient mqtt(client);

// ----- WI-FI -----
const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

// ----- MQTT -----
const char* MQTT_SERVER = "ce8f972bcadf4b7a99acd3e2f2fdf20e.s1.eu.hivemq.cloud";
const int   MQTT_PORT   = 8883;  // Porta segura SSL
const char* MQTT_USER   = "s3_caio";
const char* MQTT_PASS   = "S3caiopoleza";

// ----- PINOS -----
const int LED_ILUM = 2;
const int LED_R = 25;
const int LED_G = 26;
const int LED_B = 27;

const int PRESENCA_PIN = 34;
const int SERVO1_PIN = 13;
const int SERVO2_PIN = 12;

const int TRIGGER_PIN = 5;
const int ECHO_PIN = 18;

// ----- OBJETOS SERVO -----
Servo servo1;
Servo servo2;

// ----- TOPICOS MQTT -----
const char* TOPICO_PRESENCA = "Ferrorama/S3/Presenca";
const char* TOPICO_LED      = "S1/Iluminacao";
const char* TOPICO_SERVO1   = "Ferrorama/S3/Servo1";
const char* TOPICO_SERVO2   = "Ferrorama/S3/Servo2";
const char* TOPICO_DISTANCIA = "S1/DISTANCIA";  // Distância recebida

// ----- VARIÁVEIS -----
bool presencaAnterior = false;
unsigned long ultimoEnvio = 0;
const unsigned long intervalo = 2000;

// ----- FUNÇÃO RGB -----
void setRGB(int r, int g, int b) {
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}

// ----- ULTRASSÔNICO -----
long lerDistancia() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH, 30000);
  long distancia = duracao * 0.0343 / 2;
  return distancia;
}

// ----- CALLBACK MQTT -----
void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.print("Recebido de ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msg);

  // Controle de LED
  if (String(topic) == TOPICO_LED) {
    digitalWrite(LED_ILUM, msg == "1" ? HIGH : LOW);
  }

  // Controle servo1 por MQTT
  if (String(topic) == TOPICO_SERVO1) {
    servo1.write(msg.toInt());
  }

  // Controle servo2 por MQTT
  if (String(topic) == TOPICO_SERVO2) {
    servo2.write(msg.toInt());
  }

  // DISTÂNCIA RECEBIDA DO OUTRO ESP32
  if (String(topic) == TOPICO_DISTANCIA) {
    long distancia = msg.toInt();
    Serial.print("DISTÂNCIA RECEBIDA: ");
    Serial.println(distancia);

    if (distancia == 10) {
      servo1.write(0);
      Serial.println("⚠ Servo1 LOW porque distância = 10 cm");
    }
  }
}

// ----- CONEXÃO COM O MQTT -----
void conectaMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao HiveMQ... ");
    
    String clientId = "ESP32-" + String(random(0xffff), HEX);  // Gerar ID único para o cliente
    if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("Conectado ao MQTT!");
      mqtt.subscribe(TOPICO_LED);
      mqtt.subscribe(TOPICO_SERVO1);
      mqtt.subscribe(TOPICO_SERVO2);
      mqtt.subscribe(TOPICO_DISTANCIA);  // Inscrevendo no tópico de distância
    } else {
      Serial.println("Falhou. Tentando novamente...");
      delay(1000);
    }
  }
}

// ----- SETUP -----
void setup() {
  Serial.begin(115200);

  pinMode(LED_ILUM, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  pinMode(PRESENCA_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  setRGB(0, 0, 255);  // Cor inicial do LED RGB

  // Conectar Wi-Fi
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi conectado!");

  client.setInsecure();  // Não valida o certificado SSL (necessário se o certificado não for verificado)
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);

  conectaMQTT();  // Conecta ao broker MQTT HiveMQ
}

// ----- LOOP -----
void loop() {
  mqtt.loop();

  bool presencaAtual = digitalRead(PRESENCA_PIN);
  if (presencaAtual != presencaAnterior) {
    presencaAnterior = presencaAtual;
    mqtt.publish(TOPICO_PRESENCA, presencaAtual ? "1" : "0");
  }

  unsigned long agora = millis();
  if (agora - ultimoEnvio > intervalo) {
    ultimoEnvio = agora;

    long distancia = lerDistancia();
    Serial.print("DISTÂNCIA: ");
    Serial.print(distancia);
    Serial.println(" cm");

    // Publica distância
    String msgDist = String

