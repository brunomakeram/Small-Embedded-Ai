#pragma once

#include <Arduino.h>
#include <math.h>
#include <vector>
#include "../Compilar/Compilar.h"
#include "../Treinar/Treinar.h"
#include "../Gradientes/Gradientes.h"
#include "../Perdas/Perdas.h"
#include "../Prever/Prever.h"

// Coordena o treinamento por épocas e o embaralhamento das amostras.
namespace Epocas
{
    // Executa as épocas solicitadas sobre matrizes de entrada e saída.
    class Epocas
    {
        // Os métodos públicos controlam a validação e a repetição do treinamento.
    public:
        // Treina o modelo e retorna a perda média quadrática da última época.
        float executar(
            int quantidadeEpocas,
            const std::vector<std::vector<float>> &entradas,
            const std::vector<std::vector<float>> &saidasEsperadas,
            Compilar::Compilar &modelo,
            int tamanhoLote = 32)
        {
            if (quantidadeEpocas < 1 || entradas.empty() ||
                entradas.size() != saidasEsperadas.size() ||
                entradas[0].empty() || saidasEsperadas[0].empty() ||
                modelo.neuronios.numeroEntradas != (int)entradas[0].size() ||
                modelo.neuronios.numeroNeuronios != (int)saidasEsperadas[0].size())
            {
                Serial.println("Erro: épocas e matrizes de treino devem conter valores válidos.");
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
                for (std::size_t indiceEntrada = 0; indiceEntrada < entradas[indiceAmostra].size(); indiceEntrada++)
                {
                    if (!isfinite(entradas[indiceAmostra][indiceEntrada]))
                    {
                        Serial.println("Erro: as entradas devem conter apenas valores finitos.");
                        return 0.0f;
                    }
                }
                for (std::size_t indiceSaida = 0; indiceSaida < saidasEsperadas[indiceAmostra].size(); indiceSaida++)
                {
                    if (!isfinite(saidasEsperadas[indiceAmostra][indiceSaida]))
                    {
                        Serial.println("Erro: as saídas devem conter apenas valores finitos.");
                        return 0.0f;
                    }
                }
            }

            Treinar::Treinar treinador;
            Gradientes::Gradientes calculadorGradientes;
            Perdas::Perdas calculadorPerdas;
            Prever::Prever previsor;
            int numeroTotalAmostras = (int)entradas.size();
            int tamanhoLoteValido = tamanhoLote < 1 ? 1 : tamanhoLote;
            std::vector<int> indices(numeroTotalAmostras);
            for (int indiceAmostra = 0; indiceAmostra < numeroTotalAmostras; indiceAmostra++)
            {
                indices[indiceAmostra] = indiceAmostra;
            }

            float perdaFinal = 0.0f;
            for (int indiceEpoca = 0; indiceEpoca < quantidadeEpocas; indiceEpoca++)
            {
                for (int indiceAmostra = numeroTotalAmostras - 1; indiceAmostra > 0; indiceAmostra--)
                {
                    int indiceAleatorio = (int)(esp_random() % (uint32_t)(indiceAmostra + 1));
                    int indiceTemporario = indices[indiceAmostra];
                    indices[indiceAmostra] = indices[indiceAleatorio];
                    indices[indiceAleatorio] = indiceTemporario;
                }
                float somaPerdasAmostras = treinador.executarLotes(
                    entradas,
                    saidasEsperadas,
                    indices,
                    tamanhoLoteValido,
                    modelo,
                    calculadorGradientes,
                    calculadorPerdas,
                    previsor);
                perdaFinal = calculadorPerdas.calcularMSE(somaPerdasAmostras, numeroTotalAmostras);
            }
            return perdaFinal;
        }
    };
}
