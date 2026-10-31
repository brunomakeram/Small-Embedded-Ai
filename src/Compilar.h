#pragma once
#include "Neuronios.h"
#include "Adam.h"

class Compilar {
public:
    Neuronios neuronios;
    Adam otimizador;

    // Padrões: taxa de aprendizado 0.05 (ajustada para este projeto); beta1, beta2 e eps são os do paper do Adam
    void configurar(float taxaAprendizado = 0.05f, float beta1 = 0.9f, float beta2 = 0.999f, float eps = 1e-7f) {
        neuronios.inicializar();
        otimizador.inicializar(taxaAprendizado, beta1, beta2, eps);
    }
};