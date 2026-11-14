#pragma once

#include "../Placas/Placas.h"
#include <Arduino.h>
#include <math.h>
#include <vector>
#include "../Compilar/Compilar.h"
#include "../Treinar/Treinar.h"
#include "../Gradientes/Gradientes.h"
#include "../Perdas/Perdas.h"
#include "../Prever/Prever.h"

// Namespace Seai coordena as rotinas de treinamento e previsão da biblioteca....
namespace Seai
{
    // Executa as épocas solicitadas sobre matrizes de entrada e saída....
    class Epocas
    {
        // Os métodos públicos controlam a validação e a repetição do treinamento....
    public:
        // Valida as matrizes, embaralha as amostras e executa o treino pelo número de épocas solicitado....
        // entradas e saidasEsperadas devem ter pares de linhas finitas, dimensões uniformes e compatíveis com modelo....
        // tamanhoLote menor que um é ajustado para um; quantidadeEpocas deve ser positiva....
        // Retorna a perda MSE da última época ou zero quando os dados ou dimensões são inválidos....
        // quantidadeEpocas define as passagens; entradas e saidasEsperadas fornecem os pares de treino....
        // modelo fornece a camada e o otimizador; tamanhoLote define o máximo de amostras por atualização....
        float executar(
            int quantidadeEpocas,
            const std::vector<std::vector<float>> &entradas,
            const std::vector<std::vector<float>> &saidasEsperadas,
            Compilar &modelo,
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

            // Instancia os componentes que processam lotes, derivadas, perdas e previsões....
            Treinar treinador;
            Gradientes calculadorGradientes;
            Perdas calculadorPerdas;
            Prever previsor;
            // Converte a quantidade de amostras para o tipo usado pelos índices e pelos lotes....
            int numeroTotalAmostras = (int)entradas.size();
            // Evita um incremento inválido quando o tamanho do lote recebido é menor que um....
            int tamanhoLoteValido = tamanhoLote < 1 ? 1 : tamanhoLote;
            // Mantém a ordem das amostras em um vetor que será embaralhado a cada época....
            std::vector<int> indices(numeroTotalAmostras);
            // Inicializa os índices com a posição original de cada par de treino....
            for (int indiceAmostra = 0; indiceAmostra < numeroTotalAmostras; indiceAmostra++)
            {
                indices[indiceAmostra] = indiceAmostra;
            }

            // Guarda a perda média calculada após o processamento de cada época....
            float perdaFinal = 0.0f;
            for (int indiceEpoca = 0; indiceEpoca < quantidadeEpocas; indiceEpoca++)
            {
                // Embaralha os índices sem modificar a associação entre entradas e saídas esperadas....
                for (int indiceAmostra = numeroTotalAmostras - 1; indiceAmostra > 0; indiceAmostra--)
                {
                    // Sorteia uma posição válida e troca os elementos para produzir uma permutação....
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
