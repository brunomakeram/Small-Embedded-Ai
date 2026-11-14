#pragma once

#include <Arduino.h>
#include <vector>
#include "../Compilar/Compilar.h"
#include "../Gradientes/Gradientes.h"
#include "../Perdas/Perdas.h"
#include "../Prever/Prever.h"

// Namespace Seai reúne o processamento dos lotes e a atualização dos parâmetros treináveis....
namespace Seai
{
    // Calcula perdas e gradientes de matrizes de qualquer dimensão e atualiza os parâmetros....
    class Treinar
    {
        // Os métodos públicos processam um conjunto de lotes....
    public:
        // Processa as amostras na ordem de indices em blocos e atualiza pesos e vieses após cada lote....
        // entradas e saidasEsperadas devem ser matrizes pareadas; indices deve conter uma permutação válida das amostras....
        // modelo fornece os parâmetros e o otimizador; os três calculadores executam gradiente, perda e inferência....
        // tamanhoLote deve ser positivo; o retorno é a soma da perda de cada lote ponderada por sua quantidade de amostras....
        // calculadorGradientes, calculadorPerdas e previsor são os componentes reutilizados durante as atualizações....
        float executarLotes(
            const std::vector<std::vector<float>> &entradas,
            const std::vector<std::vector<float>> &saidasEsperadas,
            const std::vector<int> &indices,
            int tamanhoLote,
            Compilar &modelo,
            Gradientes &calculadorGradientes,
            Perdas &calculadorPerdas,
            Prever &previsor)
        {
            if (entradas.empty() || entradas.size() != saidasEsperadas.size() ||
                indices.size() != entradas.size() || tamanhoLote < 1 ||
                entradas[0].empty() || saidasEsperadas[0].empty() ||
                modelo.neuronios.numeroEntradas != (int)entradas[0].size() ||
                modelo.neuronios.numeroNeuronios != (int)saidasEsperadas[0].size())
            {
                Serial.println("Erro: dados, índices e tamanho do lote são incompatíveis.");
                return 0.0f;
            }
            for (std::size_t indiceAmostra = 0; indiceAmostra < entradas.size(); indiceAmostra++)
            {
                if (entradas[indiceAmostra].size() != entradas[0].size() ||
                    saidasEsperadas[indiceAmostra].size() != saidasEsperadas[0].size())
                {
                    Serial.println("Erro: todas as amostras devem ter dimensões consistentes.");
                    return 0.0f;
                }
            }
            for (std::size_t indiceAmostra = 0; indiceAmostra < indices.size(); indiceAmostra++)
            {
                if (indices[indiceAmostra] < 0 || (std::size_t)indices[indiceAmostra] >= entradas.size())
                {
                    Serial.println("Erro: um índice de amostra está fora dos limites.");
                    return 0.0f;
                }
            }

            // Registra os totais usados para percorrer a matriz e dimensionar os vetores de parâmetros....
            int numeroTotalAmostras = (int)entradas.size();
            int quantidadeSaidas = (int)saidasEsperadas[0].size();
            // Acumula a perda de todos os lotes, ponderada pelo número de amostras de cada lote....
            float somaPerdaEpoca = 0.0f;
            for (int inicioLote = 0; inicioLote < numeroTotalAmostras; inicioLote += tamanhoLote)
            {
                // Limita o fim do lote ao tamanho da matriz, incluindo o possível último lote parcial....
                int fimLote = inicioLote + tamanhoLote;
                if (fimLote > numeroTotalAmostras)
                {
                    fimLote = numeroTotalAmostras;
                }
                // Obtém a quantidade real de amostras neste lote e zera a soma de erro correspondente....
                int numeroAmostrasLote = fimLote - inicioLote;
                float somaErroQuadrado = 0.0f;
                // Aloca acumuladores zerados com o mesmo formato da matriz de pesos e do vetor de vieses....
                std::vector<std::vector<float>> gradientesPesos(
                    quantidadeSaidas,
                    std::vector<float>(modelo.neuronios.numeroEntradas, 0.0f));
                std::vector<float> gradientesVieses(quantidadeSaidas, 0.0f);

                for (int indiceNoLote = inicioLote; indiceNoLote < fimLote; indiceNoLote++)
                {
                    // Recupera a posição original da amostra na permutação recebida....
                    std::size_t indiceAmostra = (std::size_t)indices[indiceNoLote];
                    if (!calculadorGradientes.processarAmostraLote(
                            entradas[indiceAmostra],
                            saidasEsperadas[indiceAmostra],
                            modelo.neuronios,
                            gradientesPesos,
                            gradientesVieses,
                            somaErroQuadrado,
                            previsor))
                    {
                        return 0.0f;
                    }
                }

                // Conta os valores de saída avaliados e calcula o MSE do lote....
                int quantidadeValoresLote = numeroAmostrasLote * quantidadeSaidas;
                float perdaLote = calculadorPerdas.calcularMSE(somaErroQuadrado, quantidadeValoresLote);
                somaPerdaEpoca += perdaLote * numeroAmostrasLote;

                // Achata pesos e vieses na ordem esperada pelo otimizador Adam....
                std::vector<float> parametros;
                std::vector<float> gradientes;
                for (int indiceSaida = 0; indiceSaida < quantidadeSaidas; indiceSaida++)
                {
                    for (int indiceEntrada = 0; indiceEntrada < modelo.neuronios.numeroEntradas; indiceEntrada++)
                    {
                        parametros.push_back(modelo.neuronios.pesosCamada[indiceSaida][indiceEntrada]);
                        gradientes.push_back(gradientesPesos[indiceSaida][indiceEntrada]);
                    }
                    parametros.push_back(modelo.neuronios.viesCamada[indiceSaida]);
                    gradientes.push_back(gradientesVieses[indiceSaida]);
                }
                modelo.otimizador.atualizarParametros(parametros, gradientes, numeroAmostrasLote);

                // Percorre os parâmetros atualizados para restaurá-los às matrizes e vetores da camada....
                std::size_t indiceParametro = 0;
                for (int indiceSaida = 0; indiceSaida < quantidadeSaidas; indiceSaida++)
                {
                    for (int indiceEntrada = 0; indiceEntrada < modelo.neuronios.numeroEntradas; indiceEntrada++)
                    {
                        modelo.neuronios.pesosCamada[indiceSaida][indiceEntrada] = parametros[indiceParametro++];
                    }
                    modelo.neuronios.viesCamada[indiceSaida] = parametros[indiceParametro++];
                }
            }
            return somaPerdaEpoca;
        }
    };
}
