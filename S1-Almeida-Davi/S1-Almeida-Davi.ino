#include <WiFiClientSecure.h>
#include <PubSubClient.h>

WiFiClientSecure client;
PubSubClient mqtt(client);

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

const String URL   = "ce8f972bcadf4b7a99acd3e2f2fdf20e.s1.eu.hivemq.cloud
";
const int PORT     = 8883;
const String broker_USR   = "s1_davi_rafael";
const String broker_PASS  = "S1davirafael";
const String MyTopic = "Almeida";
const String OtherTopic = "s1_davi_rafael";

void setup() {
  Serial.begin(115200);

  Serial.println("Conectando ao WiFi");
  WiFi.begin(SSID.c_str(), PASS.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  Serial.println("\nConectado com sucesso!");
  client.setInsecure();
  Serial.println("Conectando ao Broker");

  mqtt.setServer(URL.c_str(), PORT);
  mqtt.setCallback(callback);

  while (!mqtt.connected()) {
    String ID = "s1_";
    ID += String(random(0xffff), HEX);
    mqtt.connect(ID.c_str(), USR.c_str(), broker_PASS.c_str());
    Serial.print(".");
    delay(200);
  }

  mqtt.subscribe(MyTopic.c_str());
  Serial.println("\nConectado com sucesso ao broker !");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN , LOW);
}

void loop() {
  // if (!mqtt.connected()) {
  //   while (!mqtt.connected()) {
  //     String ID = "s3_";
  //     ID += String(random(0xffff), HEX);
  //     mqtt.connect(ID.c_str(), USR.c_str(), broker_PASS.c_str());
  //     delay(200);
  //   }
  //   mqtt.subscribe(MyTopic.c_str());
  // }


  String mensagem = "";
  if (Serial.available() > 0) {
    mensagem += Serial.readStringUntil('\n');
    mqtt.publish(OtherTopic.c_str(), mensagem.c_str());
  }

  mqtt.loop();
  delay(1000);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.println("Recebido: ");
  Serial.println(mensagem);

  if (mensagem.equalsIgnoreCase("acender")) {
    digitalWrite(LED_BUILTIN , HIGH);
    Serial.println("LED ACESO!");
  } 
  else if (mensagem.equalsIgnoreCase("apagar")) {
    digitalWrite(LED_BUILTIN , LOW);
    Serial.println("LED APAGADO!");
  }
  else {
    Serial.println("Mensagem não reconhecida.");
  }
}
