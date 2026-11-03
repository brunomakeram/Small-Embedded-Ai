#pragma once
#include <Arduino.h>
#include <vector>
#include "../Camadas/Camadas.h"
#include "../Neuronios/Neuronios.h"
#include "../Verbose/Verbose.h"

namespace Modelo
{
    // Coordena o treinamento genérico e a previsão para vetores de qualquer dimensão válida.
    class Modelo
    {
    public:
        // Define o tamanho das atualizações do Adam; valores maiores alteram os parâmetros mais rapidamente.
        float taxaAprendizado = 0.05f;
        // Define quantas vezes todo o conjunto de amostras será percorrido.
        int epocas = 500;
        // Define quantas amostras contribuem para cada atualização dos parâmetros.
        int tamanhoLote = 32;
        // Cada valor configura uma camada oculta; a camada de saída usa a dimensão dos alvos.
        std::vector<int> neuroniosPorCamada = {Neuronios::Neuronios::NUMERO_NEURONIOS_PADRAO};
        // Mantém as opções de log e exibe a perda final após a previsão.
        Verbose::Verbose verbose;

        // Treina usando matrizes de amostras e prevê a saída correspondente à entrada informada.
        // Cada linha de entradas e saidasEsperadas forma um par de treino; entradaPrevisao é avaliada após o ajuste.
        bool executar(
            const std::vector<std::vector<float>> &entradas,
            const std::vector<std::vector<float>> &saidasEsperadas,
            const std::vector<float> &entradaPrevisao)
        {
            // Impede o início do treino quando as amostras, dimensões ou parâmetros são incompatíveis.
            if (!validarDados(entradas, saidasEsperadas, entradaPrevisao))
            {
                return false;
            }

            // Recria a arquitetura para corresponder às dimensões das amostras recebidas.
            camadas.limpar();
            // A largura da primeira camada vem do número de características de cada entrada.
            int quantidadeEntradas = entradas[0].size();
            // Encadeia as camadas ocultas, usando a saída de cada camada como entrada da próxima.
            for (std::size_t indiceCamada = 0; indiceCamada < neuroniosPorCamada.size(); indiceCamada++)
            {
                camadas.adicionar(quantidadeEntradas, neuroniosPorCamada[indiceCamada]);
                quantidadeEntradas = neuroniosPorCamada[indiceCamada];
            }
            // Define a dimensão da última camada pelo número de valores de saída nos alvos.
            camadas.adicionar(quantidadeEntradas, saidasEsperadas[0].size());
            verbose.registrarConfiguracao(
                taxaAprendizado,
                epocas,
                tamanhoLote,
                neuroniosPorCamada,
                saidasEsperadas[0].size());

            // Inicializa os momentos do otimizador Adam para cada peso e viés.
            std::vector<std::vector<std::vector<float>>> primeiroMomentoPesos;
            std::vector<std::vector<std::vector<float>>> segundoMomentoPesos;
            std::vector<std::vector<float>> primeiroMomentoVieses;
            std::vector<std::vector<float>> segundoMomentoVieses;
            inicializarMomentos(primeiroMomentoPesos, segundoMomentoPesos, primeiroMomentoVieses, segundoMomentoVieses);

            // Usa os parâmetros padrão do Adam para manter estáveis as atualizações.
            // beta1 e beta2 controlam as médias móveis; epsilon evita divisão instável quando a variância é pequena.
            const float beta1 = 0.9f;
            const float beta2 = 0.999f;
            const float epsilon = 1e-7f;
            // Conta atualizações de lote para corrigir o viés das médias móveis do Adam.
            long atualizacao = 0;
            // Recebe a perda calculada depois do treinamento completo.
            float perdaFinal = 0.0f;
            // Mantém o lote positivo para que o incremento do laço não seja zero ou negativo.
            int lote = tamanhoLote < 1 ? 1 : tamanhoLote;
            // Armazena uma permutação de índices para embaralhar sem reorganizar as matrizes de entrada.
            std::vector<std::size_t> indices(entradas.size());
            // Inicia a permutação na ordem original das amostras.
            for (std::size_t indice = 0; indice < indices.size(); indice++)
            {
                indices[indice] = indice;
            }

            // Percorre as épocas e atualiza os parâmetros em lotes de amostras.
            for (int indiceEpoca = 0; indiceEpoca < epocas; indiceEpoca++)
            {
                // Embaralha a ordem das amostras para reduzir a dependência da ordenação do dataset.
                for (std::size_t indice = indices.size() - 1; indice > 0; indice--)
                {
                    std::size_t indiceAleatorio = esp_random() % (indice + 1);
                    std::size_t temporario = indices[indice];
                    indices[indice] = indices[indiceAleatorio];
                    indices[indiceAleatorio] = temporario;
                }

                // Cria acumuladores de gradiente independentes para o lote atual.
                for (std::size_t inicioLote = 0; inicioLote < entradas.size(); inicioLote += lote)
                {
                    std::vector<std::vector<std::vector<float>>> gradientesPesos;
                    std::vector<std::vector<float>> gradientesVieses;
                    inicializarGradientes(gradientesPesos, gradientesVieses);

                    // Calcula o fim do lote sem ultrapassar o total de amostras.
                    std::size_t fimLote = inicioLote + lote;
                    if (fimLote > entradas.size())
                    {
                        fimLote = entradas.size();
                    }

                    // Executa a propagação direta e acumula gradientes para cada amostra do lote.
                    for (std::size_t indiceAmostraLote = inicioLote; indiceAmostraLote < fimLote; indiceAmostraLote++)
                    {
                        std::size_t indiceAmostra = indices[indiceAmostraLote];
                        std::vector<std::vector<float>> ativacoes;
                        // Guarda a entrada e a saída de cada camada para reutilizá-las no cálculo dos gradientes.
                        propagar(entradas[indiceAmostra], ativacoes);

                        // Reserva um vetor de derivadas por camada para propagar o erro para trás.
                        std::vector<std::vector<float>> deltas(camadas.tamanho());
                        std::size_t indiceUltimaCamada = camadas.tamanho() - 1;
                        const std::vector<float> &previsao = ativacoes.back();
                        deltas[indiceUltimaCamada].resize(previsao.size());

                        // A derivada da perda quadrática em relação à saída é o erro dividido pelo número de saídas.
                        for (std::size_t indiceSaida = 0; indiceSaida < previsao.size(); indiceSaida++)
                        {
                            float erro = previsao[indiceSaida] - saidasEsperadas[indiceAmostra][indiceSaida];
                            deltas[indiceUltimaCamada][indiceSaida] = erro / previsao.size();
                        }

                        // Propaga os gradientes pelas camadas ocultas usando a derivada de tanh.
                        for (int indiceCamada = (int)indiceUltimaCamada - 1; indiceCamada >= 0; indiceCamada--)
                        {
                            deltas[indiceCamada].resize(camadas[indiceCamada].numeroNeuronios);
                            for (int indiceNeuronio = 0; indiceNeuronio < camadas[indiceCamada].numeroNeuronios; indiceNeuronio++)
                            {
                                float somaGradientes = 0.0f;
                                for (int indiceProximo = 0; indiceProximo < camadas[indiceCamada + 1].numeroNeuronios; indiceProximo++)
                                {
                                    somaGradientes += camadas[indiceCamada + 1].pesosCamada[indiceProximo][indiceNeuronio] * deltas[indiceCamada + 1][indiceProximo];
                                }
                                float ativacao = ativacoes[indiceCamada + 1][indiceNeuronio];
                                deltas[indiceCamada][indiceNeuronio] = somaGradientes * (1.0f - ativacao * ativacao);
                            }
                        }

                        for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
                        {
                            // Usa a ativação que alimentou a camada para calcular os gradientes dos seus pesos.
                            const std::vector<float> &entradasCamada = ativacoes[indiceCamada];
                            for (int indiceNeuronio = 0; indiceNeuronio < camadas[indiceCamada].numeroNeuronios; indiceNeuronio++)
                            {
                                gradientesVieses[indiceCamada][indiceNeuronio] += deltas[indiceCamada][indiceNeuronio];
                                for (int indiceEntrada = 0; indiceEntrada < camadas[indiceCamada].numeroEntradas; indiceEntrada++)
                                {
                                    gradientesPesos[indiceCamada][indiceNeuronio][indiceEntrada] +=
                                        deltas[indiceCamada][indiceNeuronio] * entradasCamada[indiceEntrada];
                                }
                            }

                            // O índice usa a época corrente; como a chamada fica neste ponto do laço, ela é avaliada a cada passagem por uma camada da amostra.
                            verbose.registrarEpoca(indiceEpoca + 1);
                            if (verbose.mostrarPerdaPorEpoca)
                            {
                                verbose.registrarPerdaEpoca(calcularPerda(entradas, saidasEsperadas));
                            }
                        }
                    }

                    // Aplica Adam aos gradientes médios do lote.
                    atualizacao++;
                    // Corrige o viés inicial das médias móveis para a atualização corrente.
                    float correcaoPrimeiroMomento = 1.0f - powf(beta1, (float)atualizacao);
                    float correcaoSegundoMomento = 1.0f - powf(beta2, (float)atualizacao);
                    // Divide os gradientes acumulados pela quantidade real de amostras deste lote.
                    float quantidadeAmostrasLote = (float)(fimLote - inicioLote);

                    // Atualiza todos os pesos e vieses, mantendo momentos distintos por parâmetro.
                    for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
                    {
                        for (int indiceNeuronio = 0; indiceNeuronio < camadas[indiceCamada].numeroNeuronios; indiceNeuronio++)
                        {
                            for (int indiceEntrada = 0; indiceEntrada < camadas[indiceCamada].numeroEntradas; indiceEntrada++)
                            {
                                atualizarParametro(
                                    camadas[indiceCamada].pesosCamada[indiceNeuronio][indiceEntrada],
                                    gradientesPesos[indiceCamada][indiceNeuronio][indiceEntrada] / quantidadeAmostrasLote,
                                    primeiroMomentoPesos[indiceCamada][indiceNeuronio][indiceEntrada],
                                    segundoMomentoPesos[indiceCamada][indiceNeuronio][indiceEntrada],
                                    correcaoPrimeiroMomento,
                                    correcaoSegundoMomento,
                                    beta1,
                                    beta2,
                                    epsilon);
                            }
                            atualizarParametro(
                                camadas[indiceCamada].viesCamada[indiceNeuronio],
                                gradientesVieses[indiceCamada][indiceNeuronio] / quantidadeAmostrasLote,
                                primeiroMomentoVieses[indiceCamada][indiceNeuronio],
                                segundoMomentoVieses[indiceCamada][indiceNeuronio],
                                correcaoPrimeiroMomento,
                                correcaoSegundoMomento,
                                beta1,
                                beta2,
                                epsilon);
                        }
                    }
                }
            }

            // Recalcula a perda com os parâmetros finais, evitando reportar valores intermediários dos lotes.
            perdaFinal = calcularPerda(entradas, saidasEsperadas);

            // Prevê a saída solicitada e mantém a apresentação dos resultados pelo módulo Verbose.
            std::vector<std::vector<float>> ativacoesPrevisao;
            propagar(entradaPrevisao, ativacoesPrevisao);
            verbose.mostrarResultados(entradaPrevisao, perdaFinal, ativacoesPrevisao.back());
            return true;
        }

