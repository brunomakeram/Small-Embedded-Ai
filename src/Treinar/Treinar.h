#pragma once

#include <Arduino.h>
#include <vector>
#include "../Compilar/Compilar.h"
#include "../Gradientes/Gradientes.h"
#include "../Perdas/Perdas.h"
#include "../Prever/Prever.h"

// Agrupa o processamento dos lotes que compõem uma época de treinamento.
namespace Treinar
{
    // Calcula perdas e gradientes de matrizes de qualquer dimensão e atualiza os parâmetros.
    class Treinar
    {
        // Os métodos públicos processam um conjunto de lotes.
    public:
        // Processa os índices em lotes e retorna a soma das perdas ponderadas pela quantidade de amostras.
        float executarLotes(
            const std::vector<std::vector<float>> &entradas,
            const std::vector<std::vector<float>> &saidasEsperadas,
            const std::vector<int> &indices,
            int tamanhoLote,
            Compilar::Compilar &modelo,
            Gradientes::Gradientes &calculadorGradientes,
            Perdas::Perdas &calculadorPerdas,
            Prever::Prever &previsor)
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

            int numeroTotalAmostras = (int)entradas.size();
            int quantidadeSaidas = (int)saidasEsperadas[0].size();
            float somaPerdaEpoca = 0.0f;
            for (int inicioLote = 0; inicioLote < numeroTotalAmostras; inicioLote += tamanhoLote)
            {
                int fimLote = inicioLote + tamanhoLote;
                if (fimLote > numeroTotalAmostras)
                {
                    fimLote = numeroTotalAmostras;
                }
                int numeroAmostrasLote = fimLote - inicioLote;
                float somaErroQuadrado = 0.0f;
                std::vector<std::vector<float>> gradientesPesos(
                    quantidadeSaidas,
                    std::vector<float>(modelo.neuronios.numeroEntradas, 0.0f));
                std::vector<float> gradientesVieses(quantidadeSaidas, 0.0f);

                for (int indiceNoLote = inicioLote; indiceNoLote < fimLote; indiceNoLote++)
                {
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

                int quantidadeValoresLote = numeroAmostrasLote * quantidadeSaidas;
                float perdaLote = calculadorPerdas.calcularMSE(somaErroQuadrado, quantidadeValoresLote);
                somaPerdaEpoca += perdaLote * numeroAmostrasLote;

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
