#pragma once

#include "../Placas/Placas.h"

#if SEAI_ESP32
#include <Arduino.h>
#include <LittleFS.h>
#include <math.h>
#include <stdlib.h>
#include <vector>

// Namespace Seai organiza as operações opcionais de leitura e gravação de dados no LittleFS....
namespace Seai
{
    // Agrupa a leitura e a gravação de matrizes numéricas CSV no LittleFS....
    class Armazenamento
    {
        // Os métodos públicos expõem as operações de persistência do CSV....
    public:
        // Monta o LittleFS sem criar arquivos de dados e valida o caminho CSV informado....
        // caminho é o caminho a validar; as operações de leitura e gravação recebem o próprio caminho....
        // Retorna true quando o LittleFS é montado e false quando o caminho ou a montagem falha....
        bool montar(const char *caminho)
        {
            if (caminho == nullptr || caminho[0] == '\0')
            {
                Serial.println("Erro: o caminho do arquivo CSV é inválido.");
                return false;
            }
            if (!LittleFS.begin(true))
            {
                Serial.println("Erro: falha ao montar o LittleFS.");
                return false;
            }
            return true;
        }

        // Lê uma matriz CSV e separa suas primeiras N colunas como entradas....
        // caminho identifica o CSV; quantidadeEntradas define o limite entre entradas e saídas....
        // entradas e saidas recebem as matrizes correspondentes somente após a validação completa do arquivo....
        // Retorna false se o cabeçalho, as dimensões ou qualquer valor do CSV forem inválidos....
        bool lerDados(
            const char *caminho,
            std::size_t quantidadeEntradas,
            std::vector<std::vector<float>> &entradas,
            std::vector<std::vector<float>> &saidas)
        {
            if (caminho == nullptr || quantidadeEntradas == 0)
            {
                Serial.println("Erro: caminho e quantidade de entradas devem ser válidos.");
                return false;
            }
            File arquivo = LittleFS.open(caminho, FILE_READ);
            if (!arquivo)
            {
                Serial.println("Erro: falha ao abrir o arquivo CSV para leitura.");
                return false;
            }

            std::vector<std::vector<float>> entradasLidas;
            std::vector<std::vector<float>> saidasLidas;
            std::vector<String> cabecalho;
            bool cabecalhoLido = false;
            while (arquivo.available())
            {
                String linha = arquivo.readStringUntil('\n');
                linha.trim();
                if (linha.length() == 0)
                {
                    continue;
                }

                std::vector<String> campos;
                if (!dividirLinha(linha, campos))
                {
                    arquivo.close();
                    Serial.println("Erro: linha CSV contém aspas inválidas.");
                    return false;
                }
                if (!cabecalhoLido)
                {
                    cabecalho = campos;
                    if (cabecalho.size() <= quantidadeEntradas)
                    {
                        arquivo.close();
                        Serial.println("Erro: o cabeçalho deve conter entradas e pelo menos uma saída.");
                        return false;
                    }
                    for (std::size_t indiceCampo = 0; indiceCampo < cabecalho.size(); indiceCampo++)
                    {
                        cabecalho[indiceCampo].trim();
                        if (cabecalho[indiceCampo].length() == 0)
                        {
                            arquivo.close();
                            Serial.println("Erro: o cabeçalho CSV não pode conter nomes vazios.");
                            return false;
                        }
                    }
                    cabecalhoLido = true;
                    continue;
                }
                if (campos.size() != cabecalho.size())
                {
                    arquivo.close();
                    Serial.println("Erro: a quantidade de valores não corresponde ao cabeçalho CSV.");
                    return false;
                }

                std::vector<float> entradasLinha;
                std::vector<float> saidasLinha;
                for (std::size_t indiceCampo = 0; indiceCampo < campos.size(); indiceCampo++)
                {
                    float valor;
                    if (!converterNumero(campos[indiceCampo], valor))
                    {
                        arquivo.close();
                        Serial.println("Erro: o CSV contém um valor numérico inválido.");
                        return false;
                    }
                    if (indiceCampo < quantidadeEntradas)
                    {
                        entradasLinha.push_back(valor);
                    }
                    else
                    {
                        saidasLinha.push_back(valor);
                    }
                }
                entradasLidas.push_back(entradasLinha);
                saidasLidas.push_back(saidasLinha);
            }
            arquivo.close();
            if (!cabecalhoLido || entradasLidas.empty())
            {
                Serial.println("Erro: o CSV precisa conter cabeçalho e pelo menos uma amostra.");
                return false;
            }

            entradas = entradasLidas;
            saidas = saidasLidas;
            return true;
        }

