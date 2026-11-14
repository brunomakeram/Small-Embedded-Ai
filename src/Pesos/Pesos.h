#pragma once

#include <Arduino.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "../Camadas/Camadas.h"
#include "../Neuronios/Neuronios.h"
#include "../Placas/Placas.h"

#if SEAI_ESP32
#include <LittleFS.h>
#else
#include <EEPROM.h>
#endif

// Namespace Seai disponibiliza persistência e transferência serial dos pesos do modelo....
// O CSV versão 1 contém cabeçalho, dimensões de cada camada e uma linha de pesos e viés por neurônio....
namespace Seai
{
    // Exporta, importa e persiste os pesos sem alterar o formato CSV SEAI_PESOS....
    class Pesos
    {
    public:
        // Escreve no destino o CSV SEAI_PESOS versão 1 com dimensões, pesos e vieses das camadas....
        // destino recebe as linhas CSV; camadas fornece os parâmetros e a arquitetura do modelo....
        // Retorna false se não houver camadas, valores não finitos ou falha ao escrever....
        static bool exportarCsv(Print &destino, const Camadas &camadas)
        {
            if (camadas.tamanho() == 0 ||
                destino.println(
                    String("SEAI_PESOS,") + (unsigned int)VERSAO + "," +
                    (unsigned int)camadas.tamanho()) == 0)
            {
                return false;
            }
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                const Neuronios &camada = camadas[indiceCamada];
                if (destino.println(String(camada.numeroEntradas) + "," + camada.numeroNeuronios) == 0)
                {
                    return false;
                }
                for (int indiceNeuronio = 0; indiceNeuronio < camada.numeroNeuronios; indiceNeuronio++)
                {
                    for (int indiceEntrada = 0; indiceEntrada < camada.numeroEntradas; indiceEntrada++)
                    {
                        if (!isfinite(camada.pesosCamada[indiceNeuronio][indiceEntrada]) ||
                            destino.print(camada.pesosCamada[indiceNeuronio][indiceEntrada], 9) == 0 ||
                            destino.print(',') == 0)
                        {
                            return false;
                        }
                    }
                    if (!isfinite(camada.viesCamada[indiceNeuronio]) ||
                        destino.println(camada.viesCamada[indiceNeuronio], 9) == 0)
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        // Lê e valida um CSV SEAI_PESOS versão 1 a partir de um fluxo serial....
        // origem deve estar posicionada no cabeçalho; camadas só é substituída após todas as linhas serem válidas....
        // A importação confere versão, dimensões encadeadas e valores numéricos finitos....
        static bool importarCsv(Stream &origem, Camadas &camadas)
        {
            String linha;
            uint32_t cabecalho[2];
            if (!lerLinha(origem, linha) || !linha.startsWith("SEAI_PESOS,") ||
                !lerInteiros(linha.substring(11), cabecalho, 2) ||
                cabecalho[0] != VERSAO || cabecalho[1] == 0 || cabecalho[1] > MAXIMO_CAMADAS)
            {
                return false;
            }

            Camadas camadasLidas;
            int quantidadeSaidasAnterior = 0;
            for (uint32_t indiceCamada = 0; indiceCamada < cabecalho[1]; indiceCamada++)
            {
                uint32_t dimensoes[2];
                if (!lerLinha(origem, linha) || !lerInteiros(linha, dimensoes, 2) ||
                    dimensoes[0] == 0 || dimensoes[0] > UINT16_MAX ||
                    dimensoes[1] == 0 || dimensoes[1] > Neuronios::NUMERO_NEURONIOS_MAXIMO ||
                    (indiceCamada > 0 && dimensoes[0] != (uint32_t)quantidadeSaidasAnterior))
                {
                    return false;
                }
                camadasLidas.adicionar((int)dimensoes[0], (int)dimensoes[1]);
                quantidadeSaidasAnterior = (int)dimensoes[1];

                std::vector<float> valores;
                for (int indiceNeuronio = 0; indiceNeuronio < (int)dimensoes[1]; indiceNeuronio++)
                {
                    if (!lerLinha(origem, linha) ||
                        !lerNumeros(linha, valores, dimensoes[0] + 1))
                    {
                        return false;
                    }
                    for (uint32_t indiceEntrada = 0; indiceEntrada < dimensoes[0]; indiceEntrada++)
                    {
                        camadasLidas[indiceCamada].pesosCamada[indiceNeuronio][indiceEntrada] =
                            valores[indiceEntrada];
                    }
                    camadasLidas[indiceCamada].viesCamada[indiceNeuronio] = valores[dimensoes[0]];
                }
            }
            camadas = camadasLidas;
            return true;
        }

        // Grava os pesos nas memórias disponíveis para a arquitetura compilada....
        // No ESP32 grava o CSV no LittleFS; no RP2040 grava a representação binária na EEPROM....
        // A função só executa quando chamada explicitamente e retorna false se a gravação falhar....
        static bool salvar(const Camadas &camadas)
        {
            if (camadas.tamanho() == 0)
            {
                Serial.println("Erro: nao ha camadas para salvar.");
                return false;
            }
#if SEAI_ESP32
            if (!LittleFS.begin(true))
            {
                Serial.println("Erro: falha ao montar o LittleFS para salvar os pesos.");
                return false;
            }
            File arquivo = LittleFS.open(CAMINHO, FILE_WRITE);
            if (!arquivo)
            {
                Serial.println("Erro: falha ao abrir o arquivo de pesos para gravacao.");
                return false;
            }
            bool sucesso = exportarCsv(arquivo, camadas);
            arquivo.close();
            if (!sucesso)
            {
                Serial.println("Erro: falha ao gravar os pesos no LittleFS.");
            }
            return sucesso;
#else
            prepararEeprom();
            std::size_t tamanhoNecessario = calcularTamanho(camadas);
            if (tamanhoNecessario > EEPROM.length())
            {
                Serial.println("Erro: os pesos do modelo excedem o espaco EEPROM disponivel.");
                return false;
            }
            std::size_t endereco = 0;
            gravarUint32(endereco, MAGIC);
            gravarUint8(endereco, VERSAO);
            gravarUint16(endereco, (uint16_t)camadas.tamanho());
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                gravarUint16(endereco, (uint16_t)camadas[indiceCamada].numeroEntradas);
                gravarUint16(endereco, (uint16_t)camadas[indiceCamada].numeroNeuronios);
            }
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                for (int indiceNeuronio = 0; indiceNeuronio < camadas[indiceCamada].numeroNeuronios; indiceNeuronio++)
                {
                    for (int indiceEntrada = 0; indiceEntrada < camadas[indiceCamada].numeroEntradas; indiceEntrada++)
                    {
                        gravarFloat(endereco, camadas[indiceCamada].pesosCamada[indiceNeuronio][indiceEntrada]);
                    }
                    gravarFloat(endereco, camadas[indiceCamada].viesCamada[indiceNeuronio]);
                }
            }
#if SEAI_RP2040
            EEPROM.commit();
#endif
            return true;
#endif
        }

