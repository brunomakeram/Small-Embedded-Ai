#pragma once
#include "Dataset.h"
#include "Compilar.h"
#include "Epocas.h"

class Aprendizagem {
public:
    float treinarModelo(Dataset &dados, Compilar &modelo, int totalEpocas, int tamanhoLote = 32) {
        Epocas epocas;
        return epocas.executar(totalEpocas, dados, modelo, tamanhoLote);
    }
};