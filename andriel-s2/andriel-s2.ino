#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>


// PINOS
const byte LED_PIN = 21;

// Sensor Ultrassônico 1
const byte TRIGGER_PIN1 = 13;
const byte ECHO_PIN1    = 12;

// Sensor Ultrassônico 2
const byte TRIGGER_PIN2 = 5;
const byte ECHO_PIN2    = 18;


// OBJETOS MQTT 
WiFiClientSecure client;
PubSubClient mqtt(client);

// DADOS DO MQTT 
const char* MQTT_SERVER = "ce8f972bcadf4b7a99acd3e2f2fdf20e.s1.eu.hivemq.cloud";
const int   MQTT_PORT   = 8883;
const char* MQTT_USER   = "s2_andriel";
const char* MQTT_PASS   = "S2andriel";

// WI-FI
const char* SSID      = "FIESC_IOT_EDU";
const char* WIFI_PASS = "8120gv08";

// TÓPICOS MQTT

const char* TOPICO_DISTANCIA1     = "S2/PROXIMIDADE";   // <-- Novo tópico
const char* TOPICO_DISTANCIA2     = "S2/DISTANCIA";   // <-- Novo tópico
const char* TOPICO_LED =  "S1/ILUMINACAO";
// ULTRASSÔNICO
long lerDistancia(int TRIGGER_PIN1, int ECHO_PIN1) {
  digitalWrite(TRIGGER_PIN1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN1, LOW);

  long duracao = pulseIn(ECHO_PIN1, HIGH);
  long distancia = duracao * 349.24 / 2 / 10000;  // cm

  return distancia;
}

//

long lerDistancia2(int TRIGGER_PIN2, int ECHO_PIN2) {
  digitalWrite(TRIGGER_PIN2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN2, LOW);

  long duracao = pulseIn(ECHO_PIN2, HIGH);
  long distancia = duracao * 349.24 / 2 / 10000;  // cm

  return distancia;
}

// CALLBACK MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem;

  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("Recebido em ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(mensagem);

  if(strcmp(topic, TOPICO_LED) == 0){
    if (mensagem == "ACENDER") {
      digitalWrite(LED_PIN, HIGH);
    } 
    else if (mensagem == "APAGAR") {
      digitalWrite(LED_PIN, LOW);
    }
  }
}

// CONEXÃO MQTT
void conectaMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao HiveMQ... ");

    String clientId = "ESP32-S1-";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("Conectado!");

      mqtt.subscribe(TOPICO_LED);
    } else {
      Serial.println("Falhou. Tentando novamente...");
      delay(1000);
    }
  }
}

// SETUP
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  pinMode(TRIGGER_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  pinMode(TRIGGER_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);

  // Conecta ao Wi-Fi
  Serial.print("Conectando ao Wi-Fi ");
  WiFi.begin(SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" Conectado!");

  client.setInsecure();
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.subscribe -> topico da iluminação
  mqtt.setCallback(callback);
  conectaMQTT();^

}

// LOOP
void loop() {

  if (!mqtt.connected()) {
    conectaMQTT();
  }
  mqtt.loop();

  // ULTRASSÔNICO 1
  long distancia = lerDistancia(TRIGGER_PIN1, ECHO_PIN1);
  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");
  if (distancia < 10) {
    Serial.println("Objeto próximo!");
  }
  mqtt.publish(TOPICO_DISTANCIA, String(distancia).c_str());

  delay(5000);

   long distancia = lerDistancia2(TRIGGER_PIN2, ECHO_PIN2);
  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");
  if (distancia < 10) {
    Serial.println("Objeto próximo!");
  }
  mqtt.publish(TOPICO_DISTANCIA, String(distancia).c_str());

  delay(5000);
}