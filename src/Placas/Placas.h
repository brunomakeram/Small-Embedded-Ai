#pragma once

// Detecta as arquiteturas de placa suportadas e fornece as diferenças de plataforma da Seai....
#include <Arduino.h>
#include <stdint.h>

// Permite que projetos compatíveis definam a arquitetura antes de incluir os cabeçalhos da biblioteca....
#ifndef SEAI_ESP32
#define SEAI_ESP32 0
#endif

// Mantém desligada por padrão a seleção do núcleo RP2040....
#ifndef SEAI_RP2040
#define SEAI_RP2040 0
#endif

// Reconhece a arquitetura ESP32 informada pelo núcleo Arduino ou pelo SDK....
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
#undef SEAI_ESP32
#define SEAI_ESP32 1
#endif

// Reconhece a arquitetura RP2040 informada pelo núcleo Arduino....
#if defined(ARDUINO_ARCH_RP2040)
#undef SEAI_RP2040
#define SEAI_RP2040 1
#endif

// Impede configurações ambíguas que ativem simultaneamente duas arquiteturas....
#if (SEAI_ESP32 + SEAI_RP2040) > 1
#error "A Seai detectou mais de uma arquitetura de placa. Verifique as macros SEAI_*."
#endif

// Interrompe a compilação quando a plataforma não é suportada ou não foi identificada....
#if !SEAI_ESP32 && !SEAI_RP2040
#error "Placa nao reconhecida pela Seai. Use ESP32 ou RP2040; em placas compatíveis, defina SEAI_ESP32 ou SEAI_RP2040 antes de incluir Seai.h."
#endif

// Indica se os recursos opcionais de gravação de áudio estão disponíveis....
#ifndef SEAI_TEM_GRAVADOR
#define SEAI_TEM_GRAVADOR 0
#endif

// Habilita o gravador I2S automaticamente no ESP32 com Arduino core 3 ou superior....
#if SEAI_ESP32 && defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
#undef SEAI_TEM_GRAVADOR
#define SEAI_TEM_GRAVADOR 1
#endif

// Implementa no RP2040 a fonte aleatória usada pelas rotinas de inicialização e treino....
#if !SEAI_ESP32
inline uint32_t esp_random()
{
    // Combina duas amostras de 16 bits para retornar um valor com 32 bits....
    uint32_t parteAlta = (uint32_t)random(0x10000L);
    uint32_t parteBaixa = (uint32_t)random(0x10000L);
    return (parteAlta << 16) | parteBaixa;
}
#endif
