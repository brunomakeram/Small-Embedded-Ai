#pragma once
#include <vector>
#include "Dataset.h"
#include "Compilar.h"
#include "Gradientes.h"
#include "Perdas.h"
#include "Prever.h"

class Treinar {
public:
    float executarLotes(Dataset &dados, const std::vector<int> &indices, int tamanho_lote, Compilar &modelo, Gradientes &gradientesCalc, Perdas &perdasCalc, Prever &previsor) {
        int numeroTotalAmostras = dados.gasto.size();
        float soma_perda_epoca = 0.0f;

        for (int inicioLote = 0; inicioLote < numeroTotalAmostras; inicioLote += tamanho_lote) {
            int fimLote = min(inicioLote + tamanho_lote, numeroTotalAmostras);
            int numeroAmostrasLote = fimLote - inicioLote;
            
            float soma_erro2 = 0.0f;
            float gradientes[3] = {0.0f, 0.0f, 0.0f};

            for (int i = inicioLote; i < fimLote; i++) {
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

            float perda_lote = perdasCalc.calcularMSE(soma_erro2, numeroAmostrasLote);
            soma_perda_epoca += perda_lote * numeroAmostrasLote;
            
            modelo.otimizador.atualizarParametros(modelo.neuronios.pesos, modelo.neuronios.vies, gradientes, numeroAmostrasLote);
        }
        return soma_perda_epoca;
    }
};