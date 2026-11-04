#include <Seai.h>
#include <math.h>
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
    const float correnteNormal[] = {
        2.00f, 2.04f, 1.98f, 2.03f, 2.01f,
        1.97f, 2.02f, 2.05f, 1.99f, 2.01f
    };
    const size_t quantidadeAmostras =
        sizeof(correnteNormal) / sizeof(correnteNormal[0]);
    const float correnteAtual = 2.50f;

    float media = calcularMedia(correnteNormal, quantidadeAmostras);
    float desvioPadrao = calcularDesvioPadrao(
        correnteNormal, quantidadeAmostras, media);

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
