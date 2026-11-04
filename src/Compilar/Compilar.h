#pragma once
#include "../Neuronios/Neuronios.h"
#include "../Adam/Adam.h"

// Reúne os neurônios e o otimizador usados pelo treinamento de baixo nível.
namespace Compilar
{
    // Configura uma camada linear com dimensões variáveis e seu otimizador.
    class Compilar
    {
        // Os métodos públicos configuram os parâmetros treináveis.
    public:
        // Expõe os pesos e vieses da camada que será treinada.
        Neuronios::Neuronios neuronios;
        // Mantém os momentos de Adam alinhados aos parâmetros da camada.
        Adam::Adam otimizador;

        // Configura as dimensões da camada e os parâmetros de Adam.
        bool configurar(
            int quantidadeEntradas,
            int quantidadeSaidas,
            float taxaAprendizado = 0.05f,
            float decaimentoPrimeiroMomento = 0.9f,
            float decaimentoSegundoMomento = 0.999f,
            float margemEstabilidadeNumerica = 1e-7f)
        {
            if (quantidadeEntradas < 1 || quantidadeSaidas < 1 ||
                quantidadeSaidas > Neuronios::Neuronios::NUMERO_NEURONIOS_MAXIMO)
            {
                Serial.println("Erro: quantidade de entradas ou saídas fora dos limites suportados.");
                return false;
            }
            neuronios.configurar(quantidadeEntradas, quantidadeSaidas);
            neuronios.inicializar();
            otimizador.inicializar(
                taxaAprendizado,
                decaimentoPrimeiroMomento,
                decaimentoSegundoMomento,
                margemEstabilidadeNumerica);
            return true;
        }
    };
}