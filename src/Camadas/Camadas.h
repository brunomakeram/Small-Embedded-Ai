#pragma once
#include <vector>
#include "../Neuronios/Neuronios.h"

// Organiza as camadas da rede sem manter uma segunda cópia da lógica de Neuronios.
namespace Camadas {
    // Mantém a sequência de camadas sem duplicar os parâmetros ou a lógica dos neurônios.
    class Camadas {
    private:
        // Cada elemento representa uma camada configurada pela classe Neuronios.
        std::vector<Neuronios::Neuronios> camadas;

    public:
        // Cria e inicializa uma camada com as dimensões informadas.
        // quantidadeEntradas determina o tamanho de cada vetor de pesos e quantidadeNeuronios determina quantos vetores são criados.
        void adicionar(int quantidadeEntradas, int quantidadeNeuronios) {
            Neuronios::Neuronios camada;
            camada.configurar(quantidadeEntradas, quantidadeNeuronios);
            camada.inicializar();
            camadas.push_back(camada);
        }

        // Remove todas as camadas configuradas.
        void limpar() {
            camadas.clear();
        }

        // Informa quantas camadas estão armazenadas.
        // O resultado é usado para percorrer a arquitetura na ordem em que as camadas foram adicionadas.
        std::size_t tamanho() const {
            return camadas.size();
        }

        // Permite acessar uma camada para leitura ou atualização dos parâmetros.
        // O índice corresponde à ordem de inserção em adicionar().
        Neuronios::Neuronios &operator[](std::size_t indice) {
            return camadas[indice];
        }

        // Permite acessar uma camada por uma instância constante do organizador.
        // A referência constante impede alteração dos parâmetros por meio deste acesso.
        const Neuronios::Neuronios &operator[](std::size_t indice) const {
            return camadas[indice];
        }
    };
}