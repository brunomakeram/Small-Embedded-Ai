#pragma once

#include <Arduino.h>
#include <math.h>
#include <vector>
#include "../Neuronios/Neuronios.h"

// Agrupa a inferência linear para vetores de entrada e saída de qualquer dimensão.
namespace Prever
{
    // Calcula uma saída usando os parâmetros de uma camada de neurônios.
    class Prever
    {
        // Os métodos públicos realizam a inferência sobre uma camada configurada.
    public:
        // Avalia as entradas com uma camada linear e preenche o vetor de saídas.
        bool calcular(
            const std::vector<float> &entradas,
            const Neuronios::Neuronios &neuronios,
            std::vector<float> &saidas) const
        {
            if (neuronios.numeroEntradas < 1 || neuronios.numeroNeuronios < 1 ||
                entradas.size() != (std::size_t)neuronios.numeroEntradas)
            {
                Serial.println("Erro: a quantidade de entradas não corresponde à configuração da camada.");
                saidas.clear();
                return false;
            }
            for (std::size_t indiceEntrada = 0; indiceEntrada < entradas.size(); indiceEntrada++)
            {
                if (!isfinite(entradas[indiceEntrada]))
                {
                    Serial.println("Erro: as entradas da previsão devem ser finitas.");
                    saidas.clear();
                    return false;
                }
            }
            neuronios.calcular(entradas, saidas, false);
            return !saidas.empty();
        }
    };
}