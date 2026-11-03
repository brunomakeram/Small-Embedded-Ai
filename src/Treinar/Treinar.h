#pragma once
#include <vector>
#include "../Dataset/Dataset.h"
#include "../Compilar/Compilar.h"
#include "../Gradientes/Gradientes.h"
#include "../Perdas/Perdas.h"
#include "../Prever/Prever.h"

// Agrupa o processamento dos lotes que compõem uma época de treinamento.
namespace Treinar {
class Treinar {
public:
    // Calcula perdas e gradientes por lote e aplica atualizações aos parâmetros do modelo.
    // dados contém amostras, indices define sua ordem, tamanho_lote limita cada atualização,
    // e os demais objetos fornecem parâmetros, cálculo de gradientes, perda e previsão.
    float executarLotes(Dataset::Dataset &dados, const std::vector<int> &indices, int tamanho_lote, Compilar::Compilar &modelo, Gradientes::Gradientes &gradientesCalc, Perdas::Perdas &perdasCalc, Prever::Prever &previsor) {
        // Usa o vetor de alvos para determinar quantas amostras devem ser percorridas.
        int numeroTotalAmostras = dados.gasto.size();
        // Soma as perdas de cada lote ponderadas pela quantidade de amostras nele.
        float soma_perda_epoca = 0.0f;

        // Avança pela ordem de índices em blocos do tamanho solicitado.
        for (int inicioLote = 0; inicioLote < numeroTotalAmostras; inicioLote += tamanho_lote) {
            // Limita o último lote ao fim do dataset e calcula quantas amostras ele contém.
            int fimLote = min(inicioLote + tamanho_lote, numeroTotalAmostras);
            int numeroAmostrasLote = fimLote - inicioLote;
            
            // Reinicia os acumuladores para que cada lote tenha sua própria perda e gradientes.
            float soma_erro2 = 0.0f;
            // Os dois primeiros valores correspondem aos pesos de entrada e o terceiro ao viés.
            float gradientes[3] = {0.0f, 0.0f, 0.0f};

            // Calcula as contribuições de todas as amostras do lote antes de atualizar os parâmetros.
            for (int i = inicioLote; i < fimLote; i++) {
                // O índice embaralhado associa a posição do lote à amostra original.
                int indiceAmostra = indices[i];
                gradientesCalc.processarAmostraLote(
                    dados.carga[indiceAmostra],
                    dados.distancia[indiceAmostra],
                    dados.gasto[indiceAmostra],
                    modelo.neuronios.pesos, 
                    modelo.neuronios.vies, 
                    gradientes, 
                    soma_erro2, 
                    previsor
                );
            }

            // Converte a soma de erro do lote em MSE e a pondera para somar perdas de épocas.
            float perda_lote = perdasCalc.calcularMSE(soma_erro2, numeroAmostrasLote);
            soma_perda_epoca += perda_lote * numeroAmostrasLote;
            
            // Aplica o otimizador ao gradiente acumulado do lote.
            modelo.otimizador.atualizarParametros(modelo.neuronios.pesos, modelo.neuronios.vies, gradientes, numeroAmostrasLote);
        }
        return soma_perda_epoca;
    }
};
}