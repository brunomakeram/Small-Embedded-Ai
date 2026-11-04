#pragma once

#include <Arduino.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "../Camadas/Camadas.h"
#include "../Placas/Placas.h"

#if SEAI_ESP32
#include <LittleFS.h>
#else
#include <EEPROM.h>
#endif

namespace PersistenciaPesos
{
    enum ResultadoCarregamento
    {
        PESOS_CARREGADOS,
        PESOS_AUSENTES,
        PESOS_INVALIDOS
    };

    class Pesos
    {
    public:
        static bool salvar(const Camadas::Camadas &camadas)
        {
#if SEAI_ESP32
            if (!LittleFS.begin(true))
            {
                Serial.println("Erro: falha ao montar o LittleFS para salvar os pesos.");
                return false;
            }

            File arquivo = LittleFS.open(CAMINHO, FILE_WRITE);
            if (!arquivo)
            {
                Serial.println("Erro: falha ao abrir o arquivo de pesos para gravação.");
                return false;
            }

            bool sucesso = gravarCsv(arquivo, camadas);
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
                Serial.println("Erro: os pesos do modelo excedem o espaço EEPROM disponível.");
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

        static ResultadoCarregamento carregar(Camadas::Camadas &camadas)
        {
#if SEAI_ESP32
            if (!LittleFS.begin(true))
            {
                Serial.println("Erro: falha ao montar o LittleFS para carregar os pesos.");
                return PESOS_INVALIDOS;
            }
            if (!LittleFS.exists(CAMINHO))
            {
                return PESOS_AUSENTES;
            }

            File arquivo = LittleFS.open(CAMINHO, FILE_READ);
            if (!arquivo)
            {
                Serial.println("Erro: falha ao abrir o arquivo de pesos para leitura.");
                return PESOS_INVALIDOS;
            }
            if (!validarCsv(arquivo, camadas) || !arquivo.seek(0))
            {
                arquivo.close();
                return PESOS_INVALIDOS;
            }
            bool sucesso = lerCsv(arquivo, camadas);
            arquivo.close();
            return sucesso ? PESOS_CARREGADOS : PESOS_INVALIDOS;
#else
            prepararEeprom();
            std::size_t tamanhoNecessario = calcularTamanho(camadas);
            if (EEPROM.length() < sizeof(uint32_t))
            {
                Serial.println("Erro: o espaço EEPROM disponível é insuficiente.");
                return PESOS_INVALIDOS;
            }

            uint32_t magic = 0;
            std::size_t endereco = 0;
            lerUint32(endereco, magic);
            if (magic == UINT32_MAX)
            {
                return PESOS_AUSENTES;
            }
            if (tamanhoNecessario > EEPROM.length() || !validarEeprom(camadas))
            {
                return PESOS_INVALIDOS;
            }

            endereco = CABECALHO_EEPROM + camadas.tamanho() * TAMANHO_DIMENSAO_EEPROM;
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                for (int indiceNeuronio = 0; indiceNeuronio < camadas[indiceCamada].numeroNeuronios; indiceNeuronio++)
                {
                    for (int indiceEntrada = 0; indiceEntrada < camadas[indiceCamada].numeroEntradas; indiceEntrada++)
                    {
                        float valor;
                        lerFloat(endereco, valor);
                        camadas[indiceCamada].pesosCamada[indiceNeuronio][indiceEntrada] = valor;
                    }
                    float vies;
                    lerFloat(endereco, vies);
                    camadas[indiceCamada].viesCamada[indiceNeuronio] = vies;
                }
            }
            return PESOS_CARREGADOS;
#endif
        }

    private:
        static constexpr const char *CAMINHO = "/seai_pesos.csv";
        static constexpr uint32_t MAGIC = 0x53454149UL;
        static constexpr uint8_t VERSAO = 1;
        static constexpr std::size_t CABECALHO_EEPROM = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint16_t);
        static constexpr std::size_t TAMANHO_DIMENSAO_EEPROM = 2 * sizeof(uint16_t);

