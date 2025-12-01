#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";
const char* URL   = "ce8f972bcadf4b7a99acd3e2f2fdf20e.s1.eu.hivemq.cloud";
const int PORT     = 8883;
const char* USR   = "tremarthur";
const char* broker_PASS = "Tremarthur4";

const char* TREM_COMANDO_TOPIC = "trem/comando";  // Tópico que o trem ASSINA (recebe comandos)
const char* TREM_STATUS_TOPIC = "trem/status";    // Tópico que o trem PUBLICA (envia status)

const int ENA_PIN = 2;   // Pino para controle de velocidade (PWM)
const int IN1_PIN = 4;   // Pino para controle de direção 1
const int IN2_PIN = 16;  // Pino para controle de direção 2

#define FREQ 5000       // Frequência do sinal PWM
#define RESOLUTION 10   // Resolução de 10 bits (valores de 0 a 1023)
#define PWM_CHANNEL 0   // Canal PWM a ser utilizado

void moverTrem(int velocidade, bool direcaoFrente) {
  int dutyCycle = abs(velocidade);
  
  // Define a velocidade usando o PWM do ESP32
  ledcWrite(PWM_CHANNEL, dutyCycle);

  if (velocidade == 0) {
    // PARAR: Pinos de direção em LOW
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    Serial.println("Trem PARADO.");
    mqtt.publish(TREM_STATUS_TOPIC, "PARADO");
  } else if (direcaoFrente) {
    // AVANÇAR
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
    Serial.print("Trem AVANÇANDO (Velocidade: "); Serial.print(dutyCycle); Serial.println(")");
    mqtt.publish(TREM_STATUS_TOPIC, "MOVENDO_AVANCAR");
  } else {
    // RECUAR
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, HIGH);
    Serial.print("Trem RECUANDO (Velocidade: "); Serial.print(dutyCycle); Serial.println(")");
    mqtt.publish(TREM_STATUS_TOPIC, "MOVENDO_RECUAR");
  }
}

// --- FUNÇÃO DE CALLBACK (RECEPÇÃO DE MENSAGENS) ---

void callback(char* topic, byte* payload, unsigned int length){
  String message = "";
  for(int i = 0; i < length; i++){
    message += (char)payload[i];
  }

  Serial.print("Mensagem recebida no tópico [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  if (strcmp(topic, TREM_COMANDO_TOPIC) == 0) {
    if (message == "AVANCAR") {
      moverTrem(800, true); 
    } else if (message == "PARAR") {
      moverTrem(0, true); 
    } else if (message == "RECUAR") {
      moverTrem(500, false);
    }
  }

  String mensagem_serial = "Recebido: " + message;
  Serial.println(mensagem_serial);
}

void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Tentando conexão MQTT...");
    String ID = "s4_trem_";
    ID += String(random(0xffff),HEX);
    
    // Conecta usando ID, usuário e senha do broker
    if (mqtt.connect(ID.c_str(),USR,broker_PASS)) {
      Serial.println("conectado!");
      // O Trem ASSINA o tópico de COMANDO para receber ordens
      mqtt.subscribe(TREM_COMANDO_TOPIC);
      Serial.print("Inscrito no tópico: ");
      Serial.println(TREM_COMANDO_TOPIC);
      mqtt.publish(TREM_STATUS_TOPIC, "ONLINE_PRONTO");

    } else {
      Serial.print("falhou, rc=");
      Serial.print(mqtt.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);

  // Configuração do PWM do ESP32
  ledcSetup(PWM_CHANNEL, FREQ, RESOLUTION);
  ledcAttachPin(ENA_PIN, PWM_CHANNEL);
  
  // Configuração dos pinos de direção
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  moverTrem(0, true); // Garante que o motor esteja parado no início

  // Conexão Wi-Fi
  Serial.println("Conectando ao WiFi");
  WiFi.begin(SSID,PASS);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado com sucesso!");
  
  // Configuração do MQTT e Conexão
  mqtt.setServer(URL, PORT);
  mqtt.setCallback(callback);

  reconnect();
}

void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  
  mqtt.loop(); 

  delay(10);
}