    private:
        // Armazena as camadas da rede sem repetir a estrutura dos neurônios.
        Camadas::Camadas camadas;

        // Confere dimensões e limites antes de alocar pesos ou iniciar o treinamento.
        // Retorna false e informa a primeira inconsistência encontrada pelo Serial.
        // entradas e saidasEsperadas são as matrizes de treino; entradaPrevisao precisa ter a largura de uma entrada.
        bool validarDados(
            const std::vector<std::vector<float>> &entradas,
            const std::vector<std::vector<float>> &saidasEsperadas,
            const std::vector<float> &entradaPrevisao) const
        {
            // Exige pares de entrada e alvo para cada amostra.
            if (entradas.empty() || saidasEsperadas.empty() || entradas.size() != saidasEsperadas.size())
            {
                Serial.println("Erro: entradas e saídas devem conter o mesmo número de amostras.");
                return false;
            }
            // A primeira amostra define as dimensões que todas as demais e a consulta devem respeitar.
            if (entradas[0].empty() || saidasEsperadas[0].empty() || entradaPrevisao.size() != entradas[0].size())
            {
                Serial.println("Erro: as dimensões das entradas ou da entrada de previsão são inválidas.");
                return false;
            }
            // Evita executar o otimizador com quantidade de épocas ou taxa de aprendizado inválida.
            if (epocas < 1 || !isfinite(taxaAprendizado) || taxaAprendizado <= 0.0f)
            {
                Serial.println("Erro: épocas e taxa de aprendizado devem ser maiores que zero.");
                return false;
            }
            // A camada de saída também usa Neuronios e, por isso, segue o limite definido por esse módulo.
            if (saidasEsperadas[0].size() > Neuronios::Neuronios::NUMERO_NEURONIOS_MAXIMO)
            {
                Serial.println("Erro: a camada de saída aceita no máximo 64 neurônios.");
                return false;
            }
            // Confere que cada largura configurada para uma camada oculta está dentro dos limites suportados.
            for (std::size_t indiceCamada = 0; indiceCamada < neuroniosPorCamada.size(); indiceCamada++)
            {
                if (neuroniosPorCamada[indiceCamada] < Neuronios::Neuronios::NUMERO_NEURONIOS_MINIMO || neuroniosPorCamada[indiceCamada] > Neuronios::Neuronios::NUMERO_NEURONIOS_MAXIMO)
                {
                    Serial.println("Erro: cada camada deve ter entre 1 e 64 neurônios.");
                    return false;
                }
            }
            // Verifica valores finitos e dimensões uniformes em todas as amostras.
            for (std::size_t indiceAmostra = 0; indiceAmostra < entradas.size(); indiceAmostra++)
            {
                for (std::size_t indiceEntrada = 0; indiceEntrada < entradas[indiceAmostra].size(); indiceEntrada++)
                {
                    if (!isfinite(entradas[indiceAmostra][indiceEntrada]))
                    {
                        Serial.println("Erro: as entradas devem conter apenas valores finitos.");
                        return false;
                    }
                }
                for (std::size_t indiceSaida = 0; indiceSaida < saidasEsperadas[indiceAmostra].size(); indiceSaida++)
                {
                    if (!isfinite(saidasEsperadas[indiceAmostra][indiceSaida]))
                    {
                        Serial.println("Erro: as saídas devem conter apenas valores finitos.");
                        return false;
                    }
                }
                if (entradas[indiceAmostra].size() != entradas[0].size() || saidasEsperadas[indiceAmostra].size() != saidasEsperadas[0].size())
                {
                    Serial.println("Erro: todas as amostras devem ter dimensões consistentes.");
                    return false;
                }
            }
            // A consulta precisa conter apenas valores finitos e uma característica por entrada de treino.
            for (std::size_t indiceEntrada = 0; indiceEntrada < entradaPrevisao.size(); indiceEntrada++)
            {
                if (!isfinite(entradaPrevisao[indiceEntrada]))
                {
                    Serial.println("Erro: a entrada de previsão deve conter apenas valores finitos.");
                    return false;
                }
            }
            return true;
        }

