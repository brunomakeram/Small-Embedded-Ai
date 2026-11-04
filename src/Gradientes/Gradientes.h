#pragma once
#include <Arduino.h>
#include <math.h>
#include <vector>
#include "../Neuronios/Neuronios.h"
#include "../Prever/Prever.h"

// Agrupa os cálculos dos gradientes para amostras com dimensões variáveis.
namespace Gradientes
{
    // Acumula as derivadas de uma amostra para todos os pesos e vieses da camada.
    class Gradientes
    {
        // Os métodos públicos acumulam derivadas e erros de treino.
    public:
        // Calcula previsões e acumula gradientes para cada entrada e saída da amostra.
        bool processarAmostraLote(
            const std::vector<float> &entradas,
            const std::vector<float> &saidasEsperadas,
            const Neuronios::Neuronios &neuronios,
            std::vector<std::vector<float>> &gradientesPesos,
            std::vector<float> &gradientesVieses,
            float &somaErroQuadrado,
            Prever::Prever &previsor)
        {
            std::vector<float> saidasPrevistas;
            if (!previsor.calcular(entradas, neuronios, saidasPrevistas))
            {
                return false;
            }
            if (saidasEsperadas.size() != saidasPrevistas.size() ||
                gradientesPesos.size() != saidasPrevistas.size() ||
                gradientesVieses.size() != saidasPrevistas.size())
            {
                Serial.println("Erro: as saídas esperadas e os gradientes devem corresponder à camada.");
                return false;
            }

            for (std::size_t indiceSaida = 0; indiceSaida < saidasPrevistas.size(); indiceSaida++)
            {
                if (!isfinite(saidasEsperadas[indiceSaida]) ||
                    gradientesPesos[indiceSaida].size() != entradas.size())
                {
                    Serial.println("Erro: dimensão ou valor inválido no cálculo dos gradientes.");
                    return false;
                }
                float erro = saidasPrevistas[indiceSaida] - saidasEsperadas[indiceSaida];
                float derivadaErro = 2.0f * erro / saidasPrevistas.size();
                somaErroQuadrado += erro * erro;
                for (std::size_t indiceEntrada = 0; indiceEntrada < entradas.size(); indiceEntrada++)
                {
                    gradientesPesos[indiceSaida][indiceEntrada] += derivadaErro * entradas[indiceEntrada];
                }
                gradientesVieses[indiceSaida] += derivadaErro;
            }
            return true;
        }
    };
}