        // Substitui o CSV por matrizes de treino com nomes separados para entradas e saídas....
        // caminho identifica o arquivo; nomesEntradas e nomesSaidas definem as colunas gravadas no cabeçalho....
        // entradas e saidas contêm amostras alinhadas, com larguras compatíveis com seus nomes de coluna....
        // Retorna false quando os dados são inconsistentes ou o arquivo não pode ser gravado....
        bool gravarDados(
            const char *caminho,
            const std::vector<String> &nomesEntradas,
            const std::vector<String> &nomesSaidas,
            const std::vector<std::vector<float>> &entradas,
            const std::vector<std::vector<float>> &saidas)
        {
            if (caminho == nullptr || nomesEntradas.empty() || nomesSaidas.empty() ||
                entradas.empty() || entradas.size() != saidas.size())
            {
                Serial.println("Erro: cabeçalhos e matrizes CSV devem conter entradas e saídas correspondentes.");
                return false;
            }
            for (std::size_t indiceColuna = 0; indiceColuna < nomesEntradas.size(); indiceColuna++)
            {
                if (nomesEntradas[indiceColuna].length() == 0 ||
                    nomesEntradas[indiceColuna].indexOf('\n') >= 0 ||
                    nomesEntradas[indiceColuna].indexOf('\r') >= 0)
                {
                    Serial.println("Erro: os nomes das colunas de entrada não podem ser vazios.");
                    return false;
                }
            }
            for (std::size_t indiceColuna = 0; indiceColuna < nomesSaidas.size(); indiceColuna++)
            {
                if (nomesSaidas[indiceColuna].length() == 0 ||
                    nomesSaidas[indiceColuna].indexOf('\n') >= 0 ||
                    nomesSaidas[indiceColuna].indexOf('\r') >= 0)
                {
                    Serial.println("Erro: os nomes das colunas de saída não podem ser vazios.");
                    return false;
                }
            }
            for (std::size_t indiceAmostra = 0; indiceAmostra < entradas.size(); indiceAmostra++)
            {
                if (entradas[indiceAmostra].size() != nomesEntradas.size() ||
                    saidas[indiceAmostra].size() != nomesSaidas.size())
                {
                    Serial.println("Erro: a dimensão de uma amostra não corresponde aos nomes das colunas.");
                    return false;
                }
                for (std::size_t indiceEntrada = 0; indiceEntrada < entradas[indiceAmostra].size(); indiceEntrada++)
                {
                    if (!isfinite(entradas[indiceAmostra][indiceEntrada]))
                    {
                        Serial.println("Erro: as entradas devem conter apenas valores finitos.");
                        return false;
                    }
                }
                for (std::size_t indiceSaida = 0; indiceSaida < saidas[indiceAmostra].size(); indiceSaida++)
                {
                    if (!isfinite(saidas[indiceAmostra][indiceSaida]))
                    {
                        Serial.println("Erro: as saídas devem conter apenas valores finitos.");
                        return false;
                    }
                }
            }

            File arquivo = LittleFS.open(caminho, FILE_WRITE);
            if (!arquivo)
            {
                Serial.println("Erro: falha ao abrir o arquivo CSV para gravação.");
                return false;
            }
            for (std::size_t indiceColuna = 0; indiceColuna < nomesEntradas.size(); indiceColuna++)
            {
                if (indiceColuna > 0)
                {
                    arquivo.print(",");
                }
                escreverCampoCsv(arquivo, nomesEntradas[indiceColuna]);
            }
            for (std::size_t indiceColuna = 0; indiceColuna < nomesSaidas.size(); indiceColuna++)
            {
                arquivo.print(",");
                escreverCampoCsv(arquivo, nomesSaidas[indiceColuna]);
            }
            arquivo.print("\n");

            for (std::size_t indiceAmostra = 0; indiceAmostra < entradas.size(); indiceAmostra++)
            {
                for (std::size_t indiceEntrada = 0; indiceEntrada < entradas[indiceAmostra].size(); indiceEntrada++)
                {
                    if (indiceEntrada > 0)
                    {
                        arquivo.print(",");
                    }
                    arquivo.print(formatarNumero(entradas[indiceAmostra][indiceEntrada]));
                }
                for (std::size_t indiceSaida = 0; indiceSaida < saidas[indiceAmostra].size(); indiceSaida++)
                {
                    arquivo.print(",");
                    arquivo.print(formatarNumero(saidas[indiceAmostra][indiceSaida]));
                }
                arquivo.print("\n");
            }
            arquivo.close();
            return true;
        }

