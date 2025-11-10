#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

WiFiClient client;
PubSubClient mqtt(client);

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

const String URL   = "ce8f972bcadf4b7a99acd3e2f2fdf20e.s1.eu.hivemq.cloud";
const int PORT     = 8883;
const String USR   = "s3_caio";
const String broker_PASS  = "S3caiopoleza";

const int LED_ILUM = 2;
const int LED_R = 25;
const int LED_G = 26;
const int LED_B = 27;
const int PRESENCA_PIN = 34;
const int SERVO1_PIN = 13;
const int SERVO2_PIN = 12;
const byte TRIGGER_PIN = 5;
const byte ECHO_PIN = 18;

Servo servo1;
Servo servo2;

const String MyTopic_Presenca = "Ferrorama/S3/Presenca";
const String TOPICO_LED = "S1/Iluminacao";
const String Topic_Servo1 = "Ferrorama/S3/Servo1";
const String Topic_Servo2 = "Ferrorama/S3/Servo2";
const String Topic_Distancia = "Ferrorama/S3/Ultrassom";

bool presencaAnterior = false;
unsigned long ultimoEnvio = 0;
const unsigned long intervalo = 2000; 

void setRGB(int r, int g, int b) {
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}

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

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem;
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("Recebido de ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(mensagem);

  if (String(topic) == TOPICO_LED) {
    if (mensagem == "1") {
      digitalWrite(LED_ILUM, HIGH);
      Serial.println("Luz acesa");
    } else {
      digitalWrite(LED_ILUM, LOW);
      Serial.println("Luz apagada");
    }
  }

  if (String(topic) == Topic_Servo1) {
    int angulo = mensagem.toInt();
    servo1.write(angulo);
    Serial.print("Servo1 -> ");
    Serial.println(angulo);
  }

  if (String(topic) == Topic_Servo2) {
    int angulo = mensagem.toInt();
    servo2.write(angulo);
    Serial.print("Servo2 -> ");
    Serial.println(angulo);
  }
}

void setup() {
  Serial.begin(115200);

  // Configura pinos
  pinMode(LED_ILUM, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(PRESENCA_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  digitalWrite(LED_ILUM, LOW);
  setRGB(0, 0, 255);

  Serial.println("Conectando ao WiFi...");
  WiFi.begin(SSID.c_str(), PASS.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi conectado!");

  mqtt.setServer(URL.c_str(), PORT);
  mqtt.setCallback(callback);

  Serial.println("Conectando ao Broker...");
  while (!mqtt.connected()) {
    String clientId = "s3_caio_" + String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str(), USR.c_str(), broker_PASS.c_str())) {
      Serial.println("Conectado ao Broker!");
      setRGB(0, 255, 0); 
    } else {
      Serial.print(".");
      delay(500);
    }
  }

  mqtt.subscribe(TOPICO_LED.c_str());
  mqtt.subscribe(Topic_Servo1.c_str());
  mqtt.subscribe(Topic_Servo2.c_str());
}

void loop() {
  mqtt.loop();

  bool presencaAtual = digitalRead(PRESENCA_PIN);
  if (presencaAtual != presencaAnterior) {
    presencaAnterior = presencaAtual;
    String msg = presencaAtual ? "1" : "0";
    mqtt.publish(MyTopic_Presenca.c_str(), msg.c_str());
    Serial.print("Publicando presença: ");
    Serial.println(msg);
  }

  unsigned long agora = millis();
  if (agora - ultimoEnvio > intervalo) {
    ultimoEnvio = agora;
    long distancia = lerDistancia();
    Serial.print("Distância: ");
    Serial.print(distancia);
    Serial.println(" cm");

    String distMsg = String(distancia);
    mqtt.publish(Topic_Distancia.c_str(), distMsg.c_str());

    if (distancia < 10) {
      mqtt.publish(MyTopic_Presenca.c_str(), "1");
      Serial.println("Objeto próximo! Publicando presença 1.");
    } else {
      mqtt.publish(MyTopic_Presenca.c_str(), "0");
    }
  }
}
