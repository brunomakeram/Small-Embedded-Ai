#pragma once
#include "../Placas/Placas.h"
#include <Arduino.h>
#include <math.h>
#include <vector>

namespace Neuronios {
// Armazena os parâmetros de uma camada totalmente conectada e calcula suas ativações.
class Neuronios {
public:
    // Define os limites da quantidade de neurônios que uma camada pode conter.
    enum {
        // Garante pelo menos um neurônio por camada configurada.
        NUMERO_NEURONIOS_MINIMO = 1,
        // Usado quando a quantidade de neurônios não é informada.
        NUMERO_NEURONIOS_PADRAO = 8,
        // Limita o tamanho máximo de uma camada neste modelo.
        NUMERO_NEURONIOS_MAXIMO = 64
    };

    // Mantém os dois pesos e o viés usados pela API linear anterior da biblioteca.
    float pesos[2] = {0.0f, 0.0f};
    float vies = 0.0f;
    // Registra o tamanho da entrada e a quantidade configurada de neurônios nesta camada.
    int numeroEntradas = 0;
    int numeroNeuronios = NUMERO_NEURONIOS_PADRAO;
    // Armazena um vetor de pesos e um viés para cada neurônio da camada.
    std::vector<std::vector<float>> pesosCamada;
    std::vector<float> viesCamada;

    // Configura as dimensões da camada e limita a quantidade de neurônios ao intervalo permitido.
    // quantidadeEntradas define o número de coeficientes por neurônio; quantidadeNeuronios define o número de unidades da camada.
    void configurar(int quantidadeEntradas, int quantidadeNeuronios = NUMERO_NEURONIOS_PADRAO) {
        numeroEntradas = quantidadeEntradas;
        numeroNeuronios = quantidadeNeuronios < NUMERO_NEURONIOS_MINIMO
            ? NUMERO_NEURONIOS_MINIMO
            : (quantidadeNeuronios > NUMERO_NEURONIOS_MAXIMO
                ? NUMERO_NEURONIOS_MAXIMO
                : quantidadeNeuronios);
        pesosCamada.assign(numeroNeuronios, std::vector<float>(numeroEntradas > 0 ? numeroEntradas : 0, 0.0f));
        viesCamada.assign(numeroNeuronios, 0.0f);
    }

    // Gera um valor pseudoaleatório entre zero e um usando o gerador do microcontrolador.
    float gerarNumeroAleatorio() {
        // Divide o valor de 32 bits de esp_random pelo seu máximo para obter uma fração entre zero e um.
        return (float)((double)esp_random() / 4294967295.0);
    }

    // Inicializa os pesos com Glorot e zera os vieses de todos os neurônios.
    void inicializar() {
        if (numeroEntradas <= 0) {
            // Mantém a configuração padrão de duas entradas para chamadas antigas sem dimensões.
            configurar(2, numeroNeuronios);
        }

        // O limite Glorot usa as dimensões de entrada e saída para controlar a escala inicial dos pesos.
        const float limiteGlorot = sqrtf(6.0f / (numeroEntradas + numeroNeuronios));
        for (int indiceNeuronio = 0; indiceNeuronio < numeroNeuronios; indiceNeuronio++) {
            for (int indiceEntrada = 0; indiceEntrada < numeroEntradas; indiceEntrada++) {
                pesosCamada[indiceNeuronio][indiceEntrada] =
                    (gerarNumeroAleatorio() * 2.0f - 1.0f) * limiteGlorot;
            }
            viesCamada[indiceNeuronio] = 0.0f;
        }

        // Espelha os primeiros parâmetros no formato da API linear legada, quando houver até duas entradas.
        pesos[0] = numeroEntradas > 0 ? pesosCamada[0][0] : 0.0f;
        pesos[1] = numeroEntradas > 1 ? pesosCamada[0][1] : 0.0f;
        vies = viesCamada[0];
    }

    // Calcula a saída de cada neurônio, aplicando tanh apenas quando a camada é oculta.
    // entradas deve ter numeroEntradas valores; ativacaoTanh escolhe a ativação usada nesta camada.
    void calcular(const std::vector<float> &entradas, std::vector<float> &saidas, bool ativacaoTanh) const {
        if ((int)entradas.size() != numeroEntradas) {
            Serial.println("Erro: a quantidade de entradas não corresponde à configuração da camada.");
            saidas.clear();
            return;
        }

        // Reserva uma saída por neurônio antes de preencher os resultados da camada.
        saidas.resize(numeroNeuronios);
        for (int indiceNeuronio = 0; indiceNeuronio < numeroNeuronios; indiceNeuronio++) {
            // Cada saída começa no viés da unidade e acumula os produtos peso-entrada.
            float soma = viesCamada[indiceNeuronio];
            for (int indiceEntrada = 0; indiceEntrada < numeroEntradas; indiceEntrada++) {
                soma += pesosCamada[indiceNeuronio][indiceEntrada] * entradas[indiceEntrada];
            }
            saidas[indiceNeuronio] = ativacaoTanh ? tanhf(soma) : soma;
        }
    }
};
}
