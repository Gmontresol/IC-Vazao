#include <WiFi.h>
#include "ThingSpeak.h"

// ==========================================
// CONFIGURAÇÕES DE REDE E THINGSPEAK
// ==========================================
const char* ssid = "Rede_Wifi";       // Substitua pelo nome da sua rede
const char* password = "Senha_Wifi";   // Substitua pela senha da sua rede

unsigned long myChannelNumber = 3465873;      // Substitua pelo seu Channel ID (número)
const char * myWriteAPIKey = "40S9VR89O1068B9A";   // Substitua pela sua Write API Key (entre aspas)

WiFiClient client;

// ==========================================
// CONFIGURAÇÕES DO SENSOR
// ==========================================
const int pinSensorVazao = 32; 

volatile int contadorPulsos = 0; 
unsigned long tempoAnteriorSensor = 0;
unsigned long tempoAnteriorThingSpeak = 0; // Novo timer exclusivo para o ThingSpeak

float vazaoLPM = 0.0;            
float volumeTotal = 0.0;        

// Função de interrupção
void IRAM_ATTR contarPulsos() {
  contadorPulsos++;
}

void setup() {
  Serial.begin(115200);

  // 1. Configuração do Wi-Fi
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado com sucesso!");
  
  // 2. Inicializa o cliente ThingSpeak
  ThingSpeak.begin(client);

  // 3. Configuração do Sensor
  pinMode(pinSensorVazao, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinSensorVazao), contarPulsos, RISING);

  Serial.println("Leitura do Sensor de Vazao Iniciada!");
  Serial.println("-------------------------------------------");
}

void loop() {
  
  // ==========================================
  // BLOCO 1: LEITURA DO SENSOR (A CADA 1 SEGUNDO)
  // ==========================================
  if (millis() - tempoAnteriorSensor >= 1000) {
    detachInterrupt(digitalPinToInterrupt(pinSensorVazao));

    vazaoLPM = contadorPulsos / 7.5;
    volumeTotal += (vazaoLPM / 60.0);

    Serial.print("Vazao: ");
    Serial.print(vazaoLPM);
    Serial.print(" L/min \t | Volume Total: ");
    Serial.print(volumeTotal);
    Serial.println(" L");

    contadorPulsos = 0;
    tempoAnteriorSensor = millis();

    attachInterrupt(digitalPinToInterrupt(pinSensorVazao), contarPulsos, RISING);
  }

  // ==========================================
  // BLOCO 2: ENVIO PARA O THINGSPEAK (A CADA 20 SEGUNDOS)
  // ==========================================
  if (millis() - tempoAnteriorThingSpeak >= 20000) {
    
    // Configura os campos com as variáveis do sensor
    ThingSpeak.setField(1, vazaoLPM);
    ThingSpeak.setField(2, volumeTotal);

    // Envia os dados e captura o código de retorno
    int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    
    if (httpCode == 200) {
      Serial.println("---> Dados enviados ao ThingSpeak com sucesso!");
    } else {
      Serial.print("---> Erro ao enviar dados. Codigo HTTP: ");
      Serial.println(httpCode);
    }
    
    // Atualiza o tempo do último envio
    tempoAnteriorThingSpeak = millis();
  }
}
