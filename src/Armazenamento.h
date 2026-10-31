#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include "Dataset.h"

class Armazenamento {
public:
    bool montar(const char *caminho) {
        if (!LittleFS.begin(true)) {
            Serial.println("Erro: falha ao montar o LittleFS.");
            return false;
        }
        if (!LittleFS.exists(caminho)) {
            File arquivo = LittleFS.open(caminho, FILE_WRITE);
            if (!arquivo) {
                Serial.println("Erro: falha ao criar o arquivo do dataset.");
                return false;
            }
            arquivo.print(DATASET_PADRAO);
            arquivo.close();
        }
        return true;
    }

    bool lerDataset(const char *caminho, Dataset &dados) {
        File arquivo = LittleFS.open(caminho, FILE_READ);
        if (!arquivo) return false;

        int colunaCarga = -1, colunaDistancia = -1, colunaGasto = -1;
        bool cabecalho = true;

        while (arquivo.available()) {
            String linhaTabela = arquivo.readStringUntil('\n');
            linhaTabela.trim();
            if (linhaTabela.length() == 0) continue;

            String camposLinha[8];
            int totalCampos = 0, inicio = 0;

            while (totalCampos < 8) {
                int virgula = linhaTabela.indexOf(',', inicio);
                if (virgula < 0) {
                    camposLinha[totalCampos++] = linhaTabela.substring(inicio);
                    break;
                }
                camposLinha[totalCampos++] = linhaTabela.substring(inicio, virgula);
                inicio = virgula + 1;
            }

            if (cabecalho) {
                for (int i = 0; i < totalCampos; i++) {
                    camposLinha[i].trim();
                    if (camposLinha[i] == "Carga") colunaCarga = i;
                    else if (camposLinha[i] == "Distância") colunaDistancia = i;
                    else if (camposLinha[i] == "Gasto") colunaGasto = i;
                }
                if (colunaCarga < 0 || colunaDistancia < 0 || colunaGasto < 0) {
                    arquivo.close();
                    return false;
                }
                cabecalho = false;
                continue;
            }

            dados.carga.push_back(camposLinha[colunaCarga].toFloat());
            dados.distancia.push_back(camposLinha[colunaDistancia].toFloat());
            dados.gasto.push_back(camposLinha[colunaGasto].toFloat());
        }

        arquivo.close();
        return !dados.gasto.empty();
    }

    String formatarNumero(float valor) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.9g", (double)valor);
        String texto = String(buffer);
        if (texto.indexOf('.') < 0 && texto.indexOf('e') < 0) texto += ".0";
        return texto;
    }

    bool gravarRegistro(const char *caminho, float carga, float distancia, float gasto) {
        File arquivo = LittleFS.open(caminho, FILE_APPEND);
        if (!arquivo) return false;
        
        arquivo.print(formatarNumero(carga));
        arquivo.print(",");
        arquivo.print(formatarNumero(distancia));
        arquivo.print(",");
        arquivo.print(formatarNumero(gasto));
        arquivo.print("\n");
        
        arquivo.close();
        return true;
    }
};