#include <Seai.h>
#include <vector>

// Cada linha contém carga em kg, distância em km e percentual de bateria gasto numa viagem....
// Substitua os valores sintéticos por medições do mesmo equipamento e mantenha unidades consistentes....
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

// Referências usadas para normalizar entradas e manter os valores de treino aproximadamente entre zero e um....
const float CARGA_MAXIMA_KG = 100.0f;
const float DISTANCIA_REFERENCIA_KM = 100.0f;

// Normaliza as amostras, treina a regressão e estima o gasto para uma viagem de teste....
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Previsao de consumo de bateria ===");

    // Separa as colunas físicas em duas entradas normalizadas e um alvo percentual normalizado....
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

    // Esta regressão não usa camada oculta; os parâmetros controlam convergência e tamanho de lote....
    Seai::Modelo modelo;
    modelo.neuroniosPorCamada.clear();
    modelo.epocas = 1500;
    modelo.tamanhoLote = 4;
    modelo.taxaAprendizado = 0.03f;

    if (!modelo.executar(entradasTreino, saidasTreino, entradasTreino[0]))
    {
        Serial.println("Falha ao treinar o modelo.");
        return;
    }

    // Altere estes valores para prever o consumo de outra viagem....
    const float cargaTesteKg = 45.0f;
    const float distanciaTesteKm = 30.0f;
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

    // Converte a saída normalizada para percentual e limita a estimativa ao intervalo físico de 0 a 100....
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

// O exemplo não agenda novas previsões; a análise termina após setup()....
void loop()
{
}
