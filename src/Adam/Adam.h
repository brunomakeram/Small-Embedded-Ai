// Evita gravação múltipla do arquivo de cabeçalho
#pragma once
// Inclui a biblioteca Arduino.h para acesso às funcionalidades do Arduino
#include <Arduino.h>

// Define o namespace Adam para encapsular a classe Adam
namespace Adam
{
    // Define a classe Adam que implementa o algoritmo de otimização Adam para atualização dos pesos da rede neural
    class Adam
    {
        // Os métodos públicos podem ser acessados de fora da classe
    public:
        // Tamanho do passo que o modelo dá ao ajustar os pesos durante o treinamento; valores maiores podem acelerar o aprendizado, mas também podem causar instabilidade
        float taxaAprendizado;
        // Fatores de decaimento para os momentos do gradiente; valores próximos a 1.0 significam que o modelo "lembra" mais das atualizações anteriores, enquanto valores menores dão mais peso às atualizações recentes
        float decaimentoPrimeiroMomento;
        // Fatores de decaimento para os momentos do gradiente; valores próximos a 1.0 significam que o modelo "lembra" mais das atualizações anteriores, enquanto valores menores dão mais peso às atualizações recentes
        float decaimentoSegundoMomento;
        // Pequena constante adicionada para evitar divisão por zero durante a atualização dos pesos; valores muito pequenos podem causar instabilidade numérica, enquanto valores muito grandes podem retardar o aprendizado
        float margemEstabilidadeNumerica;
        // Contador de quantas vezes os parâmetros foram atualizados; usado para calcular as correções de viés nos momentos do gradiente
        long numeroAtualizacoes;
        // Vetores que armazenam as médias móveis dos gradientes e dos quadrados dos gradientes; usados para calcular as atualizações dos pesos de forma adaptativa
        float mediaMovelGradientes[3];
        // Vetores que armazenam as médias móveis dos gradientes e dos quadrados dos gradientes; usados para calcular as atualizações dos pesos de forma adaptativa
        float mediaMovelGradientesQuadrados[3];

        // Construtor da classe Adam que inicializa os parâmetros do otimizador
        void inicializar(float lr = 0.05f, float beta1 = 0.9f, float beta2 = 0.999f, float eps = 1e-7f)
        {
            // lr é a taxa de aprendizado, que controla o tamanho do passo que o modelo dá ao ajustar os pesos durante o treinamento
            taxaAprendizado = lr;
            // Beta1 é o fator de decaimento para o primeiro momento (média móvel dos gradientes); valores próximos a 1.0 significam que o modelo "lembra" mais das atualizações anteriores, enquanto valores menores dão mais peso às atualizações recentes
            decaimentoPrimeiroMomento = beta1;
            // Beta2 é o fator de decaimento para o segundo momento (média móvel dos quadrados dos gradientes); valores próximos a 1.0 significam que o modelo "lembra" mais das atualizações anteriores, enquanto valores menores dão mais peso às atualizações recentes
            decaimentoSegundoMomento = beta2;
            // eps é a pequena constante adicionada para evitar divisão por zero durante a atualização dos pesos; valores muito pequenos podem causar instabilidade numérica, enquanto valores muito grandes podem retardar o aprendizado
            margemEstabilidadeNumerica = eps;
            // Inicializa o contador de atualizações
            numeroAtualizacoes = 0;

            // Percorre os vetores de médias móveis dos gradientes e dos quadrados dos gradientes, inicializando todos os elementos com zero
            for (int indiceParametro = 0; indiceParametro < 3; indiceParametro++)
            {
                // Inicializa a média móvel dos gradientes para o parâmetro atual com zero
                mediaMovelGradientes[indiceParametro] = 0.0f;
                // Inicializa a média móvel dos quadrados dos gradientes para o parâmetro atual com zero
                mediaMovelGradientesQuadrados[indiceParametro] = 0.0f;
            }
        }