        // Acrescenta uma amostra à ordem de colunas já existente no arquivo CSV....
        // caminho identifica o CSV; entradas e saidas fornecem os valores da nova amostra....
        // Retorna false quando a amostra não corresponde ao cabeçalho ou não pode ser gravada....
        bool gravarRegistro(
            const char *caminho,
            const std::vector<float> &entradas,
            const std::vector<float> &saidas)
        {
            if (caminho == nullptr || entradas.empty() || saidas.empty())
            {
                Serial.println("Erro: a amostra deve conter entradas e saídas.");
                return false;
            }
            for (std::size_t indiceValor = 0; indiceValor < entradas.size(); indiceValor++)
            {
                if (!isfinite(entradas[indiceValor]))
                {
                    Serial.println("Erro: as entradas devem conter apenas valores finitos.");
                    return false;
                }
            }
            for (std::size_t indiceValor = 0; indiceValor < saidas.size(); indiceValor++)
            {
                if (!isfinite(saidas[indiceValor]))
                {
                    Serial.println("Erro: as saídas devem conter apenas valores finitos.");
                    return false;
                }
            }

            File arquivoLeitura = LittleFS.open(caminho, FILE_READ);
            if (!arquivoLeitura)
            {
                Serial.println("Erro: falha ao abrir o arquivo CSV para validar o cabeçalho.");
                return false;
            }
            String linhaCabecalho = arquivoLeitura.readStringUntil('\n');
            arquivoLeitura.close();
            std::vector<String> camposCabecalho;
            if (!dividirLinha(linhaCabecalho, camposCabecalho) ||
                camposCabecalho.size() != entradas.size() + saidas.size())
            {
                Serial.println("Erro: as dimensões da amostra não correspondem ao cabeçalho CSV.");
                return false;
            }

            File arquivo = LittleFS.open(caminho, FILE_APPEND);
            if (!arquivo)
            {
                Serial.println("Erro: falha ao abrir o arquivo CSV para acrescentar uma amostra.");
                return false;
            }
            for (std::size_t indiceEntrada = 0; indiceEntrada < entradas.size(); indiceEntrada++)
            {
                if (indiceEntrada > 0)
                {
                    arquivo.print(",");
                }
                arquivo.print(formatarNumero(entradas[indiceEntrada]));
            }
            for (std::size_t indiceSaida = 0; indiceSaida < saidas.size(); indiceSaida++)
            {
                arquivo.print(",");
                arquivo.print(formatarNumero(saidas[indiceSaida]));
            }
            arquivo.print("\n");
            arquivo.close();
            return true;
        }

        // Os métodos privados validam e formatam campos usados pelas operações públicas....
    private:

        // Divide uma linha CSV respeitando campos entre aspas e aspas escapadas....
        bool dividirLinha(const String &linha, std::vector<String> &campos) const
        {
            campos.clear();
            String campo;
            bool dentroAspas = false;
            for (std::size_t indiceCaractere = 0; indiceCaractere < linha.length(); indiceCaractere++)
            {
                char caractere = linha[indiceCaractere];
                if (caractere == '"')
                {
                    if (dentroAspas && indiceCaractere + 1 < linha.length() && linha[indiceCaractere + 1] == '"')
                    {
                        campo += '"';
                        indiceCaractere++;
                    }
                    else
                    {
                        dentroAspas = !dentroAspas;
                    }
                }
                else if (caractere == ',' && !dentroAspas)
                {
                    campo.trim();
                    campos.push_back(campo);
                    campo = "";
                }
                else
                {
                    campo += caractere;
                }
            }
            if (dentroAspas)
            {
                return false;
            }
            campo.trim();
            campos.push_back(campo);
            return true;
        }

        // Converte um campo CSV completo em float, recusando texto residual e valores não finitos....
        bool converterNumero(String campo, float &valor) const
        {
            campo.trim();
            if (campo.length() == 0)
            {
                return false;
            }
            char *fim;
            valor = strtof(campo.c_str(), &fim);
            return fim != campo.c_str() && *fim == '\0' && isfinite(valor);
        }

        // Formata float com precisão suficiente para reconstituir seu valor....
        String formatarNumero(float valor) const
        {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%.9g", (double)valor);
            String texto(buffer);
            if (texto.indexOf('.') < 0 && texto.indexOf('e') < 0)
            {
                texto += ".0";
            }
            return texto;
        }

        // Grava um nome de coluna escapando aspas e separadores conforme o formato CSV....
        void escreverCampoCsv(File &arquivo, const String &campo) const
        {
            bool precisaAspas = campo.indexOf(',') >= 0 || campo.indexOf('"') >= 0 ||
                                campo.indexOf('\n') >= 0 || campo.indexOf('\r') >= 0;
            if (!precisaAspas)
            {
                arquivo.print(campo);
                return;
            }
            arquivo.print("\"");
            for (std::size_t indiceCaractere = 0; indiceCaractere < campo.length(); indiceCaractere++)
            {
                if (campo[indiceCaractere] == '"')
                {
                    arquivo.print("\"\"");
                }
                else
                {
                    arquivo.write((uint8_t)campo[indiceCaractere]);
                }
            }
            arquivo.print("\"");
        }
    };
}
#endif
