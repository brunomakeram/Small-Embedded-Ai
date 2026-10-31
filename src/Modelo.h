#pragma once
#include <Arduino.h>
#include "Dataset.h"
#include "Armazenamento.h"
#include "Verbose.h"
#include "Neuronios.h"
#include "Adam.h"
#include "Compilar.h"
#include "Prever.h"
#include "Perdas.h"
#include "Gradientes.h"
#include "Treinar.h"
#include "Epocas.h"
#include "Aprendizagem.h"

class Modelo {
private:
    const char *caminhoCSV = "/Bateria.csv";

public:
    // Ajustáveis pelo usuário antes de chamar executar(); os valores iniciais são os padrões
    float taxaAprendizado = 0.05f;
    int epocas = 500;
    int tamanhoLote = 32;

    void executar() {
        Armazenamento armazenamento;
        Dataset bateria;
        Verbose verbose;
        
        if (!armazenamento.lerDataset(caminhoCSV, bateria)) {
            Serial.println("Erro: não foi possível ler o dataset.");
            return;
        }

        float carga = verbose.lerNumero("Digite a carga (kg): ");
        float distancia = verbose.lerNumero("Digite a distância (km): ");

        const float cargaMin = 1.0, cargaMax = 100.0;
        const float distanciaMin = 1.0, distanciaMax = 9.0;
        
        if (!(cargaMin <= carga && carga <= cargaMax)) {
            Serial.printf("Erro: Carga (%.2f kg) fora do limite físico permitido (%.1f a %.1f kg). Dado não salvo.\n", carga, cargaMin, cargaMax);
        }
        else if (!(distanciaMin <= distancia && distancia <= distanciaMax)) {
            Serial.printf("Erro: Distância (%.2f km) fora do limite permitido (%.1f a %.1f km). Dado não salvo.\n", distancia, distanciaMin, distanciaMax);
        }
        else {
            Compilar modeloCompilado;
            modeloCompilado.configurar(taxaAprendizado);

            Aprendizagem aprendizagem;
            float perdas = aprendizagem.treinarModelo(bateria, modeloCompilado, epocas, tamanhoLote);

            Prever previsor;
            float gasto_previsto = previsor.calcular(carga, distancia, modeloCompilado.neuronios.pesos, modeloCompilado.neuronios.vies);
            
            const float gasto_min = 0.0, gasto_max = 100.0;
            if (!(gasto_min <= gasto_previsto && gasto_previsto <= gasto_max)) {
                Serial.printf("Alerta: A previsão de gasto (%.2f%%) é fisicamente impossível. Possível ponto fora da curva. Dado descartado.\n", gasto_previsto);
            } else {
                armazenamento.gravarRegistro(caminhoCSV, carga, distancia, gasto_previsto);
            }
            
            verbose.mostrarResultados(carga, distancia, perdas, modeloCompilado.neuronios.pesos, modeloCompilado.neuronios.vies, gasto_previsto);
        }
    }
};