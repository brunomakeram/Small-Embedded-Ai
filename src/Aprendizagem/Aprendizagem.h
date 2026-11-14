#pragma once

#include <vector>
#include "../Compilar/Compilar.h"
#include "../Epocas/Epocas.h"

// Namespace Seai reúne a interface de alto nível para treinamento de modelos....
namespace Seai
{
    // Encaminha as matrizes de treino ao executor de épocas....
    class Aprendizagem
    {
        // Os métodos públicos oferecem a interface de alto nível de treinamento....
    public:
        // Encaminha as matrizes e configurações ao executor de épocas e devolve a perda MSE final....
        // Cada linha de entradas deve corresponder à linha de saidasEsperadas; modelo deve estar configurado para essas dimensões....
        // quantidadeEpocas define as passagens pelo conjunto e tamanhoLote controla a quantidade de amostras por atualização....
        // Retorna a perda final calculada pelo executor, ou zero quando os dados ou dimensões forem inválidos....
        float treinarModelo(
            const std::vector<std::vector<float>> &entradas,
            const std::vector<std::vector<float>> &saidasEsperadas,
            Compilar &modelo,
            int quantidadeEpocas,
            int tamanhoLote = 32)
        {
            Epocas executorEpocas;
            return executorEpocas.executar(
                quantidadeEpocas,
                entradas,
                saidasEsperadas,
                modelo,
                tamanhoLote);
        }
    };
}
