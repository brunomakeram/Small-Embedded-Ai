#pragma once
#include <Arduino.h>
#include <math.h>
#include <vector>
#include "../Neuronios/Neuronios.h"
#include "../Prever/Prever.h"

// Namespace Seai agrupa o cálculo e o acúmulo das derivadas usadas durante o treinamento....
// Cada chamada acrescenta à matriz acumuladora as derivadas de uma amostra do lote....
namespace Seai
{
    // Calcula a derivada do erro quadrático médio em relação aos pesos e vieses de uma camada....
    class Gradientes
    {
        // O método público calcula uma previsão e acumula as derivadas e o erro da amostra....
    public:
        // Calcula previsões e acumula gradientes para cada peso e viés da amostra informada....
        // entradas e saidasEsperadas são os valores da amostra; neuronios fornece os parâmetros da camada....
        // gradientesPesos e gradientesVieses devem estar alocados e zerados para a dimensão da camada....
        // somaErroQuadrado recebe a soma dos erros quadráticos; previsor realiza a inferência linear....
        // Para cada saída, a derivada é 2 vezes o erro dividido pela quantidade de saídas; o peso multiplica esse valor pela entrada....
        // Retorna false se a previsão falhar ou as dimensões e valores não corresponderem....
        bool processarAmostraLote(
            const std::vector<float> &entradas,
            const std::vector<float> &saidasEsperadas,
            const Neuronios &neuronios,
            std::vector<std::vector<float>> &gradientesPesos,
            std::vector<float> &gradientesVieses,
            float &somaErroQuadrado,
            Prever &previsor)
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