        // Atualiza os parâmetros (pesos e viés) do modelo com base nos gradientes calculados durante o treinamento; aplica o algoritmo Adam para ajustar os pesos de forma adaptativa, levando em consideração as médias móveis dos gradientes e dos quadrados dos gradientes
        void atualizarParametros(float *pesos, float &vies, float *gradientes, int numeroAmostrasLote)
        { // Se o número de amostras no lote for menor ou igual a zero, não há gradientes para atualizar, então a função retorna sem fazer nada
            if (numeroAmostrasLote <= 0)
                return;
            // Normaliza os gradientes dividindo cada um pelo número de amostras no lote; isso garante que a magnitude das atualizações dos pesos seja proporcional ao tamanho do lote, evitando que lotes maiores causem atualizações desproporcionalmente grandes
            for (int indiceParametro = 0; indiceParametro < 3; indiceParametro++)
            { // Divide o gradiente do parâmetro atual pelo número de amostras no lote para normalizar a atualização
                gradientes[indiceParametro] /= (float)numeroAmostrasLote;
            }
            // Incrementa o contador de atualizações, que é usado para calcular as correções de viés nos momentos do gradiente
            numeroAtualizacoes++;

            // sqrtf é uma função da biblioteca matemática que calcula a raiz quadrada de um número; aqui, é usada para calcular a correção do segundo momento dos gradientes, que ajuda a estabilizar as atualizações dos pesos
            float correcaoSegundoMomento = sqrtf(1.0f - powf(decaimentoSegundoMomento, (float)numeroAtualizacoes));
            // powf é uma função da biblioteca matemática que calcula a potência de um número; aqui, é usada para calcular a correção do primeiro momento dos gradientes, que ajuda a estabilizar as atualizações dos pesos
            float correcaoPrimeiroMomento = 1.0f - powf(decaimentoPrimeiroMomento, (float)numeroAtualizacoes);
            // Calcula a taxa de aprendizado corrigida, que leva em consideração as correções de viés nos momentos do gradiente; isso ajuda a estabilizar as atualizações dos pesos, especialmente nos estágios iniciais do treinamento
            float taxaAprendizadoCorrigida = taxaAprendizado * correcaoSegundoMomento / correcaoPrimeiroMomento;
            // Cria um array de ponteiros para os parâmetros (pesos e viés) que serão atualizados; isso permite iterar sobre os parâmetros de forma genérica, aplicando a mesma lógica de atualização a todos eles
            float *parametros[3] = {&pesos[0], &pesos[1], &vies};

            // Itera sobre cada parâmetro (dois pesos e um viés) para atualizar seus valores com base nos gradientes calculados; aplica o algoritmo Adam para ajustar os pesos de forma adaptativa, levando em consideração as médias móveis dos gradientes e dos quadrados dos gradientes
            for (int indiceParametro = 0; indiceParametro < 3; indiceParametro++)
            {
                // Atualiza a média móvel dos gradientes para o parâmetro atual; isso ajuda a suavizar as atualizações dos pesos, evitando mudanças abruptas que podem desestabilizar o treinamento
                mediaMovelGradientes[indiceParametro] = decaimentoPrimeiroMomento * mediaMovelGradientes[indiceParametro] + (1.0f - decaimentoPrimeiroMomento) * gradientes[indiceParametro];
                // Atualiza a média móvel dos quadrados dos gradientes para o parâmetro atual; isso ajuda a estabilizar as atualizações dos pesos, especialmente quando os gradientes variam muito entre as iterações
                mediaMovelGradientesQuadrados[indiceParametro] = decaimentoSegundoMomento * mediaMovelGradientesQuadrados[indiceParametro] + (1.0f - decaimentoSegundoMomento) * gradientes[indiceParametro] * gradientes[indiceParametro];
                // Calcula o ajuste para o parâmetro atual com base nas médias móveis dos gradientes e dos quadrados dos gradientes
                float ajusteParametro = taxaAprendizadoCorrigida * mediaMovelGradientes[indiceParametro] / (sqrtf(mediaMovelGradientesQuadrados[indiceParametro]) + margemEstabilidadeNumerica);
                // Atualiza o valor do parâmetro atual subtraindo o ajuste calculado; isso aplica a atualização dos pesos de forma adaptativa, levando em consideração as médias móveis dos gradientes e dos quadrados dos gradientes
                *(parametros[indiceParametro]) -= ajusteParametro;
            }
        }
    };
}