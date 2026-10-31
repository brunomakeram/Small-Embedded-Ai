#include <Arduino.h>
#include <Modelo.h>

Armazenamento armazenamentoGlobal;
Modelo modeloMachineLearning;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    // Inicia e confere o arquivo CSV na memória do ESP32
    armazenamentoGlobal.montar("/Bateria.csv");
}

void loop() {
    // Roda toda a orquestração do modelo
    modeloMachineLearning.executar();
    
    Serial.println("\n================================");
    Serial.println("Pronto para uma nova consulta!");
    Serial.println("================================\n");
    
    delay(500);
}