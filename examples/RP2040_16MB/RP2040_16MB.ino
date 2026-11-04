#define SALVAR_PESO SALVAR_PESO_LIGADO
#include <Seai.h>
#include <math.h>
#include <stdlib.h>
#include <vector>

Modelo::Modelo modelo;

const std::vector<std::vector<float>> entradas = {
    {0.0f},
    {1.0f},
    {2.0f},
    {3.0f}
};

const std::vector<std::vector<float>> saidas = {
    {0.0f},
    {2.0f},
    {4.0f},
    {6.0f}
};

bool lerEntrada(float &valor)
{
    while (true)
    {
        Serial.println("Digite um valor numerico para prever (ex.: 3.0):");
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
    delay(2000);

    modelo.neuroniosPorCamada = {2};
    modelo.epocas = 100;
    modelo.tamanhoLote = 4;

    Serial.println("Teste Seai - RP2040 com flash de 16 MB");
    Serial.println("EEPROM emulada usada pela Seai: 4096 bytes");
    Serial.println("Ao reiniciar, os pesos salvos compativeis devem ser carregados.");
    randomSeed(analogRead(A0));

    if (!modelo.executar(entradas, saidas, entradas[0]))
    {
        Serial.println("FALHA: treinamento nao executado.");
        return;
    }

    float entradaTeste = 0.0f;
    if (!lerEntrada(entradaTeste))
    {
        Serial.println("FALHA: entrada de previsao invalida.");
        return;
    }
    std::vector<float> previsao = modelo.prever({entradaTeste});
    Serial.print("Perda final: ");
    Serial.println(modelo.perdaFinal, 6);
    if (!previsao.empty())
    {
        Serial.printf("Previsao para entrada %.2f (alvo esperado ~%.2f): ",
                      entradaTeste, entradaTeste * 2.0f);
        Serial.println(previsao[0], 4);
    }
    Serial.println("Teste concluido. Reinicie a placa para testar novamente a leitura dos pesos.");
}

void loop()
{
}
