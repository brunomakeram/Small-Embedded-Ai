#pragma once
#include <vector>
#include "../Neuronios/Neuronios.h"

// Namespace Seai reúne as camadas que formam a sequência da rede neural....
namespace Seai {
    // Mantém a sequência de camadas sem duplicar os parâmetros ou a lógica dos neurônios....
    class Camadas {
    private:
        // Cada elemento representa uma camada configurada pela classe Neuronios....
        std::vector<Neuronios> camadas;

    public:
        // Cria, inicializa e acrescenta uma camada ao final da arquitetura....
        // quantidadeEntradas determina o tamanho de cada vetor de pesos; quantidadeNeuronios determina quantos neurônios serão criados....
        void adicionar(int quantidadeEntradas, int quantidadeNeuronios) {
            Neuronios camada;
            camada.configurar(quantidadeEntradas, quantidadeNeuronios);
            camada.inicializar();
            camadas.push_back(camada);
        }

        // Remove todas as camadas e seus parâmetros, deixando a arquitetura vazia....
        void limpar() {
            camadas.clear();
        }

        // Devolve o número de camadas atualmente armazenadas....
        std::size_t tamanho() const {
            return camadas.size();
        }

        // Permite ler ou alterar uma camada pelo índice de inserção, começando em zero....
        // O índice deve estar entre zero e tamanho() - 1....
        Neuronios &operator[](std::size_t indice) {
            return camadas[indice];
        }

        // Devolve acesso somente para leitura à camada na posição indicada....
        // O índice deve estar entre zero e tamanho() - 1; a referência não permite alterar os parâmetros....
        const Neuronios &operator[](std::size_t indice) const {
            return camadas[indice];
        }
    };
}