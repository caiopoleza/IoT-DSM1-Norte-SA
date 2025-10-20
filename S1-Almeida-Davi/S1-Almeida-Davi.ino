#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient client;
PubSubClient mqtt(client);

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

const String URL      = "test.mosquitto.org";
const int PORT        = 1883;
const String USR  = "";
const String broker_PASS     = "";
const String Topic    = "DSM1";

void setup() {
  Serial.begin(115200);
  Serial.println("Conectando ao Wifi");
  WiFi.begin(SSID,PASS);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectando co sucesso!");
  Serial.println("\nConectado ao Broker");
  mqtt.setServer(URL.c_str(),PORT);
  while(!mqtt.connected()){
    String ID = "S1_";
    ID += String(random(0xffff),HEX);
    mqtt.connect(ID.c_str(),USR.c_str(),broker_PASS.c_str());
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado com sucesso ao broker !");
}

void loop() {
  String mensagem = "Rafael Almeida";
  mensagem += "4x0 no grêmio";

  mqtt.publish(Topic.c_str(),mensagem.c_str());

  mqtt.loop();
  delay(10000);

}