        static std::size_t calcularTamanho(const Camadas::Camadas &camadas)
        {
            std::size_t tamanho = CABECALHO_EEPROM + camadas.tamanho() * TAMANHO_DIMENSAO_EEPROM;
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                tamanho += (std::size_t)camadas[indiceCamada].numeroNeuronios *
                    ((std::size_t)camadas[indiceCamada].numeroEntradas + 1) * sizeof(float);
            }
            return tamanho;
        }

#if SEAI_ESP32
        static bool gravarCsv(File &arquivo, const Camadas::Camadas &camadas)
        {
            if (arquivo.println(
                    String("SEAI_PESOS,") + (unsigned int)VERSAO + "," +
                    (unsigned int)camadas.tamanho()) == 0)
            {
                return false;
            }
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                if (arquivo.println(String(camadas[indiceCamada].numeroEntradas) + "," +
                    camadas[indiceCamada].numeroNeuronios) == 0)
                {
                    return false;
                }
                for (int indiceNeuronio = 0; indiceNeuronio < camadas[indiceCamada].numeroNeuronios; indiceNeuronio++)
                {
                    for (int indiceEntrada = 0; indiceEntrada < camadas[indiceCamada].numeroEntradas; indiceEntrada++)
                    {
                        if (arquivo.print(camadas[indiceCamada].pesosCamada[indiceNeuronio][indiceEntrada], 9) == 0 ||
                            arquivo.print(',') == 0)
                        {
                            return false;
                        }
                    }
                    if (arquivo.println(camadas[indiceCamada].viesCamada[indiceNeuronio], 9) == 0)
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        static bool lerCamposInteiros(const String &linha, uint32_t *valores, std::size_t quantidade)
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

        static bool lerLinhaCsv(File &arquivo, std::vector<float> &valores, std::size_t quantidade)
        {
            if (!arquivo.available())
            {
                return false;
            }
            String linha = arquivo.readStringUntil('\n');
            linha.trim();
            const char *cursor = linha.c_str();
            valores.resize(quantidade);
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

        static bool validarCsv(File &arquivo, const Camadas::Camadas &camadas)
        {
            String linha = arquivo.readStringUntil('\n');
            linha.trim();
            const char *prefixo = "SEAI_PESOS,";
            if (!linha.startsWith(prefixo))
            {
                return false;
            }
            uint32_t cabecalho[2];
            String camposCabecalho = linha.substring(strlen(prefixo));
            if (!lerCamposInteiros(camposCabecalho, cabecalho, 2) ||
                cabecalho[0] != VERSAO || cabecalho[1] != camadas.tamanho())
            {
                return false;
            }

            std::vector<float> valores;
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                linha = arquivo.readStringUntil('\n');
                linha.trim();
                uint32_t dimensoes[2];
                if (!lerCamposInteiros(linha, dimensoes, 2) ||
                    dimensoes[0] != (uint32_t)camadas[indiceCamada].numeroEntradas ||
                    dimensoes[1] != (uint32_t)camadas[indiceCamada].numeroNeuronios)
                {
                    return false;
                }
                for (int indiceNeuronio = 0; indiceNeuronio < camadas[indiceCamada].numeroNeuronios; indiceNeuronio++)
                {
                    if (!lerLinhaCsv(arquivo, valores, camadas[indiceCamada].numeroEntradas + 1))
                    {
                        return false;
                    }
                }
            }
            return !arquivo.available();
        }

        static bool lerCsv(File &arquivo, Camadas::Camadas &camadas)
        {
            String linha = arquivo.readStringUntil('\n');
            linha.trim();
            std::vector<float> valores;
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                linha = arquivo.readStringUntil('\n');
                for (int indiceNeuronio = 0; indiceNeuronio < camadas[indiceCamada].numeroNeuronios; indiceNeuronio++)
                {
                    if (!lerLinhaCsv(arquivo, valores, camadas[indiceCamada].numeroEntradas + 1))
                    {
                        return false;
                    }
                    for (int indiceEntrada = 0; indiceEntrada < camadas[indiceCamada].numeroEntradas; indiceEntrada++)
                    {
                        camadas[indiceCamada].pesosCamada[indiceNeuronio][indiceEntrada] = valores[indiceEntrada];
                    }
                    camadas[indiceCamada].viesCamada[indiceNeuronio] =
                        valores[camadas[indiceCamada].numeroEntradas];
                }
            }
            return true;
        }
#else
        static void prepararEeprom()
        {
#if SEAI_RP2040
            EEPROM.begin(4096);
#endif
        }

        static void gravarUint8(std::size_t &endereco, uint8_t valor)
        {
            EEPROM.write(endereco++, valor);
        }

        static void gravarUint16(std::size_t &endereco, uint16_t valor)
        {
            for (std::size_t indice = 0; indice < sizeof(valor); indice++)
            {
                gravarUint8(endereco, (uint8_t)(valor >> (8 * indice)));
            }
        }

        static void gravarUint32(std::size_t &endereco, uint32_t valor)
        {
            for (std::size_t indice = 0; indice < sizeof(valor); indice++)
            {
                gravarUint8(endereco, (uint8_t)(valor >> (8 * indice)));
            }
        }

        static void gravarFloat(std::size_t &endereco, float valor)
        {
            uint8_t bytes[sizeof(float)];
            memcpy(bytes, &valor, sizeof(float));
            for (std::size_t indice = 0; indice < sizeof(float); indice++)
            {
                gravarUint8(endereco, bytes[indice]);
            }
        }

        static uint8_t lerUint8(std::size_t &endereco)
        {
            return EEPROM.read(endereco++);
        }

        static uint16_t lerUint16(std::size_t &endereco)
        {
            uint16_t valor = 0;
            for (std::size_t indice = 0; indice < sizeof(valor); indice++)
            {
                valor |= (uint16_t)lerUint8(endereco) << (8 * indice);
            }
            return valor;
        }

        static void lerUint32(std::size_t &endereco, uint32_t &valor)
        {
            valor = 0;
            for (std::size_t indice = 0; indice < sizeof(valor); indice++)
            {
                valor |= (uint32_t)lerUint8(endereco) << (8 * indice);
            }
        }

        static void lerFloat(std::size_t &endereco, float &valor)
        {
            uint8_t bytes[sizeof(float)];
            for (std::size_t indice = 0; indice < sizeof(float); indice++)
            {
                bytes[indice] = lerUint8(endereco);
            }
            memcpy(&valor, bytes, sizeof(float));
        }

        static bool validarEeprom(const Camadas::Camadas &camadas)
        {
            std::size_t endereco = 0;
            uint32_t magic;
            lerUint32(endereco, magic);
            uint8_t versao = lerUint8(endereco);
            uint16_t quantidadeCamadas = lerUint16(endereco);
            if (magic != MAGIC || versao != VERSAO || quantidadeCamadas != camadas.tamanho())
            {
                return false;
            }
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                uint16_t quantidadeEntradas = lerUint16(endereco);
                uint16_t quantidadeNeuronios = lerUint16(endereco);
                if (quantidadeEntradas != camadas[indiceCamada].numeroEntradas ||
                    quantidadeNeuronios != camadas[indiceCamada].numeroNeuronios)
                {
                    return false;
                }
            }
            for (std::size_t indiceCamada = 0; indiceCamada < camadas.tamanho(); indiceCamada++)
            {
                for (int indiceNeuronio = 0; indiceNeuronio < camadas[indiceCamada].numeroNeuronios; indiceNeuronio++)
                {
                    for (int indiceEntrada = 0; indiceEntrada <= camadas[indiceCamada].numeroEntradas; indiceEntrada++)
                    {
                        float valor;
                        lerFloat(endereco, valor);
                        if (!isfinite(valor))
                        {
                            return false;
                        }
                    }
                }
            }
            return true;
        }
#endif
    };
}