    private:
        // Caminho usado para persistir o CSV no LittleFS do ESP32....
        static constexpr const char *CAMINHO = "/seai_pesos.csv";
        // Identificador e versão que distinguem os registros armazenados na EEPROM....
        static constexpr uint32_t MAGIC = 0x53454149UL;
        static constexpr uint8_t VERSAO = 1;
        // Limita a arquitetura importada para impedir dimensões ilimitadas no cabeçalho....
        static constexpr uint32_t MAXIMO_CAMADAS = 64;
        // Tamanho em bytes dos campos fixos do registro EEPROM....
        static constexpr std::size_t CABECALHO_EEPROM = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint16_t);
        static constexpr std::size_t TAMANHO_DIMENSAO_EEPROM = 2 * sizeof(uint16_t);

        // Lê uma linha até o fim de linha, remove espaços externos e rejeita conteúdo vazio....
        static bool lerLinha(Stream &origem, String &linha)
        {
            linha = origem.readStringUntil('\n');
            linha.trim();
            return linha.length() > 0;
        }

        // Converte uma linha separada por vírgulas em inteiros sem aceitar campos faltantes ou sobras....
        // valores recebe exatamente a quantidade indicada quando todos os campos são válidos....
        static bool lerInteiros(const String &linha, uint32_t *valores, std::size_t quantidade)
        {
            const char *cursor = linha.c_str();
            for (std::size_t indice = 0; indice < quantidade; indice++)
            {
                char *fim = nullptr;
                unsigned long valor = strtoul(cursor, &fim, 10);
                if (fim == cursor || valor > UINT32_MAX ||
                    (indice + 1 < quantidade ? *fim != ',' : *fim != '\0'))
                {
                    return false;
                }
                valores[indice] = (uint32_t)valor;
                cursor = indice + 1 < quantidade ? fim + 1 : fim;
            }
            return true;
        }

