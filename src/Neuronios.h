#pragma once
#include <Arduino.h>
#include <math.h>

class Neuronios {
public:
    float pesos[2];
    float vies;

    float gerarNumeroAleatorio() {
        return (float)((double)esp_random() / 4294967295.0);
    }

    void inicializar() {
        const float limite_glorot = sqrtf(6.0f / (2 + 1));
        pesos[0] = (gerarNumeroAleatorio() * 2.0f - 1.0f) * limite_glorot;
        pesos[1] = (gerarNumeroAleatorio() * 2.0f - 1.0f) * limite_glorot;
        vies = 0.0f;
    }
};