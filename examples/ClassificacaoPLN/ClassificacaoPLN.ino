#include <Seai.h>
#include <Conversor/Conversor.h>
#include <ctype.h>
#include <string.h>
#include <vector>

// O vocabulario e as frases usam ASCII para simplificar a tokenizacao no ESP32.
constexpr size_t TAMANHO_PALAVRA = 24;
const char *vocabulario[] = {
    "remedio", "comprimido", "farmacia", "receita", "paracetamol",
    "resgate", "ambulancia", "acidente", "socorro", "ferido",
    "mercadoria", "pacote", "produto", "estoque", "entrega"
};
constexpr size_t QUANTIDADE_PALAVRAS =
    sizeof(vocabulario) / sizeof(vocabulario[0]);

Conversor::Conversor conversorTexto(Conversor::Conversor::STRING);

const char *nomesClasses[] = {"REMEDIO", "RESGATE", "MERCADORIA"};

const char *frasesTreino[] = {
    "remedio para dor", "comprimido na farmacia", "receita de paracetamol",
    "comprar remedio na farmacia", "paracetamol em comprimido",
    "receita para remedio",
    "chamar resgate", "ambulancia no acidente", "socorro para ferido",
    "acidente precisa ambulancia", "pedido de socorro", "resgate de ferido",
    "mercadoria para entrega", "pacote no estoque", "produto para entrega",
    "estoque de mercadoria", "pacote com produto", "entrega de pacote"
};

const uint8_t classesTreino[] = {
    0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2
};

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
            Conversor::Conversor::STRING,
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

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Classificador PLN: remedio/resgate/mercadoria ===");

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

    Modelo::Modelo modelo;
    modelo.epocas = 1800;
    modelo.tamanhoLote = 6;
    modelo.taxaAprendizado = 0.025f;
    modelo.neuroniosPorCamada = {10};
    if (!modelo.executar(entradasTreino, saidasTreino, entradasTreino[0]))
    {
        Serial.println("Falha ao treinar o classificador.");
        return;
    }

    while (true)
    {
        Serial.println("Digite uma frase ASCII para classificar (ou 'sair' para terminar):");
        while (!Serial.available())
        {
            delay(10);
        }
        String fraseTeste = Serial.readStringUntil('\n');
        fraseTeste.trim();
        if (fraseTeste.equalsIgnoreCase("sair"))
        {
            break;
        }
        if (fraseTeste.length() == 0)
        {
            Serial.println("A frase nao pode ficar vazia.");
            continue;
        }

        std::vector<float> vetor;
        if (!codificarFrase(fraseTeste.c_str(), vetor))
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
        Serial.printf("\"%s\" -> %s\n", fraseTeste.c_str(), nomesClasses[classePrevista]);
    }
}

void loop()
{
}