        // Converte os valores CSV de uma linha em floats finitos e valida a quantidade exata de campos....
        // valores é dimensionado para quantidade e só recebe os números após cada campo ser validado....
        static bool lerNumeros(const String &linha, std::vector<float> &valores, std::size_t quantidade)
        {
            valores.resize(quantidade);
            const char *cursor = linha.c_str();
            for (std::size_t indice = 0; indice < quantidade; indice++)
            {
                char *fim = nullptr;
                float valor = strtof(cursor, &fim);
                if (fim == cursor || !isfinite(valor) ||
                    (indice + 1 < quantidade ? *fim != ',' : *fim != '\0'))
                {
                    return false;
                }
                valores[indice] = valor;
                cursor = indice + 1 < quantidade ? fim + 1 : fim;
            }
            return true;
        }

        // Calcula o espaço necessário para o cabeçalho, dimensões e parâmetros binários da EEPROM....
        static std::size_t calcularTamanho(const Camadas &camadas)
        {
            std::size_t tamanho = CABECALHO_EEPROM + camadas.tamanho() * TAMANHO_DIMENSAO_EEPROM;
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                tamanho += (std::size_t)camadas[indiceCamada].numeroNeuronios *
                    ((std::size_t)camadas[indiceCamada].numeroEntradas + 1) * sizeof(float);
            }
            return tamanho;
        }

#if !SEAI_ESP32
        // Inicializa a EEPROM simulada pelo núcleo RP2040 antes de acessar seus bytes....
        static void prepararEeprom()
        {
#if SEAI_RP2040
            EEPROM.begin(4096);
#endif
        }

        // Grava um byte na EEPROM e avança o endereço para o próximo campo....
        static void gravarUint8(std::size_t &endereco, uint8_t valor)
        {
            EEPROM.write(endereco++, valor);
        }

        // Grava um inteiro de 16 bits em ordem little-endian....
        static void gravarUint16(std::size_t &endereco, uint16_t valor)
        {
            for (std::size_t indice = 0; indice < sizeof(valor); indice++)
            {
                gravarUint8(endereco, (uint8_t)(valor >> (8 * indice)));
            }
        }

        // Grava um inteiro de 32 bits em ordem little-endian....
        static void gravarUint32(std::size_t &endereco, uint32_t valor)
        {
            for (std::size_t indice = 0; indice < sizeof(valor); indice++)
            {
                gravarUint8(endereco, (uint8_t)(valor >> (8 * indice)));
            }
        }

        // Copia a representação binária de um float para EEPROM e avança o endereço....
        static void gravarFloat(std::size_t &endereco, float valor)
        {
            uint8_t bytes[sizeof(float)];
            memcpy(bytes, &valor, sizeof(float));
            for (std::size_t indice = 0; indice < sizeof(float); indice++)
            {
                gravarUint8(endereco, bytes[indice]);
            }
        }
#endif
    };
}
