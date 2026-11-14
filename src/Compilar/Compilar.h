#pragma once
#include "../Neuronios/Neuronios.h"
#include "../Adam/Adam.h"

// Namespace Seai reúne a configuração de camadas e do otimizador associado....
namespace Seai
{
    // Configura uma camada linear com dimensões variáveis e seu otimizador....
    class Compilar
    {
        // Os métodos públicos configuram os parâmetros treináveis....
    public:
        // Expõe os pesos e vieses da camada que será treinada....
        Neuronios neuronios;
        // Mantém os momentos de Adam alinhados aos parâmetros da camada....
        Adam otimizador;

        // Configura uma camada totalmente conectada, inicializa seus parâmetros e prepara o otimizador Adam....
        // quantidadeEntradas e quantidadeSaidas definem a matriz de pesos; a saída aceita de 1 a 64 neurônios....
        // Os parâmetros seguintes ajustam a taxa, os dois decaimentos de Adam e a margem numérica....
        // Retorna false se as dimensões forem inválidas; em caso de sucesso, neuronios e otimizador ficam prontos....
        bool configurar(
            int quantidadeEntradas,
            int quantidadeSaidas,
            float taxaAprendizado = 0.05f,
            float decaimentoPrimeiroMomento = 0.9f,
            float decaimentoSegundoMomento = 0.999f,
            float margemEstabilidadeNumerica = 1e-7f)
        {
            if (quantidadeEntradas < 1 || quantidadeSaidas < 1 ||
                quantidadeSaidas > Neuronios::NUMERO_NEURONIOS_MAXIMO)
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