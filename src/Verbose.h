#pragma once
#include <Arduino.h>

class Verbose {
public:
    float lerNumero(const char *mensagem) {
        Serial.print(mensagem);
        while (Serial.available()) Serial.read();
        while (!Serial.available()) delay(10);
        
        String textoDigitado = Serial.readStringUntil('\n');
        textoDigitado.trim();
        Serial.println(textoDigitado);
        return textoDigitado.toFloat();
    }

    void mostrarResultados(float carga, float distancia, float perdas, const float *pesos, float vies, float gasto_previsto) {
        Serial.println("\n===== RESULTADOS DO TREINAMENTO =====");
        Serial.printf("Carga inputada: %.2f kg\n", carga);
        Serial.printf("Distância inputada: %.2f km\n", distancia);
        Serial.printf("Pesos Ajustados (w1, w2): [%.5f, %.5f]\n", pesos[0], pesos[1]);
        Serial.printf("Viés Ajustado (b): %.5f\n", vies);
        Serial.printf("MSE (Perda Final): %.5f\n", perdas);
        Serial.printf("-> Previsão de Gasto de Bateria: %.2f%%\n", gasto_previsto);
        Serial.println("=====================================\n");
    }
};