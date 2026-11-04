# Small Embedded AI (Seai) — v3.0.0

Biblioteca C++ para treinar e executar modelos de Machine Learning diretamente
em placas ESP32 e Raspberry Pi Pico (RP2040). A Seai oferece redes neurais
multicamada, conversão de dados, persistência de pesos e captura de áudio I2S.
Esta documentação acompanha a versão 3.0.0 da biblioteca; somente ESP32 e
Raspberry Pi Pico (RP2040) são arquiteturas suportadas.

**Site da biblioteca:** https://small-embedded-ai.netlify.app/

## Requisitos

- Arduino IDE e a biblioteca Seai instalada.
- Placas suportadas pela biblioteca: ESP32 e Raspberry Pi Pico (RP2040).
  Placas AVR, incluindo Arduino Uno, não são suportadas.
- O gravador de áudio requer ESP32-S3, Arduino-ESP32 core 3.x e microfone
  INMP441 compatível.
- O exemplo de UF2 do Pico requer o core Arduino-Pico e uma placa RP2040
  compatível com o perfil selecionado.

## O que a Seai pode fazer

Os exemplos demonstram classificação de palavras/textos, classificação de
sentimentos, regressão linear para estimar consumo de bateria e análise de
anomalias pela corrente do motor. São exemplos compactos para execução
embarcada; a qualidade depende dos dados coletados e da representatividade das
amostras. Os classificadores de texto usam um vocabulário explícito, não um
modelo de linguagem geral.

## Exemplos e entrada pelo Monitor Serial

Abra cada pasta como sketch independente na Arduino IDE. As referências abaixo
apontam para imagens da pasta `imagens/` do GitHub; o código executável fica nos
links `.ino`.

Os conjuntos de treino sintéticos continuam no código para demonstrar o
treinamento. Os dados usados para a previsão/classificação de cada execução
são informados pelo Monitor Serial; configure a velocidade indicada para cada
exemplo e selecione final de linha `Nova linha` ou `NL e CR`.

### Teste de persistência no Raspberry Pi Pico (RP2040)

![Exemplo RP2040](imagens/RP2040.jpg)

[ Abrir o sketch RP2040_16MB.ino ](examples/RP2040_16MB/RP2040_16MB.ino)

O exemplo demonstra persistência de pesos no Raspberry Pi Pico. Digite um
número no Monitor Serial para obter uma previsão (por exemplo, `3.0`); o treino
demonstra a relação `y = 2x`. Use o core Arduino-Pico e um perfil de placa
compatível com a flash selecionada. A EEPROM emulada reserva 4096 bytes; os
16 MB de flash não são usados como espaço de EEPROM.
