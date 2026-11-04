#pragma once

#include <Arduino.h>
#include <stdint.h>

#ifndef SEAI_ESP32
#define SEAI_ESP32 0
#endif

#ifndef SEAI_RP2040
#define SEAI_RP2040 0
#endif

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
#undef SEAI_ESP32
#define SEAI_ESP32 1
#endif

#if defined(ARDUINO_ARCH_RP2040)
#undef SEAI_RP2040
#define SEAI_RP2040 1
#endif

#if (SEAI_ESP32 + SEAI_RP2040) > 1
#error "A Seai detectou mais de uma arquitetura de placa. Verifique as macros SEAI_*."
#endif

#if !SEAI_ESP32 && !SEAI_RP2040
#error "Placa nao reconhecida pela Seai. Use ESP32 ou RP2040; em placas compatíveis, defina SEAI_ESP32 ou SEAI_RP2040 antes de incluir Seai.h."
#endif

#ifndef SEAI_TEM_GRAVADOR
#define SEAI_TEM_GRAVADOR 0
#endif

#if SEAI_ESP32 && defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
#undef SEAI_TEM_GRAVADOR
#define SEAI_TEM_GRAVADOR 1
#endif

#ifndef SEAI_TEM_ARMAZENAMENTO
#define SEAI_TEM_ARMAZENAMENTO 0
#endif

#if SEAI_ESP32
#undef SEAI_TEM_ARMAZENAMENTO
#define SEAI_TEM_ARMAZENAMENTO 1
#endif

#ifndef SALVAR_PESO_LIGADO
#define SALVAR_PESO_LIGADO 1
#endif

#ifndef SALVAR_PESO_DESLIGADO
#define SALVAR_PESO_DESLIGADO 0
#endif

#ifndef SALVAR_PESO
#define SALVAR_PESO SALVAR_PESO_DESLIGADO
#endif

#if !SEAI_ESP32
inline uint32_t esp_random()
{
    uint32_t parteAlta = (uint32_t)random(0x10000L);
    uint32_t parteBaixa = (uint32_t)random(0x10000L);
    return (parteAlta << 16) | parteBaixa;
}
#endif
