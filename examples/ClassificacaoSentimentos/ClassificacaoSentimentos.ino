#include <Seai.h>
#include <Conversor/Conversor.h>
#include <ctype.h>
#include <string.h>
#include <vector>

// Use palavras ASCII sem acentos neste exemplo de tokenizacao simples....
// O limite deve acomodar o maior termo e também é usado nas larguras do Conversor....
constexpr size_t TAMANHO_PALAVRA = 24;
// Cada termo vira uma característica binária na entrada fornecida à rede....
const char *vocabulario[] = {
    "bom", "otimo", "feliz", "gostei",
    "ruim", "pessimo", "triste", "odiei",
    "normal", "comum", "talvez", "indiferente"
};
constexpr size_t QUANTIDADE_PALAVRAS =
    sizeof(vocabulario) / sizeof(vocabulario[0]);

// Mantém os tokens em buffers de largura fixa durante a normalização....
Seai::Conversor conversorTexto(Seai::Conversor::STRING);
// Nomes exibidos para os índices 0, 1 e 2 previstos pelo modelo....
const char *nomesClasses[] = {"POSITIVO", "NEGATIVO", "NEUTRO"};

// Frases de treino exemplificam as classes positiva, negativa e neutra....
const char *frasesTreino[] = {
    "bom", "otimo", "feliz", "gostei",
    "muito bom", "estou feliz", "gostei muito", "resultado otimo",
    "ruim", "pessimo", "triste", "odiei",
    "muito ruim", "estou triste", "odiei isso", "resultado pessimo",
    "normal", "comum", "talvez", "indiferente",
    "tudo normal", "resultado comum", "talvez sim", "estou indiferente"
};

// Cada rótulo aponta para a classe da frase na mesma posição do vetor de treino....
const uint8_t classesTreino[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2
};

// Converte uma palavra ASCII para minúsculas e grava o resultado no buffer fornecido....
// Retorna false se o token ultrapassar TAMANHO_PALAVRA ou se o Conversor falhar....
bool normalizarPalavra(const char *palavra, char palavraConvertida[TAMANHO_PALAVRA])
{
    uint8_t entrada[TAMANHO_PALAVRA] = {};
    uint8_t saida[TAMANHO_PALAVRA] = {};
    size_t tamanho = strlen(palavra);
    if (tamanho >= TAMANHO_PALAVRA)
    {
        return false;
    }
    memcpy(entrada, palavra, tamanho);
    if (!conversorTexto.converterDado(
            Seai::Conversor::STRING,
            entrada,
            saida,
            1,
            TAMANHO_PALAVRA,
            TAMANHO_PALAVRA))
    {
        return false;
    }
    memcpy(palavraConvertida, saida, TAMANHO_PALAVRA);
    for (size_t indice = 0; palavraConvertida[indice] != '\0'; indice++)
    {
        palavraConvertida[indice] =
            (char)tolower((unsigned char)palavraConvertida[indice]);
    }
    return true;
}

// Codifica a frase como vetor de presença, ignorando palavras ausentes do vocabulário....
// O vetor de saída recebe QUANTIDADE_PALAVRAS valores; palavras acima do limite causam false....
bool codificarFrase(const char *frase, std::vector<float> &vetor)
{
    vetor.assign(QUANTIDADE_PALAVRAS, 0.0f);
    char palavra[TAMANHO_PALAVRA] = {};
    size_t tamanhoPalavra = 0;

    for (size_t indice = 0; ; indice++)
    {
        unsigned char caractere = (unsigned char)frase[indice];
        if (caractere == '\0' || !isalpha(caractere))
        {
            if (tamanhoPalavra > 0)
            {
                palavra[tamanhoPalavra] = '\0';
                char palavraConvertida[TAMANHO_PALAVRA] = {};
                if (!normalizarPalavra(palavra, palavraConvertida))
                {
                    return false;
                }
                for (size_t indiceVocabulario = 0;
                     indiceVocabulario < QUANTIDADE_PALAVRAS;
                     indiceVocabulario++)
                {
                    if (strcmp(palavraConvertida, vocabulario[indiceVocabulario]) == 0)
                    {
                        vetor[indiceVocabulario] = 1.0f;
                        break;
                    }
                }
                memset(palavra, 0, sizeof(palavra));
                tamanhoPalavra = 0;
            }
            if (caractere == '\0')
            {
                break;
            }
        }
        else
        {
            if (tamanhoPalavra >= TAMANHO_PALAVRA - 1)
            {
                return false;
            }
            palavra[tamanhoPalavra++] = (char)tolower(caractere);
        }
    }
    return true;
}

// Prepara os dados one-hot, treina a rede e informa a classe prevista para cada frase de teste....
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Classificador de sentimentos ===");

    // Cria uma entrada por frase e um alvo one-hot correspondente à classe rotulada....
    std::vector<std::vector<float>> entradasTreino;
    std::vector<std::vector<float>> saidasTreino;
    for (size_t indice = 0; indice < sizeof(frasesTreino) / sizeof(frasesTreino[0]); indice++)
    {
        std::vector<float> vetor;
        if (!codificarFrase(frasesTreino[indice], vetor))
        {
            Serial.println("Falha ao converter frase de treino.");
            return;
        }
        std::vector<float> classe(3, 0.0f);
        classe[classesTreino[indice]] = 1.0f;
        entradasTreino.push_back(vetor);
        saidasTreino.push_back(classe);
    }

    // Define os hiperparâmetros e o número de neurônios ocultos para a demonstração....
    Seai::Modelo modelo;
    modelo.epocas = 1800;
    modelo.tamanhoLote = 8;
    modelo.taxaAprendizado = 0.025f;
    modelo.neuroniosPorCamada = {8};
    if (!modelo.executar(entradasTreino, saidasTreino, entradasTreino[0]))
    {
        Serial.println("Falha ao treinar o classificador.");
        return;
    }

    // As frases de teste não participam do treino e usam o mesmo vocabulário....
    const char *frasesTeste[] = {
        "gostei muito",
        "odiei isso",
        "talvez seja normal"
    };

    for (size_t indice = 0; indice < sizeof(frasesTeste) / sizeof(frasesTeste[0]); indice++)
    {
        std::vector<float> vetor;
        if (!codificarFrase(frasesTeste[indice], vetor))
        {
            Serial.println("Falha ao converter frase de teste.");
            continue;
        }
        std::vector<float> previsao = modelo.prever(vetor);
        if (previsao.size() != 3)
        {
            Serial.println("Falha na previsao.");
            continue;
        }
        size_t classePrevista = 0;
        for (size_t classe = 1; classe < previsao.size(); classe++)
        {
            if (previsao[classe] > previsao[classePrevista])
            {
                classePrevista = classe;
            }
        }
        Serial.printf("\"%s\" -> %s\n", frasesTeste[indice], nomesClasses[classePrevista]);
    }
}

// A inferência de demonstração é executada uma única vez em setup()....
void loop()
{
}
