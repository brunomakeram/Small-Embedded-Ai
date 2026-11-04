#pragma once

#include <vector>
#include "../Compilar/Compilar.h"
#include "../Epocas/Epocas.h"

// Oferece uma interface simples para treinar o modelo com matrizes numéricas.
namespace Aprendizagem
{
    // Encaminha as matrizes de treino ao executor de épocas.
    class Aprendizagem
    {
        // Os métodos públicos oferecem a interface de alto nível de treinamento.
    public:
        // Treina o modelo e devolve a perda média quadrática final.
        float treinarModelo(
            const std::vector<std::vector<float>> &entradas,
            const std::vector<std::vector<float>> &saidasEsperadas,
            Compilar::Compilar &modelo,
            int quantidadeEpocas,
            int tamanhoLote = 32)
        {
            Epocas::Epocas executorEpocas;
            return executorEpocas.executar(
                quantidadeEpocas,
                entradas,
                saidasEsperadas,
                modelo,
                tamanhoLote);
        }
    };
}
