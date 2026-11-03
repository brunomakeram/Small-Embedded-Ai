#pragma once
#include "../Dataset/Dataset.h"
#include "../Compilar/Compilar.h"
#include "../Epocas/Epocas.h"

// Oferece uma entrada única para iniciar o treinamento do modelo compilado.
namespace Aprendizagem {
class Aprendizagem {
public:
    // Encaminha o dataset e os parâmetros de treino para o executor de épocas e devolve a perda final.
    // dados contém as amostras, modelo guarda os parâmetros treináveis, e os inteiros definem duração e lote.
    float treinarModelo(Dataset::Dataset &dados, Compilar::Compilar &modelo, int totalEpocas, int tamanhoLote = 32) {
        // O executor concentra a repetição por épocas e recebe o tamanho do lote solicitado.
        Epocas::Epocas epocas;
        return epocas.executar(totalEpocas, dados, modelo, tamanhoLote);
    }
};
}