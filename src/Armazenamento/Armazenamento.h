#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include "../Dataset/Dataset.h"

// Agrupa as operações que persistem o dataset no sistema de arquivos LittleFS.
namespace Armazenamento {
class Armazenamento {
public:
    // Monta o LittleFS e cria o arquivo com os dados iniciais quando ele ainda não existe.
    // caminho identifica o arquivo; o retorno informa se a montagem e a criação foram concluídas.
    bool montar(const char *caminho) {
        // O argumento true permite formatar LittleFS caso a montagem inicial falhe.
        if (!LittleFS.begin(true)) {
            Serial.println("Erro: falha ao montar o LittleFS.");
            return false;
        }
        // O dataset padrão é gravado apenas na primeira execução, preservando registros existentes.
        if (!LittleFS.exists(caminho)) {
            File arquivo = LittleFS.open(caminho, FILE_WRITE);
            if (!arquivo) {
                Serial.println("Erro: falha ao criar o arquivo do dataset.");
                return false;
            }
            // Inicializa o CSV com as amostras incluídas no módulo Dataset.
            arquivo.print(DATASET_PADRAO);
            arquivo.close();
        }
        return true;
    }

    // Lê o CSV e acrescenta os valores das colunas reconhecidas aos vetores do dataset recebido.
    // caminho aponta para o arquivo e dados recebe uma amostra por posição em cada vetor.
    bool lerDataset(const char *caminho, Dataset::Dataset &dados) {
        File arquivo = LittleFS.open(caminho, FILE_READ);
        if (!arquivo) return false;

        // Os índices começam inválidos e são preenchidos ao interpretar a primeira linha do CSV.
        int colunaCarga = -1, colunaDistancia = -1, colunaGasto = -1;
        // Distingue a linha de nomes das colunas das linhas que contêm valores.
        bool cabecalho = true;

        // Processa uma linha de cada vez para não carregar o arquivo inteiro na memória do microcontrolador.
        while (arquivo.available()) {
            String linhaTabela = arquivo.readStringUntil('\n');
            linhaTabela.trim();
            if (linhaTabela.length() == 0) continue;

            // O formato esperado contém três colunas; o limite de oito também evita escrita fora do array.
            String camposLinha[8];
            int totalCampos = 0, inicio = 0;

            // Divide a linha nas vírgulas, preservando o último campo mesmo sem vírgula final.
            while (totalCampos < 8) {
                int virgula = linhaTabela.indexOf(',', inicio);
                if (virgula < 0) {
                    camposLinha[totalCampos++] = linhaTabela.substring(inicio);
                    break;
                }
                camposLinha[totalCampos++] = linhaTabela.substring(inicio, virgula);
                inicio = virgula + 1;
            }

            // Localiza as colunas pelo nome para não depender de sua ordem no arquivo.
            if (cabecalho) {
                for (int i = 0; i < totalCampos; i++) {
                    camposLinha[i].trim();
                    if (camposLinha[i] == "Carga") colunaCarga = i;
                    else if (camposLinha[i] == "Distância") colunaDistancia = i;
                    else if (camposLinha[i] == "Gasto") colunaGasto = i;
                }
                // Sem os três campos esperados não há como mapear corretamente as amostras.
                if (colunaCarga < 0 || colunaDistancia < 0 || colunaGasto < 0) {
                    arquivo.close();
                    return false;
                }
                cabecalho = false;
                continue;
            }

            // Converte os valores das colunas encontradas e mantém os vetores alinhados por amostra.
            dados.carga.push_back(camposLinha[colunaCarga].toFloat());
            dados.distancia.push_back(camposLinha[colunaDistancia].toFloat());
            dados.gasto.push_back(camposLinha[colunaGasto].toFloat());
        }

        arquivo.close();
        return !dados.gasto.empty();
    }

    // Formata um valor com até nove dígitos significativos, preservando a precisão de float.
    String formatarNumero(float valor) {
        // O buffer comporta o maior texto numérico resultante de %.9g.
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.9g", (double)valor);
        String texto = String(buffer);
        // Acrescenta uma parte decimal quando o texto inteiro pode ser interpretado como campo CSV inteiro.
        if (texto.indexOf('.') < 0 && texto.indexOf('e') < 0) texto += ".0";
        return texto;
    }

    // Acrescenta uma amostra ao final do CSV usando a mesma ordem de colunas do cabeçalho padrão.
    bool gravarRegistro(const char *caminho, float carga, float distancia, float gasto) {
        // FILE_APPEND preserva os registros anteriores e posiciona a escrita no fim do arquivo.
        File arquivo = LittleFS.open(caminho, FILE_APPEND);
        if (!arquivo) return false;
        
        // Grava os três valores separados por vírgula e termina o registro com uma nova linha.
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
}