#include <Seai.h>
#include <Conversor/Conversor.h>
#include <ctype.h>
#include <string.h>
#include <vector>

// O vocabulario e as frases usam ASCII para simplificar a tokenizacao no ESP32....
// O tamanho fixo limita cada token e precisa ser igual às larguras passadas ao Conversor....
constexpr size_t TAMANHO_PALAVRA = 24;
// Cada posição do vocabulário corresponde a uma característica binária da entrada do modelo....
const char *vocabulario[] = {
    "remedio", "comprimido", "farmacia", "receita", "paracetamol",
    "resgate", "ambulancia", "acidente", "socorro", "ferido",
    "mercadoria", "pacote", "produto", "estoque", "entrega"
};
constexpr size_t QUANTIDADE_PALAVRAS =
    sizeof(vocabulario) / sizeof(vocabulario[0]);

// Conversor normaliza tokens para buffers de largura fixa antes da comparação com o vocabulário....
Seai::Conversor conversorTexto(Seai::Conversor::STRING);

// Os índices previstos são apresentados usando estes nomes de classe....
const char *nomesClasses[] = {"REMEDIO", "RESGATE", "MERCADORIA"};

// Conjunto demonstrativo; cada frase precisa ter o rótulo correspondente em classesTreino....
const char *frasesTreino[] = {
    "remedio para dor", "comprimido na farmacia", "receita de paracetamol",
    "comprar remedio na farmacia", "paracetamol em comprimido",
    "receita para remedio",
    "chamar resgate", "ambulancia no acidente", "socorro para ferido",
    "acidente precisa ambulancia", "pedido de socorro", "resgate de ferido",
    "mercadoria para entrega", "pacote no estoque", "produto para entrega",
    "estoque de mercadoria", "pacote com produto", "entrega de pacote"
};

// Rótulos inteiros por frase: 0 remédio, 1 resgate e 2 mercadoria....
const uint8_t classesTreino[] = {
    0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2
};

// Normaliza uma palavra para minúsculas e copia o resultado para um buffer de tamanho fixo....
// palavra é a entrada terminada em nulo; palavraConvertida deve comportar TAMANHO_PALAVRA bytes....
// Retorna false se a palavra for grande demais ou a conversão de largura fixa falhar....
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

// Converte uma frase em vetor de presença: uma característica por termo conhecido do vocabulário....
// Tokens fora do vocabulário são ignorados; palavras longas demais causam retorno false....
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

// Codifica as frases, treina a rede e classifica exemplos de teste pela maior saída prevista....
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Classificador PLN: remedio/resgate/mercadoria ===");

    // Constrói uma linha de entrada e um vetor one-hot de três saídas para cada frase....
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

    // Configura as épocas, o lote, a taxa e a largura da camada oculta do classificador....
    Seai::Modelo modelo;
    modelo.epocas = 1800;
    modelo.tamanhoLote = 6;
    modelo.taxaAprendizado = 0.025f;
    modelo.neuroniosPorCamada = {10};
    if (!modelo.executar(entradasTreino, saidasTreino, entradasTreino[0]))
    {
        Serial.println("Falha ao treinar o classificador.");
        return;
    }

    // Frases de teste são processadas pelo mesmo codificador usado para os dados de treino....
    const char *frasesTeste[] = {
        "receita de remedio",
        "socorro para acidente",
        "produto para entrega"
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

// Toda a classificação ocorre durante a inicialização; não há processamento contínuo no loop....
void loop()
{
}
