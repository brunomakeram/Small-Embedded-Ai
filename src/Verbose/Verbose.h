#pragma once
#include <Arduino.h>
#include <vector>

// Namespace Seai agrupa as opções de registro e as mensagens de acompanhamento do treino....
namespace Seai
{
    // Centraliza mensagens de acompanhamento e o resultado final exibido pela biblioteca....
    class Verbose
    {
    public:
        // Habilita separadamente a impressão dos parâmetros usados para iniciar o treino....
        bool mostrarConfiguracao = false;
        // Habilita a impressão do índice de cada época concluída....
        bool mostrarNumeroEpoca = false;
        // Habilita a impressão da perda calculada para cada época....
        bool mostrarPerdaPorEpoca = false;

        // Exibe taxa, épocas, lote, larguras ocultas e saídas somente quando mostrarConfiguracao está habilitado....
        // neuroniosPorCamada lista as larguras ocultas; numeroSaidas informa a dimensão da última camada....
        void registrarConfiguracao(
            // Valores usados pelo laço de treinamento e pela camada de saída....
            float taxaAprendizado,
            int epocas,
            int tamanhoLote,
            // Quantidade de neurônios de cada camada oculta e número de valores de saída....
            const std::vector<int> &neuroniosPorCamada,
            std::size_t numeroSaidas
        )
        {
            // A opção é independente das demais para que cada categoria possa ser selecionada....
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

        // Exibe o índice de época recebido, esperado começar em um, quando mostrarNumeroEpoca está habilitado....
        void registrarEpoca(int epoca)
        {
            if (!mostrarNumeroEpoca)
                return;

            Serial.printf("[Treino] Época %d\n", epoca);
        }

        // Exibe a perda MSE da época quando mostrarPerdaPorEpoca está habilitado....
        void registrarPerdaEpoca(float perda)
        {
            if (!mostrarPerdaPorEpoca)
                return;

            Serial.printf("[Treino] Perda da época: %.6f\n", perda);
        }

        // Mantém a assinatura legada e encaminha somente o valor de perda para a mensagem final....
        void mostrarResultados(float, float, float perdas, const float *, float, float)
        {
            registrarPerdaFinal(perdas);
        }

        // Recebe a entrada e saída genéricas por compatibilidade, mas exibe somente a perda final....
        void mostrarResultados(const std::vector<float> &, float perdas, const std::vector<float> &)
        {
            registrarPerdaFinal(perdas);
        }

    private:
        // Mantém a saída final restrita à perda, sem expor parâmetros ou previsões....
        void registrarPerdaFinal(float perdas)
        {
            Serial.printf("[Resultado] Perda final: %.6f\n", perdas);
        }
    };
}
