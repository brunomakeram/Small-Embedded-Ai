#pragma once
#include <Arduino.h>

// Agrupa o cálculo de uma previsão linear a partir de duas características.
namespace Prever {
class Prever {
public:
    // Combina carga e distância com seus pesos e adiciona o viés treinado.
    // pesos aponta para os coeficientes na mesma ordem das entradas.
    float calcular(float carga, float distancia, const float *pesos, float vies) {
        // Cada peso multiplica sua característica correspondente; o viés desloca o resultado.
        return pesos[0] * carga + pesos[1] * distancia + vies;
    }
};
}