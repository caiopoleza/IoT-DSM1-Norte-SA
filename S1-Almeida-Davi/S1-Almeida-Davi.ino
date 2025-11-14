#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

// PINOS
const byte LDR_PIN = 34;
const byte LED_PIN = 19;    
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// OBJETOS MQTT
WiFiClientSecure client;
PubSubClient mqtt(client);

// DADOS DO MQTT (ATUALIZADOS) 
const char* MQTT_SERVER = "ce8f972bcadf4b7a99acd3e2f2fdf20e.s1.eu.hivemq.cloud";
const int   MQTT_PORT   = 8883;
const char* MQTT_USER   = "s1_davi_rafael";
const char* MQTT_PASS   = "S1davirafael";

// WI-FI
const char* SSID      = "FIESC_IOT_EDU";
const char* WIFI_PASS = "8120gv08";

// TÓPICOS MQTT
const char* TOPICO_UMIDADE     = "S1/UMIDADE";
const char* TOPICO_TEMPERATURA = "S1/TEMPERATURA";
const char* TOPICO_LED         = "S1/ILUMINACAO";

// CALLBACK: recebe mensagens do broker
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
    } else if (mensagem == "APAGAR") {
      digitalWrite(LED_PIN, LOW);
    }
  }
}

// CONECTA AO MQTT
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
  dht.begin();

  // ----------- Conecta ao Wi-Fi -----------
  Serial.print("Conectando ao Wi-Fi ");
  WiFi.begin(SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" Conectado!");

  client.setInsecure();
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);
  conectaMQTT();
}

// LOOP PRINCIPAL
void loop() {

  if (!mqtt.connected()) {
    conectaMQTT();
  }
  mqtt.loop();

  // LER LDR
  int leituraLDR = analogRead(LDR_PIN);
  Serial.print("LDR: ");
  Serial.println(leituraLDR);

  if (leituraLDR > 3500) {
    Serial.println("ACENDER");
    digitalWrite(LED_PIN, HIGH);
    mqtt.publish(TOPICO_LED, "ACENDER");
  } else {
    Serial.println("APAGAR");
    digitalWrite(LED_PIN, LOW);
    mqtt.publish(TOPICO_LED, "APAGAR");
  }

  // (DHT11)
  
  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.print("Umidade: ");
  Serial.print(umidade);
  Serial.println(" %");

  mqtt.publish(TOPICO_TEMPERATURA, String(temperatura).c_str());
  mqtt.publish(TOPICO_UMIDADE, String(umidade).c_str());

  Serial.println("--------------------");

  delay(1500);
}
