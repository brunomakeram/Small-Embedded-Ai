#include <Seai.h>
#include <math.h>
#include <stdlib.h>
#include <vector>

// Mantém uma instância do modelo entre setup() e loop().
Modelo::Modelo modeloMachineLearning;
// Cada linha contém duas características usadas como entrada de uma amostra de treino.
const std::vector<std::vector<float>> entradasTreino = {
    {0.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 0.0f},
    {1.0f, 1.0f}
};
// Define o alvo correspondente a cada linha de entradasTreino.
const std::vector<std::vector<float>> saidasTreino = {
    {0.0f},
    {1.0f},
    {1.0f},
    {0.0f}
};
bool lerCaracteristica(const char *prompt, float &valor)
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
        if (fim != linha.c_str() && *fim == '\0' && isfinite(numero) &&
            numero >= 0.0f && numero <= 1.0f)
        {
            valor = numero;
            return true;
        }
        Serial.println("Entrada invalida. Use um numero entre 0 e 1.");
    }
}

// Inicializa a Serial, treina o modelo e aguarda uma amostra de entrada.
void setup() {
    Serial.begin(115200);
#if !SEAI_ESP32
    randomSeed(analogRead(A0));
#endif
    delay(2000);
    float entrada1 = 0.0f;
    float entrada2 = 0.0f;
    Serial.println("Digite duas caracteristicas entre 0 e 1.");
    if (!lerCaracteristica("Caracteristica 1 (0 ou 1):", entrada1) ||
        !lerCaracteristica("Caracteristica 2 (0 ou 1):", entrada2))
    {
        Serial.println("Falha ao ler as entradas.");
        return;
    }
    if (!modeloMachineLearning.executar(
            entradasTreino, saidasTreino, {entrada1, entrada2}))
    {
        Serial.println("Falha ao treinar o modelo.");
        return;
    }
    std::vector<float> previsao = modeloMachineLearning.prever({entrada1, entrada2});
    if (previsao.empty())
    {
        Serial.println("Falha ao calcular a previsao.");
        return;
    }
    Serial.printf("Entrada [%.2f, %.2f] -> previsao %.4f\n",
                  entrada1, entrada2, previsao[0]);
}

// Mantém o sketch ativo e informa que a execução inicial já foi concluída.
void loop() {
    Serial.println("\n================================");
    Serial.println("Execucao finalizada.");
    Serial.println("================================\n");
    delay(5000);
}