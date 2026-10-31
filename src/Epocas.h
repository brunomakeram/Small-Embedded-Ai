#pragma once
#include <Arduino.h>
#include <vector>
#include "Dataset.h"
#include "Compilar.h"
#include "Treinar.h"
#include "Gradientes.h"
#include "Perdas.h"
#include "Prever.h"

class Epocas {
public:
    float executar(int epochs, Dataset &dados, Compilar &modelo, int tamanhoLote = 32) {
        Treinar treinador;
        Gradientes gradientesCalc;
        Perdas perdasCalc;
        Prever previsor;
        
        int numeroTotalAmostras = dados.gasto.size();
        // Lote menor que 1 travaria o laço do Treinar, então o mínimo é 1
        int tamanho_lote = tamanhoLote < 1 ? 1 : tamanhoLote;
        std::vector<int> indices(numeroTotalAmostras);

        for (int i = 0; i < numeroTotalAmostras; i++) {
            indices[i] = i;
        }

        float perdas = 0.0f;

        for (int epoca = 0; epoca < epochs; epoca++) {
            // Embaralha as amostras (Fisher-Yates)
            for (int i = numeroTotalAmostras - 1; i > 0; i--) {
                int indiceAleatorio = (int)(esp_random() % (uint32_t)(i + 1));
                int temp = indices[i];
                indices[i] = indices[indiceAleatorio];
                indices[indiceAleatorio] = temp;
            }
            
            float soma_perda_epoca = treinador.executarLotes(dados, indices, tamanho_lote, modelo, gradientesCalc, perdasCalc, previsor);
            perdas = perdasCalc.calcularMSE(soma_perda_epoca, numeroTotalAmostras);
        }
        return perdas;
    }
};