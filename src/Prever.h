#pragma once
#include <Arduino.h>

class Prever {
public:
    float calcular(float carga, float distancia, const float *pesos, float vies) {
        return pesos[0] * carga + pesos[1] * distancia + vies;
    }
};