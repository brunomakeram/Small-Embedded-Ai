#include <Seai.h>
#include <math.h>
#include <stdlib.h>
#include <vector>

// Exemplo sintetico: substitua estes valores por ciclos medidos no seu equipamento.
const std::vector<std::vector<float>> dadosFisicos = {
    {10.0f,  5.0f,  3.75f},
    {10.0f, 20.0f,  4.50f},
    {10.0f, 50.0f,  6.00f},
    {30.0f,  5.0f,  6.75f},
    {30.0f, 20.0f,  7.50f},
    {30.0f, 50.0f,  9.00f},
    {60.0f,  5.0f, 11.25f},
    {60.0f, 20.0f, 12.00f},
    {60.0f, 50.0f, 13.50f},
    {90.0f,  5.0f, 15.75f},
    {90.0f, 20.0f, 16.50f},
    {90.0f, 50.0f, 18.00f}
};

const float CARGA_MAXIMA_KG = 100.0f;
const float DISTANCIA_REFERENCIA_KM = 100.0f;

bool lerNumero(const char *prompt, float &valor)
{
    while (true)
    {
        Serial.println(prompt);
        while (!Serial.available())
        {
            delay(10);
        }
        String linha = Serial.readStringUntil('\n');
        linha.trim();
        char *fim = nullptr;
        float numero = strtof(linha.c_str(), &fim);
        if (fim != linha.c_str() && *fim == '\0' && isfinite(numero))
        {
            valor = numero;
            return true;
        }
        Serial.println("Entrada invalida. Digite um numero usando ponto decimal.");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Previsao de consumo de bateria ===");

    std::vector<std::vector<float>> entradasTreino;
    std::vector<std::vector<float>> saidasTreino;
    for (std::size_t indice = 0; indice < dadosFisicos.size(); indice++)
    {
        entradasTreino.push_back({
            dadosFisicos[indice][0] / CARGA_MAXIMA_KG,
            dadosFisicos[indice][1] / DISTANCIA_REFERENCIA_KM
        });
        saidasTreino.push_back({dadosFisicos[indice][2] / 100.0f});
    }

    Modelo::Modelo modelo;
    modelo.neuroniosPorCamada.clear();
    modelo.epocas = 1500;
    modelo.tamanhoLote = 4;
    modelo.taxaAprendizado = 0.03f;

    if (!modelo.executar(entradasTreino, saidasTreino, entradasTreino[0]))
    {
        Serial.println("Falha ao treinar o modelo.");
        return;
    }

    float cargaTesteKg = 0.0f;
    float distanciaTesteKm = 0.0f;
    if (!lerNumero("Digite a carga em kg (ex.: 45.0):", cargaTesteKg) ||
        !lerNumero("Digite a distancia em km (ex.: 30.0):", distanciaTesteKm))
    {
        Serial.println("Falha ao ler os dados da viagem.");
        return;
    }
    std::vector<float> entradaTeste = {
        cargaTesteKg / CARGA_MAXIMA_KG,
        distanciaTesteKm / DISTANCIA_REFERENCIA_KM
    };
    std::vector<float> previsao = modelo.prever(entradaTeste);
    if (previsao.empty())
    {
        Serial.println("Falha ao prever o consumo.");
        return;
    }

    float percentualBateria = previsao[0] * 100.0f;
    if (percentualBateria < 0.0f)
    {
        percentualBateria = 0.0f;
    }
    if (percentualBateria > 100.0f)
    {
        percentualBateria = 100.0f;
    }

    Serial.printf("Carga: %.1f kg\n", cargaTesteKg);
    Serial.printf("Distancia: %.1f km\n", distanciaTesteKm);
    Serial.printf("Bateria prevista: %.2f%%\n", percentualBateria);
}

void loop()
{
}
