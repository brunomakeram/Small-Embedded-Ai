#pragma once

#include <Arduino.h>
#include <math.h>
#include <vector>

namespace Adam
{
    // Mantém os momentos do Adam e atualiza uma quantidade variável de parâmetros.
    class Adam
    {
        // Os métodos públicos configuram o otimizador e atualizam os parâmetros.
    public:
        // Expõe os parâmetros do otimizador para configuração pelo código chamador.
        float taxaAprendizado = 0.05f;
        float decaimentoPrimeiroMomento = 0.9f;
        float decaimentoSegundoMomento = 0.999f;
        float margemEstabilidadeNumerica = 1e-7f;
        long numeroAtualizacoes = 0;

        // Constrói o otimizador com a taxa, os decaimentos e a margem informados.
        // taxaAprendizadoPadrao: float de valor padrão 0.05; controla o tamanho de cada ajuste.
        // decaimentoPrimeiroMomentoPadrao: float de valor padrão 0.9; controla a média dos gradientes.
        // decaimentoSegundoMomentoPadrao: float de valor padrão 0.999; controla a média dos gradientes ao quadrado.
        // margemEstabilidadeNumericaPadrao: float de valor padrão 1e-7; evita divisão instável.
        Adam(
            float taxaAprendizadoPadrao = 0.05f,
            float decaimentoPrimeiroMomentoPadrao = 0.9f,
            float decaimentoSegundoMomentoPadrao = 0.999f,
            float margemEstabilidadeNumericaPadrao = 1e-7f)
        {
            inicializar(
                taxaAprendizadoPadrao,
                decaimentoPrimeiroMomentoPadrao,
                decaimentoSegundoMomentoPadrao,
                margemEstabilidadeNumericaPadrao);
        }

        // Reinicia o contador e os momentos para uma nova sequência de treinamento.
        // Os argumentos têm os mesmos tipos e valores padrão do construtor.
        void inicializar(
            float taxaAprendizadoPadrao = 0.05f,
            float decaimentoPrimeiroMomentoPadrao = 0.9f,
            float decaimentoSegundoMomentoPadrao = 0.999f,
            float margemEstabilidadeNumericaPadrao = 1e-7f)
        {
            taxaAprendizado = taxaAprendizadoPadrao;
            decaimentoPrimeiroMomento = decaimentoPrimeiroMomentoPadrao;
            decaimentoSegundoMomento = decaimentoSegundoMomentoPadrao;
            margemEstabilidadeNumerica = margemEstabilidadeNumericaPadrao;
            numeroAtualizacoes = 0;
            mediaMovelGradientes.clear();
            mediaMovelGradientesQuadrados.clear();
            momentosInicializados = false;
        }

        // Atualiza parâmetros usando gradientes acumulados sem modificar o vetor recebido.
        void atualizarParametros(
            std::vector<float> &parametros,
            const std::vector<float> &gradientes,
            int numeroAmostrasLote)
        {
            if (numeroAmostrasLote <= 0)
            {
                return;
            }
            if (parametros.size() != gradientes.size())
            {
                Serial.println("Erro: parâmetros e gradientes devem ter o mesmo tamanho.");
                return;
            }
            if (parametros.empty())
            {
                Serial.println("Erro: o otimizador precisa receber ao menos um parâmetro.");
                return;
            }
            if (!momentosInicializados)
            {
                mediaMovelGradientes.assign(parametros.size(), 0.0f);
                mediaMovelGradientesQuadrados.assign(parametros.size(), 0.0f);
                momentosInicializados = true;
            }
            else if (parametros.size() != mediaMovelGradientes.size())
            {
                Serial.println("Erro: a quantidade de parâmetros mudou; reinicialize o otimizador.");
                return;
            }

            numeroAtualizacoes++;

            float correcaoPrimeiroMomento = 1.0f - powf(decaimentoPrimeiroMomento, (float)numeroAtualizacoes);
            float correcaoSegundoMomento = 1.0f - powf(decaimentoSegundoMomento, (float)numeroAtualizacoes);
            for (std::size_t indiceParametro = 0; indiceParametro < parametros.size(); indiceParametro++)
            {
                float gradienteMedio = gradientes[indiceParametro] / (float)numeroAmostrasLote;
                mediaMovelGradientes[indiceParametro] =
                    decaimentoPrimeiroMomento * mediaMovelGradientes[indiceParametro] +
                    (1.0f - decaimentoPrimeiroMomento) * gradienteMedio;
                mediaMovelGradientesQuadrados[indiceParametro] =
                    decaimentoSegundoMomento * mediaMovelGradientesQuadrados[indiceParametro] +
                    (1.0f - decaimentoSegundoMomento) * gradienteMedio * gradienteMedio;

                float primeiroMomentoCorrigido = mediaMovelGradientes[indiceParametro] / correcaoPrimeiroMomento;
                float segundoMomentoCorrigido = mediaMovelGradientesQuadrados[indiceParametro] / correcaoSegundoMomento;
                parametros[indiceParametro] -=
                    taxaAprendizado * primeiroMomentoCorrigido /
                    (sqrtf(segundoMomentoCorrigido) + margemEstabilidadeNumerica);
            }
        }

        // Os dados privados mantêm um estado independente para cada parâmetro.
    private:
        // Armazena os momentos de cada parâmetro e indica se suas dimensões já foram estabelecidas.
        std::vector<float> mediaMovelGradientes;
        std::vector<float> mediaMovelGradientesQuadrados;
        bool momentosInicializados = false;
    };
}
