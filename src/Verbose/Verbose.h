#pragma once
#include <Arduino.h>
#include <vector>

namespace Verbose
{
    // Centraliza mensagens de acompanhamento e o resultado final exibido pela biblioteca.
    class Verbose
    {
    public:
        // Habilita separadamente a impressão dos parâmetros usados para iniciar o treino.
        bool mostrarConfiguracao = false;
        // Habilita a impressão do índice de cada época concluída.
        bool mostrarNumeroEpoca = false;
        // Habilita a impressão da perda calculada para cada época.
        bool mostrarPerdaPorEpoca = false;

        // Registra a arquitetura e os hiperparâmetros quando a opção de configuração está ligada.
        void registrarConfiguracao(
            // Valores usados pelo laço de treinamento e pela camada de saída.
            float taxaAprendizado,
            int epocas,
            int tamanhoLote,
            // Quantidade de neurônios de cada camada oculta e número de valores de saída.
            const std::vector<int> &neuroniosPorCamada,
            std::size_t numeroSaidas
        )
        {
            // A opção é independente das demais para que cada categoria possa ser selecionada.
            if (!mostrarConfiguracao)
                return;

            Serial.printf(
                "[Treino] Configuração: taxa de aprendizado=%.6f, épocas=%d, tamanho do lote=%d\n",
                taxaAprendizado,
                epocas,
                tamanhoLote
            );
            Serial.print("[Treino] Camadas ocultas: [");
            for (std::size_t indice = 0; indice < neuroniosPorCamada.size(); indice++)
            {
                Serial.printf(indice == 0 ? "%d" : ", %d", neuroniosPorCamada[indice]);
            }
            Serial.println("]");
            Serial.printf("[Treino] Neurônios na camada de saída: %u\n", (unsigned int)numeroSaidas);
        }

        // Registra o número começando em um, de forma correspondente à contagem apresentada ao usuário.
        void registrarEpoca(int epoca)
        {
            if (!mostrarNumeroEpoca)
                return;

            Serial.printf("[Treino] Época %d\n", epoca);
        }

        // Registra a perda da época somente quando sua opção específica está habilitada.
        void registrarPerdaEpoca(float perda)
        {
            if (!mostrarPerdaPorEpoca)
                return;

            Serial.printf("[Treino] Perda da época: %.6f\n", perda);
        }

        // Preserva a interface anterior e encaminha apenas a perda para a apresentação final.
        void mostrarResultados(float, float, float perdas, const float *, float, float)
        {
            registrarPerdaFinal(perdas);
        }

        // Aceita a interface genérica sem imprimir vetores de entrada ou de saída.
        void mostrarResultados(const std::vector<float> &, float perdas, const std::vector<float> &)
        {
            registrarPerdaFinal(perdas);
        }

    private:
        // Mantém a saída final restrita à perda, sem expor parâmetros ou previsões.
        void registrarPerdaFinal(float perdas)
        {
            Serial.printf("[Resultado] Perda final: %.6f\n", perdas);
        }
    };
}
