#pragma once
#include <Arduino.h>
#include <math.h>

class Adam {
public:
    float taxaAprendizado;
    float decaimentoPrimeiroMomento;
    float decaimentoSegundoMomento;
    float margemEstabilidadeNumerica;
    long numeroAtualizacoes;
    float mediaMovelGradientes[3];
    float mediaMovelGradientesQuadrados[3];

    void inicializar(float lr = 0.05f, float beta1 = 0.9f, float beta2 = 0.999f, float eps = 1e-7f) {
        taxaAprendizado = lr;
        decaimentoPrimeiroMomento = beta1;
        decaimentoSegundoMomento = beta2;
        margemEstabilidadeNumerica = eps;
        numeroAtualizacoes = 0;

        for (int i = 0; i < 3; i++) {
            mediaMovelGradientes[i] = 0.0f;
            mediaMovelGradientesQuadrados[i] = 0.0f;
        }
    }

    void atualizarParametros(float *pesos, float &vies, float *gradientes, int numeroAmostrasLote) {
        if (numeroAmostrasLote <= 0) return;

        for (int i = 0; i < 3; i++) {
            gradientes[i] /= (float)numeroAmostrasLote;
        }

        numeroAtualizacoes++;

        float correcaoSegundoMomento = sqrtf(1.0f - powf(decaimentoSegundoMomento, (float)numeroAtualizacoes));
        float correcaoPrimeiroMomento = 1.0f - powf(decaimentoPrimeiroMomento, (float)numeroAtualizacoes);
        float taxaAprendizadoCorrigida = taxaAprendizado * correcaoSegundoMomento / correcaoPrimeiroMomento;

        float *parametros[3] = {&pesos[0], &pesos[1], &vies};

        for (int i = 0; i < 3; i++) {
            mediaMovelGradientes[i] = decaimentoPrimeiroMomento * mediaMovelGradientes[i] + (1.0f - decaimentoPrimeiroMomento) * gradientes[i];
            mediaMovelGradientesQuadrados[i] = decaimentoSegundoMomento * mediaMovelGradientesQuadrados[i] + (1.0f - decaimentoSegundoMomento) * gradientes[i] * gradientes[i];
            
            float ajusteParametro = taxaAprendizadoCorrigida * mediaMovelGradientes[i] / (sqrtf(mediaMovelGradientesQuadrados[i]) + margemEstabilidadeNumerica);
            *(parametros[i]) -= ajusteParametro;
        }
    }
};