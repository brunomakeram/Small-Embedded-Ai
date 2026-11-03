#pragma once
#include "../Neuronios/Neuronios.h"
#include "../Adam/Adam.h"

// Reúne a camada de neurônios e o otimizador usados pelo fluxo de treinamento.
namespace Compilar {
class Compilar {
public:
    // Guarda os pesos e o viés da camada que será usada pelo modelo.
    Neuronios::Neuronios neuronios;
    // Guarda os parâmetros e os momentos do otimizador Adam.
    Adam::Adam otimizador;

    // Padrões: taxa de aprendizado 0.05 (ajustada para este projeto); beta1, beta2 e eps são os do paper do Adam
    // Inicializa a camada neural e o otimizador com os parâmetros recebidos ou seus valores padrão.
    void configurar(float taxaAprendizado = 0.05f, float beta1 = 0.9f, float beta2 = 0.999f, float eps = 1e-7f) {
        // Prepara os pesos e vieses antes de iniciar as atualizações do treinamento.
        neuronios.inicializar();
        // Repassa a taxa de aprendizado e os fatores de Adam para inicializar o estado do otimizador.
        otimizador.inicializar(taxaAprendizado, beta1, beta2, eps);
    }
};
}