#pragma once

#include <Placas/Placas.h>

//   BIBLIOTECA S-E-AI - FRAMEWORK DE MACHINE LEARNING EMBARCADO PARA MICROCONTROLADORES                //
// ==================================================================================================== //
/**
 * @file Seai.h
 * @author Bruno Maker
 * @brief Header principal da biblioteca S-E-AI para Machine Learning embarcado.
 * @details Unifica os módulos de treinamento e inferência de redes neurais,
 *          conversão de dados, persistência de pesos e captura de áudio I2S.
 *          O suporte de arquitetura é limitado a ESP32 e Raspberry Pi Pico
 *          (RP2040); placas AVR não são suportadas.
 *
 * Arquitetura Modular:
 * - Otimizadores, Camadas, Neurônios, Perdas, Gradientes, Compilação, Inferência, Armazenamento e Captura I2S.
 */

// MÓDULOS DE OTIMIZAÇÃO E MATEMÁTICA //
// Otimizador Adam: Algoritmo de descida de gradiente estocástico para ajuste dinâmico de pesos da rede neural.
#include <Adam/Adam.h>

// Gradientes: Gerenciamento e propagação do erro (backpropagation) para atualização dos gradientes.
#include <Gradientes/Gradientes.h>

// Perdas (Loss): Funções de custo para medir o erro matemático entre a predição da rede e o valor real.
#include <Perdas/Perdas.h>

// MÓDULOS DE ESTRUTURA DA REDE NEURAL //
// Neurônios: Unidades de processamento individuais com ativações e pesos sinápticos.
#include <Neuronios/Neuronios.h>

// Camadas (Layers): Agrupamento de neurônios formando as camadas ocultas, de entrada e de saída.
#include <Camadas/Camadas.h>

// Modelo: Estrutura principal que interliga as camadas, gerencia o fluxo de dados e executa a IA.
#include <Modelo/Modelo.h>

// MÓDULOS DE DADOS E APRENDIZAGEM
// Aprendizagem: Rotinas centrais de aprendizado e adaptação do modelo durante o ciclo de treino.
#include <Aprendizagem/Aprendizagem.h>

// Épocas (Epochs): Controle dos ciclos completos de treinamento sobre todo o conjunto de dados.
#include <Epocas/Epocas.h>

// Treinar (Train): Execução do loop de treinamento on-device diretamente no microcontrolador.
#include <Treinar/Treinar.h>

// MÓDULOS DE CONVERSÃO, COMPILAÇÃO E EXECUÇÃO //
// Compilar: agrupa os neurônios e o otimizador do treino de baixo nível.
#include <Compilar/Compilar.h>

// Conversor: Ferramentas de tradução e mapeamento de tipos de dados numéricos para otimização de RAM.
#include <Conversor/Conversor.h>

// Prever (Predict): Realização de inferências em tempo de execução com base no modelo treinado.
#include <Prever/Prever.h>

// MÓDULOS DE SUPORTE, HARDWARE E INFRAESTRUTURA //
// Armazenamento: leitura e gravação dos dados de treino em CSV na memória flash (LittleFS).
#if SEAI_TEM_ARMAZENAMENTO
#include <Armazenamento/Armazenamento.h>
#endif

// Gravador: Driver de captura de áudio digital via barramento I2S (ex: microfone INMP441 no ESP32).
#if SEAI_TEM_GRAVADOR
#include <Gravador/Gravador.h>
#endif

// Verbose: Sistema de logs detalhados via porta Serial para monitorar o progresso do treino e status.
#include <Verbose/Verbose.h>