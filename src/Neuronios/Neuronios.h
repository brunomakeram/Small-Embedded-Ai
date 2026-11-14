#pragma once
#include "../Placas/Placas.h"
#include <Arduino.h>
#include <math.h>
#include <vector>

// Namespace Seai contém a representação e os cálculos de uma camada totalmente conectada....
namespace Seai {
// Armazena os parâmetros de uma camada totalmente conectada e calcula suas ativações....
class Neuronios {
public:
    // Define os limites da quantidade de neurônios que uma camada pode conter....
    enum {
        // Garante pelo menos um neurônio por camada configurada....
        NUMERO_NEURONIOS_MINIMO = 1,
        // Usado quando a quantidade de neurônios não é informada....
        NUMERO_NEURONIOS_PADRAO = 8,
        // Limita o tamanho máximo de uma camada neste modelo....
        NUMERO_NEURONIOS_MAXIMO = 64
    };

    // Espelha os dois primeiros pesos e o primeiro viés para compatibilidade com a API linear anterior....
    float pesos[2] = {0.0f, 0.0f};
    // Espelha o viés do primeiro neurônio para compatibilidade com a API linear anterior....
    float vies = 0.0f;
    // Registra a largura de entrada e a quantidade efetiva de neurônios configurados....
    int numeroEntradas = 0;
    int numeroNeuronios = NUMERO_NEURONIOS_PADRAO;
    // Cada linha de pesosCamada pertence a um neurônio; viesCamada guarda seu termo independente....
    std::vector<std::vector<float>> pesosCamada;
    // Armazena um viés por neurônio, na mesma ordem das linhas de pesosCamada....
    std::vector<float> viesCamada;

    // Configura as dimensões da camada e limita a quantidade de neurônios ao intervalo permitido....
    // quantidadeEntradas define o número de coeficientes por neurônio; quantidadeNeuronios define o número de unidades da camada....
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

    // Gera um valor pseudoaleatório no intervalo inclusivo aproximado de zero a um....
    // Usa esp_random(), fornecido pelo módulo de plataforma para ESP32 e RP2040....
    float gerarNumeroAleatorio() {
        // Divide o valor de 32 bits de esp_random pelo seu máximo para obter uma fração entre zero e um....
        return (float)((double)esp_random() / 4294967295.0);
    }

    // Inicializa os pesos com distribuição Glorot e zera os vieses de todos os neurônios....
    // Se a camada ainda não tem entradas, preserva a compatibilidade usando duas entradas....
    void inicializar() {
        if (numeroEntradas <= 0) {
            // Mantém a configuração padrão de duas entradas para chamadas antigas sem dimensões....
            configurar(2, numeroNeuronios);
        }

        // O limite Glorot usa as dimensões de entrada e saída para controlar a escala inicial dos pesos....
        const float limiteGlorot = sqrtf(6.0f / (numeroEntradas + numeroNeuronios));
        for (int indiceNeuronio = 0; indiceNeuronio < numeroNeuronios; indiceNeuronio++) {
            for (int indiceEntrada = 0; indiceEntrada < numeroEntradas; indiceEntrada++) {
                pesosCamada[indiceNeuronio][indiceEntrada] =
                    (gerarNumeroAleatorio() * 2.0f - 1.0f) * limiteGlorot;
            }
            viesCamada[indiceNeuronio] = 0.0f;
        }

        // Espelha os primeiros parâmetros no formato da API linear legada, quando houver até duas entradas....
        pesos[0] = numeroEntradas > 0 ? pesosCamada[0][0] : 0.0f;
        pesos[1] = numeroEntradas > 1 ? pesosCamada[0][1] : 0.0f;
        vies = viesCamada[0];
    }

    // Calcula a soma ponderada de cada neurônio e opcionalmente aplica tanh à ativação....
    // entradas deve ter numeroEntradas valores; ativacaoTanh deve ser true para camadas ocultas e false para a saída linear....
    // saidas recebe um valor por neurônio; em caso de dimensão incompatível, é limpa e o erro é informado pelo Serial....
    void calcular(const std::vector<float> &entradas, std::vector<float> &saidas, bool ativacaoTanh) const {
        if ((int)entradas.size() != numeroEntradas) {
            Serial.println("Erro: a quantidade de entradas não corresponde à configuração da camada.");
            saidas.clear();
            return;
        }

        // Reserva uma saída por neurônio antes de preencher os resultados da camada....
        saidas.resize(numeroNeuronios);
        for (int indiceNeuronio = 0; indiceNeuronio < numeroNeuronios; indiceNeuronio++) {
            // Cada saída começa no viés da unidade e acumula os produtos peso-entrada....
            float soma = viesCamada[indiceNeuronio];
            for (int indiceEntrada = 0; indiceEntrada < numeroEntradas; indiceEntrada++) {
                soma += pesosCamada[indiceNeuronio][indiceEntrada] * entradas[indiceEntrada];
            }
            saidas[indiceNeuronio] = ativacaoTanh ? tanhf(soma) : soma;
        }
    }
};
}