        // Calcula a ativação de cada camada e guarda os resultados necessários ao backpropagation.
        // entrada é o vetor inicial; ativacoes recebe a entrada e a saída de cada camada na ordem do fluxo.
        void propagar(const std::vector<float> &entrada, std::vector<std::vector<float>> &ativacoes) const
        {
            ativacoes.clear();
            ativacoes.push_back(entrada);
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                // Guarda em um vetor temporário a saída da camada antes de usá-la como próxima entrada.
                std::vector<float> saidaCamada;
                // Usa tanh nas camadas ocultas; a camada de saída permanece linear para permitir regressão.
                bool ativacaoTanh = indiceCamada + 1 < camadas.tamanho();
                camadas[indiceCamada].calcular(ativacoes.back(), saidaCamada, ativacaoTanh);
                ativacoes.push_back(saidaCamada);
            }
        }

        // Cria os vetores de estado do Adam com o mesmo formato dos pesos e vieses.
        // Cada argumento de saída recebe uma estrutura zerada alinhada às dimensões atuais das camadas.
        // Os dois primeiros argumentos recebem momentos dos pesos; os dois últimos recebem momentos dos vieses.
        void inicializarMomentos(
            std::vector<std::vector<std::vector<float>>> &primeiroMomentoPesos,
            std::vector<std::vector<std::vector<float>>> &segundoMomentoPesos,
            std::vector<std::vector<float>> &primeiroMomentoVieses,
            std::vector<std::vector<float>> &segundoMomentoVieses) const
        {
            // Descarta estados anteriores antes de reconstruí-los para a arquitetura atual.
            primeiroMomentoPesos.clear();
            segundoMomentoPesos.clear();
            primeiroMomentoVieses.clear();
            segundoMomentoVieses.clear();
            // Aloca uma matriz de momentos por camada e um valor por peso ou viés.
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                int quantidadeNeuronios = camadas[indiceCamada].numeroNeuronios;
                int quantidadeEntradas = camadas[indiceCamada].numeroEntradas;
                primeiroMomentoPesos.push_back(
                    std::vector<std::vector<float>>(quantidadeNeuronios, std::vector<float>(quantidadeEntradas, 0.0f)));
                segundoMomentoPesos.push_back(
                    std::vector<std::vector<float>>(quantidadeNeuronios, std::vector<float>(quantidadeEntradas, 0.0f)));
                primeiroMomentoVieses.push_back(std::vector<float>(quantidadeNeuronios, 0.0f));
                segundoMomentoVieses.push_back(std::vector<float>(quantidadeNeuronios, 0.0f));
            }
        }

        // Aloca gradientes zerados para o lote sem copiar os estados persistentes do otimizador.
        // Os vetores de saída acumulam derivadas e mantêm a forma dos parâmetros da rede.
        // gradientesPesos e gradientesVieses são preenchidos com um acumulador para cada parâmetro.
        void inicializarGradientes(
            std::vector<std::vector<std::vector<float>>> &gradientesPesos,
            std::vector<std::vector<float>> &gradientesVieses) const
        {
            // Inicializa um acumulador de gradiente para cada peso e viés de cada camada.
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                int quantidadeNeuronios = camadas[indiceCamada].numeroNeuronios;
                int quantidadeEntradas = camadas[indiceCamada].numeroEntradas;
                gradientesPesos.push_back(
                    std::vector<std::vector<float>>(quantidadeNeuronios, std::vector<float>(quantidadeEntradas, 0.0f)));
                gradientesVieses.push_back(std::vector<float>(quantidadeNeuronios, 0.0f));
            }
        }

        // Avalia o erro quadrático médio dos dados com os parâmetros atuais.
        // As entradas e saídasEsperadas devem ter o mesmo número e a mesma largura por amostra.
        // O retorno é a média do erro quadrático entre amostras e saídas.
        float calcularPerda(
            const std::vector<std::vector<float>> &entradas,
            const std::vector<std::vector<float>> &saidasEsperadas) const
        {
            // Soma os erros de todas as saídas de todas as amostras antes de normalizar.
            float somaPerdas = 0.0f;
            for (std::size_t indiceAmostra = 0; indiceAmostra < entradas.size(); indiceAmostra++)
            {
                std::vector<std::vector<float>> ativacoes;
                propagar(entradas[indiceAmostra], ativacoes);
                for (std::size_t indiceSaida = 0; indiceSaida < saidasEsperadas[indiceAmostra].size(); indiceSaida++)
                {
                    // Divide por saída para que a contribuição de cada amostra seja sua MSE.
                    float erro = ativacoes.back()[indiceSaida] - saidasEsperadas[indiceAmostra][indiceSaida];
                    somaPerdas += erro * erro / saidasEsperadas[indiceAmostra].size();
                }
            }
            return somaPerdas / entradas.size();
        }

        // Atualiza um parâmetro e seus momentos móveis usando a regra do Adam.
        // parametro é alterado; gradiente vem do lote; os momentos e correções mantêm o estado do Adam.
        // beta1 e beta2 definem os decaimentos e epsilon estabiliza a divisão pelo segundo momento.
        void atualizarParametro(
            float &parametro,
            float gradiente,
            float &primeiroMomento,
            float &segundoMomento,
            float correcaoPrimeiroMomento,
            float correcaoSegundoMomento,
            float beta1,
            float beta2,
            float epsilon) const
        {
            // Atualiza a média móvel do gradiente e a média móvel de seu quadrado.
            primeiroMomento = beta1 * primeiroMomento + (1.0f - beta1) * gradiente;
            segundoMomento = beta2 * segundoMomento + (1.0f - beta2) * gradiente * gradiente;
            // Corrige o viés dos momentos e aplica a taxa de aprendizado ao parâmetro.
            float primeiroCorrigido = primeiroMomento / correcaoPrimeiroMomento;
            float segundoCorrigido = segundoMomento / correcaoSegundoMomento;
            parametro -= taxaAprendizado * primeiroCorrigido / (sqrtf(segundoCorrigido) + epsilon);
        }
    };
}
