#pragma once
#include <Arduino.h>

// Agrupa o cálculo da perda usada para acompanhar o erro médio quadrático.
namespace Perdas {
class Perdas {
public:
    // Divide a soma dos erros quadráticos pelo número de amostras e devolve zero para um lote vazio.
    float calcularMSE(float somaErrosQuadrados, int totalAmostras) {
        // Evita divisão por zero quando não há amostras para avaliar.
        if (totalAmostras <= 0) return 0.0f;
        // Converte o total para float para que a divisão preserve a parte fracionária da perda.
        return somaErrosQuadrados / (float)totalAmostras;
    }
};
}