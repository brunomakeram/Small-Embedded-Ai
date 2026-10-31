#pragma once
#include <Arduino.h>
#include "Prever.h"

class Gradientes {
public:
    void processarAmostraLote(float carga, float distancia, float gastoReal, const float *pesos, float vies, float *gradientes, float &somaErroQuadrado, Prever &previsor) {
        float gastoPrevisto = previsor.calcular(carga, distancia, pesos, vies);
        float erro = gastoPrevisto - gastoReal;
        
        somaErroQuadrado += erro * erro;
        gradientes[0] += 2.0f * erro * carga;
        gradientes[1] += 2.0f * erro * distancia;
        gradientes[2] += 2.0f * erro;
    }
};