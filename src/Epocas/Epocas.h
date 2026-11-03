#pragma once
#include <Arduino.h>
#include <vector>
#include "../Dataset/Dataset.h"
#include "../Compilar/Compilar.h"
#include "../Treinar/Treinar.h"
#include "../Gradientes/Gradientes.h"
#include "../Perdas/Perdas.h"
#include "../Prever/Prever.h"

// Coordena a repetição do treinamento e o embaralhamento das amostras a cada época.
namespace Epocas {
class Epocas {
public:
    // Executa a quantidade solicitada de épocas e devolve a MSE da última época.
    // dados fornece as amostras, modelo mantém os parâmetros e tamanhoLote define o tamanho dos lotes.
    float executar(int epochs, Dataset::Dataset &dados, Compilar::Compilar &modelo, int tamanhoLote = 32) {
        // Cria os componentes usados pelo treinador para processar cada amostra e lote.
        Treinar::Treinar treinador;
        Gradientes::Gradientes gradientesCalc;
        Perdas::Perdas perdasCalc;
        Prever::Prever previsor;
        
        // A quantidade de alvos determina o total de amostras disponível.
        int numeroTotalAmostras = dados.gasto.size();
        // Lote menor que 1 travaria o laço do Treinar, então o mínimo é 1
        int tamanho_lote = tamanhoLote < 1 ? 1 : tamanhoLote;
        // Guarda os índices para embaralhar a ordem sem mover os vetores do dataset.
        std::vector<int> indices(numeroTotalAmostras);

        // Inicializa os índices para apontarem às amostras na ordem original.
        for (int i = 0; i < numeroTotalAmostras; i++) {
            indices[i] = i;
        }

        // Armazena a MSE calculada ao final da época mais recente.
        float perdas = 0.0f;

        // Cada iteração percorre uma época completa de treinamento.
        for (int epoca = 0; epoca < epochs; epoca++) {
            // Embaralha as amostras (Fisher-Yates)
            for (int i = numeroTotalAmostras - 1; i > 0; i--) {
                int indiceAleatorio = (int)(esp_random() % (uint32_t)(i + 1));
                int temp = indices[i];
                indices[i] = indices[indiceAleatorio];
                indices[indiceAleatorio] = temp;
            }
            
            // O treinador devolve a soma ponderada das perdas dos lotes da época.
            float soma_perda_epoca = treinador.executarLotes(dados, indices, tamanho_lote, modelo, gradientesCalc, perdasCalc, previsor);
            // Normaliza a soma pelo número de amostras para obter a MSE da época.
            perdas = perdasCalc.calcularMSE(soma_perda_epoca, numeroTotalAmostras);
        }
        return perdas;
    }
};
}