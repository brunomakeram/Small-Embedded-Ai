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

            Serial.print("[Treino] Configuração: taxa de aprendizado=");
            Serial.print(taxaAprendizado, 6);
            Serial.print(", épocas=");
            Serial.print(epocas);
            Serial.print(", tamanho do lote=");
            Serial.println(tamanhoLote);
            Serial.print("[Treino] Camadas ocultas: [");
            for (std::size_t indice = 0; indice < neuroniosPorCamada.size(); indice++)
            {
                if (indice > 0)
                {
                    Serial.print(", ");
                }
                Serial.print(neuroniosPorCamada[indice]);
            }
            Serial.println("]");
            Serial.print("[Treino] Neurônios na camada de saída: ");
            Serial.println((unsigned int)numeroSaidas);
        }

        // Registra o número começando em um, de forma correspondente à contagem apresentada ao usuário.
        void registrarEpoca(int epoca)
        {
            if (!mostrarNumeroEpoca)
                return;

            Serial.print("[Treino] Época ");
            Serial.println(epoca);
        }

        // Registra a perda da época somente quando sua opção específica está habilitada.
        void registrarPerdaEpoca(float perda)
        {
            if (!mostrarPerdaPorEpoca)
                return;

            Serial.print("[Treino] Perda da época: ");
            Serial.println(perda, 6);
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
            Serial.print("[Resultado] Perda final: ");
            Serial.println(perdas, 6);
        }
    };
}
