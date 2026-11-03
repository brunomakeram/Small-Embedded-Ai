#pragma once
#include <Arduino.h>
#include "../Prever/Prever.h"

// Agrupa os cálculos de gradiente de uma amostra dentro de um lote.
namespace Gradientes {
class Gradientes {
public:
    // Calcula o erro previsto e acumula os gradientes dos dois pesos e do viés.
    // As entradas e o alvo vêm do dataset; os parâmetros e acumuladores são atualizados pelo treinamento.
    void processarAmostraLote(float carga, float distancia, float gastoReal, const float *pesos, float vies, float *gradientes, float &somaErroQuadrado, Prever::Prever &previsor) {
        // Reutiliza a previsão linear para comparar a saída estimada com o valor real.
        float gastoPrevisto = previsor.calcular(carga, distancia, pesos, vies);
        // O sinal do erro indica se a estimativa ficou acima ou abaixo do alvo.
        float erro = gastoPrevisto - gastoReal;
        
        // Acumula o erro quadrático para a MSE e a derivada em relação a cada parâmetro.
        somaErroQuadrado += erro * erro;
        gradientes[0] += 2.0f * erro * carga;
        gradientes[1] += 2.0f * erro * distancia;
        gradientes[2] += 2.0f * erro;
    }
};
}