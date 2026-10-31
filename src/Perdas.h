#pragma once
#include <Arduino.h>

class Perdas {
public:
    float calcularMSE(float somaErrosQuadrados, int totalAmostras) {
        if (totalAmostras <= 0) return 0.0f;
        return somaErrosQuadrados / (float)totalAmostras;
    }
};