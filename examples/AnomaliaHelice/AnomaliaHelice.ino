#include <Seai.h>
#include <math.h>
#include <stdlib.h>
#include <vector>

// N define o limite de corrente alta em desvios-padrao acima da media.
const float NUMERO_DESVIOS_ANOMALIA = 3.0f;

// Exemplos de valores z normalizados: z / N.
// Troque por leituras rotuladas como normais ou anomalas do seu motor.
const float desviosNormalizados[] = {
    0.0f, 0.1f, 0.2f, 0.35f, 0.5f, 0.65f, 0.8f,
    1.2f, 1.4f, 1.7f, 2.0f, 2.5f, 3.0f
};
const float rotulosAnomalia[] = {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
};

float calcularMedia(const float *amostras, size_t quantidade)
{
    float soma = 0.0f;
    for (size_t indice = 0; indice < quantidade; indice++)
    {
        soma += amostras[indice];
    }
    return soma / quantidade;
}

float calcularDesvioPadrao(const float *amostras, size_t quantidade, float media)
{
    float somaQuadrados = 0.0f;
    for (size_t indice = 0; indice < quantidade; indice++)
    {
        float diferenca = amostras[indice] - media;
        somaQuadrados += diferenca * diferenca;
    }
    return sqrtf(somaQuadrados / (quantidade - 1));
}

bool lerCorrentesNormais(std::vector<float> &amostras)
{
    while (true)
    {
        Serial.println("Digite correntes normais em A, separadas por virgula (ex.: 2.00,2.04,1.98):");
        while (!Serial.available())
        {
            delay(10);
        }
        String linha = Serial.readStringUntil('\n');
        linha.trim();
        amostras.clear();
        const char *cursor = linha.c_str();
        bool valida = true;
        while (*cursor != '\0')
        {
            char *fim = nullptr;
            float valor = strtof(cursor, &fim);
            if (fim == cursor || !isfinite(valor))
            {
                valida = false;
                break;
            }
            amostras.push_back(valor);
            cursor = fim;
            while (*cursor == ' ')
            {
                cursor++;
            }
            if (*cursor == ',')
            {
                cursor++;
            }
            else if (*cursor != '\0')
            {
                valida = false;
                break;
            }
        }
        if (valida && amostras.size() >= 2 && amostras.size() <= 100)
        {
            return true;
        }
        Serial.println("Entrada invalida. Informe de 2 a 100 valores numericos separados por virgula.");
    }
}

bool lerCorrenteAtual(float &corrente)
{
    while (true)
    {
        Serial.println("Digite a corrente atual em A (ex.: 2.50):");
        while (!Serial.available())
        {
            delay(10);
        }
        String linha = Serial.readStringUntil('\n');
        linha.trim();
        char *fim = nullptr;
        float valor = strtof(linha.c_str(), &fim);
        if (fim != linha.c_str() && *fim == '\0' && isfinite(valor))
        {
            corrente = valor;
            return true;
        }
        Serial.println("Entrada invalida. Digite um numero, usando ponto decimal.");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Anomalia de corrente da helice ===");

    std::vector<std::vector<float>> entradasTreino;
    std::vector<std::vector<float>> saidasTreino;
    for (size_t indice = 0;
         indice < sizeof(desviosNormalizados) / sizeof(desviosNormalizados[0]);
         indice++)
    {
        entradasTreino.push_back({desviosNormalizados[indice]});
        saidasTreino.push_back({rotulosAnomalia[indice]});
    }

    Modelo::Modelo modelo;
    modelo.neuroniosPorCamada = {6};
    modelo.epocas = 1400;
    modelo.tamanhoLote = 4;
    modelo.taxaAprendizado = 0.025f;
    if (!modelo.executar(entradasTreino, saidasTreino, entradasTreino[0]))
    {
        Serial.println("Falha ao treinar o detector.");
        return;
    }

    // Substitua esta serie pelos valores de corrente do motor em operacao normal.
    std::vector<float> correnteNormal;
    float correnteAtual = 0.0f;
    if (!lerCorrentesNormais(correnteNormal) || !lerCorrenteAtual(correnteAtual))
    {
        Serial.println("Falha ao ler as correntes.");
        return;
    }
    const size_t quantidadeAmostras = correnteNormal.size();

    float media = calcularMedia(correnteNormal.data(), quantidadeAmostras);
    float desvioPadrao = calcularDesvioPadrao(
        correnteNormal.data(), quantidadeAmostras, media);

    if (desvioPadrao <= 0.0f)
    {
        Serial.println("Desvio padrao invalido; colete mais amostras.");
        return;
    }

    float desvioAtual = (correnteAtual - media) / desvioPadrao;
    float entradaNormalizada = desvioAtual / NUMERO_DESVIOS_ANOMALIA;
    std::vector<float> previsao = modelo.prever({entradaNormalizada});
    if (previsao.empty())
    {
        Serial.println("Falha ao avaliar a amostra.");
        return;
    }

    // A regra N-sigma e a decisao exata; o resultado SEAI e uma classificacao aprendida.
    bool anomaliaPelaRegra = desvioAtual > NUMERO_DESVIOS_ANOMALIA;
    bool anomaliaPelaSeai = previsao[0] >= 0.5f;

    Serial.printf("Media normal: %.3f A\n", media);
    Serial.printf("Desvio padrao: %.3f A\n", desvioPadrao);
    Serial.printf("Corrente atual: %.3f A\n", correnteAtual);
    Serial.printf("Desvios acima da media: %.2f\n", desvioAtual);
    Serial.printf(
        "Regra acima de %.1f desvios: %s\n",
        NUMERO_DESVIOS_ANOMALIA,
        anomaliaPelaRegra ? "ANOMALIA" : "NORMAL");
    Serial.printf(
        "Classificacao SEAI: %s (%.3f)\n",
        anomaliaPelaSeai ? "ANOMALIA" : "NORMAL",
        previsao[0]);
}

void loop()
{
}
