<div align="center">

# Small Embedded AI (Seai) — v3.0.0

**Treinamento e inferência de redes neurais diretamente em ESP32 e Raspberry Pi Pico (RP2040).**

[![Versão](https://img.shields.io/badge/versão-3.0.0-blue.svg)](https://github.com/brunomakeram/Small-Embedded-Ai)
[![Plataformas](https://img.shields.io/badge/plataformas-ESP32%20%7C%20RP2040-orange.svg)](https://github.com/brunomakeram/Small-Embedded-Ai)
[![Linguagem](https://img.shields.io/badge/linguagem-C%2B%2B-informational.svg)](https://github.com/brunomakeram/Small-Embedded-Ai)
[![Arduino](https://img.shields.io/badge/framework-Arduino-00979D.svg?logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Documentação](https://img.shields.io/badge/documentação-site-brightgreen.svg)](https://small-embedded-ai.netlify.app/)

[**Português**](README.md) · [English](README.en.md)

</div>

Seai é uma biblioteca C++ modular para treinar e executar redes neurais leves diretamente em microcontroladores suportados. Ela oferece uma API orientada a objetos para modelos, otimizador Adam, treinamento em lotes, inferência, persistência opcional de pesos, utilitários CSV e captura de áudio I2S.

> **Escopo:** esta documentação acompanha a versão **3.0.0**. As arquiteturas suportadas são ESP32 e Raspberry Pi Pico (RP2040). Placas AVR, incluindo Arduino Uno, não são suportadas.

## Índice

- [Recursos](#recursos)
- [Hardware e requisitos](#hardware-e-requisitos)
- [Instalação](#instalação)
- [Início rápido](#início-rápido)
- [API do modelo e comportamento do treinamento](#api-do-modelo-e-comportamento-do-treinamento)
- [Exemplos](#exemplos)
- [Módulos opcionais](#módulos-opcionais)
- [Estrutura do repositório](#estrutura-do-repositório)
- [Entrada pelo Monitor Serial](#entrada-pelo-monitor-serial)
- [Limitações e recomendações](#limitações-e-recomendações)
- [Contribuições](#contribuições)
- [Licença e créditos](#licença-e-créditos)

## Recursos

- Treinamento embarcado de redes neurais feed-forward a partir de matrizes numéricas de entradas e alvos.
- Configuração do número de neurônios nas camadas ocultas, épocas, tamanho de lote e taxa de aprendizado.
- Otimização Adam e retropropagação do erro.
- Inferência com `prever(...)` após o treinamento.
- Cálculo do erro quadrático médio (MSE), disponível em `perdaFinal`.
- Persistência opcional de pesos compatíveis, quando habilitada na compilação.
- Leitura e gravação de conjuntos numéricos CSV via módulo de armazenamento em configurações compatíveis.
- Captura de áudio PCM mono por I2S com o módulo gravador para ESP32-S3.
- Logs e diagnósticos pela Serial com o módulo `Verbose`.

## Hardware e requisitos

| Item | Requisito / situação |
|---|---|
| Arquiteturas suportadas | ESP32 e RP2040 (família Raspberry Pi Pico) |
| Ambiente Arduino | Arduino IDE com a biblioteca Seai instalada |
| Placas AVR | Não suportadas, incluindo Arduino Uno |
| Gravador de voz | ESP32-S3, Arduino-ESP32 core 3.x e microfone INMP441 compatível |
| Exemplo UF2 para RP2040 | Core Arduino-Pico e placa/perfil compatível com a configuração de flash selecionada |

Antes de compilar, selecione o pacote de placas e o perfil corretos. Módulos e exemplos específicos de hardware podem ter requisitos adicionais de pinos, versão do core, flash ou comunicação serial; consulte o sketch correspondente e a documentação do gravador.

## Instalação

### Arduino IDE

Instale a Seai como biblioteca Arduino usando o repositório ou a pasta da biblioteca e reinicie a IDE, se necessário. Os metadados estão em `library.properties`:

- Nome: `Seai`
- Versão: `3.0.0`
- Arquiteturas declaradas: `esp32,rp2040`

Abra um sketch individual em `examples/` e selecione a placa e a porta antes de compilar ou carregar.

### Clonar o repositório

```bash
git clone https://github.com/brunomakeram/Small-Embedded-Ai.git
```

Instale ou coloque a biblioteca no diretório de bibliotecas Arduino de acordo com a configuração da sua IDE. Os sketches em `examples/` são independentes; abra o arquivo `.ino` na pasta do exemplo escolhido.

## Início rápido

O exemplo abaixo ilustra a API principal com duas características de entrada e uma saída. O sketch `examples/Seai/Seai.ino` também demonstra leitura pela Serial e tratamento de erros.

```cpp
#include <Seai.h>
#include <vector>

Modelo::Modelo modelo;

const std::vector<std::vector<float>> entradas = {
    {0.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 0.0f},
    {1.0f, 1.0f}
};

const std::vector<std::vector<float>> alvos = {
    {0.0f},
    {1.0f},
    {1.0f},
    {0.0f}
};

void setup() {
    Serial.begin(115200);

    // Treina com o conjunto informado e avalia a entrada indicada.
    if (!modelo.executar(entradas, alvos, {0.0f, 1.0f})) {
        Serial.println("Falha no treinamento.");
        return;
    }

    // A inferência também pode ser solicitada após o treinamento.
    const std::vector<float> previsao = modelo.prever({0.0f, 1.0f});
    if (!previsao.empty()) {
        Serial.println(previsao[0], 4);
    }
}

void loop() {}
```

`executar(...)` recebe a matriz de entradas de treino, a matriz de saídas esperadas e um vetor de entrada a ser avaliado após o treinamento. Cada linha de entrada deve corresponder a uma linha de alvo; todas as linhas devem manter dimensões consistentes e conter valores finitos.

## API do modelo e comportamento do treinamento

A classe pública principal é `Modelo::Modelo`, disponibilizada por `#include <Seai.h>`.

| Membro | Descrição |
|---|---|
| `taxaAprendizado` | Taxa positiva usada para inicializar o Adam. Padrão: `0.05f`. |
| `epocas` | Número de passagens completas pelo conjunto de treino. Padrão: `500`. |
| `tamanhoLote` | Número de amostras acumuladas por atualização dos parâmetros. Valores menores que 1 são tratados como 1 no treinamento. Padrão: `32`. |
| `neuroniosPorCamada` | Quantidade de neurônios em cada camada oculta. A largura da saída é inferida dos alvos. |
| `perdaFinal` | MSE calculado sobre o conjunto de treino após o treinamento. |
| `verbose` | Configuração de logs exposta pelo módulo `Verbose`. |
| `executar(entradas, alvos, entradaPrevisao)` | Valida dados, constrói a rede, treina, calcula a perda final, salva pesos se habilitado e avalia `entradaPrevisao`. Retorna `false` para dados ou configurações inválidos. |
| `prever(entrada)` | Executa inferência usando o modelo atual. Retorna vetor vazio se o modelo não foi treinado ou se a entrada for inválida. |

### Notas de implementação

- A arquitetura das camadas ocultas é definida por `neuroniosPorCamada`; a largura da camada de saída vem dos vetores de alvo.
- As camadas ocultas usam `tanh`; a camada de saída é linear, o que permite saídas para tarefas de regressão.
- O treinamento usa retropropagação, atualizações Adam, embaralhamento da ordem das amostras e mini-lotes.
- `perdaFinal` representa o erro quadrático médio calculado sobre o conjunto de dados fornecido.
- Cada camada oculta aceita de 1 a 64 neurônios. A camada de saída também tem limite de 64 neurônios.
- Entradas, alvos e valores de previsão devem ser finitos; as dimensões das amostras devem ser consistentes.
- A implementação usa `std::vector` para dados e buffers de trabalho. Considere a RAM disponível ao escolher o tamanho do conjunto de dados e da rede.

Exemplo de configuração:

```cpp
modelo.neuroniosPorCamada = {8, 4}; // Duas camadas ocultas
modelo.epocas = 300;
modelo.tamanhoLote = 8;
modelo.taxaAprendizado = 0.01f;
```

## Exemplos

Cada diretório contém um sketch Arduino independente. Os exemplos usam conjuntos compactos para demonstrar a biblioteca; os dados de treino e os resultados são ilustrativos, não um modelo geral pré-treinado.

| Exemplo | Sketch | Demonstração |
|---|---|---|
| Modelo básico Seai | [`examples/Seai/Seai.ino`](examples/Seai/Seai.ino) | Treinamento e inferência com duas características numéricas. |
| Classificação de texto | [`examples/ClassificacaoPLN/ClassificacaoPLN.ino`](examples/ClassificacaoPLN/ClassificacaoPLN.ino) | Classificação com vocabulário explícito; não é um modelo geral de linguagem. |
| Classificação de sentimentos | [`examples/ClassificacaoSentimentos/ClassificacaoSentimentos.ino`](examples/ClassificacaoSentimentos/ClassificacaoSentimentos.ino) | Demonstração compacta de classificação de sentimento. |
| Previsão de bateria | [`examples/PrevisaoBateria/PrevisaoBateria.ino`](examples/PrevisaoBateria/PrevisaoBateria.ino) | Estimativa por regressão usando valores relacionados à bateria. |
| Anomalia em hélice | [`examples/AnomaliaHelice/AnomaliaHelice.ino`](examples/AnomaliaHelice/AnomaliaHelice.ino) | Análise de anomalias demonstrativa com base na corrente do motor. |
| Gravador I2S | [`examples/GravadorExemplo/GravadorExemplo.ino`](examples/GravadorExemplo/GravadorExemplo.ino) | Captura de áudio em hardware ESP32-S3 compatível. |
| Persistência no RP2040 | [`examples/RP2040_16MB/RP2040_16MB.ino`](examples/RP2040_16MB/RP2040_16MB.ino) | Persistência de pesos e previsão em configuração RP2040 compatível. |

As imagens dos exemplos estão em [`imagens/`](imagens/). Abra cada sketch separadamente, configure placa e comunicação serial e siga as instruções exibidas por ele.

### Exemplo de persistência no RP2040

O sketch `RP2040_16MB` habilita a persistência de pesos usando `SALVAR_PESO`. Ele demonstra a relação linear `y = 2x` e solicita um valor de previsão pelo Monitor Serial. O exemplo usa uma área de EEPROM emulada de 4096 bytes; os 16 MB de flash anunciados pela placa **não** são usados como uma alocação de EEPROM de 16 MB. Use o core Arduino-Pico e um perfil compatível com a configuração de flash selecionada.

## Módulos opcionais

### Persistência de pesos

A persistência de pesos é opcional e pode ser habilitada no sketch por configuração de compilação, como demonstrado no exemplo RP2040. Quando ativada, o modelo tenta carregar pesos salvos compatíveis e grava os pesos finais após o treinamento. Se não houver pesos salvos ou se forem inválidos/incompatíveis, o treinamento continua com pesos inicializados novamente. Verifique a implementação de armazenamento e a capacidade disponível na placa de destino.

### Armazenamento CSV

O módulo `Armazenamento` oferece operações CSV sobre LittleFS, incluindo montagem do sistema de arquivos, leitura de colunas numéricas de entrada e saída, gravação de conjunto de dados e inclusão de registros. Na leitura, a quantidade de colunas de entrada é especificada; as colunas restantes são tratadas como saídas. O arquivo precisa conter cabeçalho e pelo menos uma amostra, com colunas numéricas consistentes.

A inclusão do módulo depende da configuração de compilação. Antes de usá-lo, confirme se a arquitetura e o ambiente de destino habilitam o módulo e fornecem a implementação LittleFS esperada.

### Gravador de áudio I2S

O módulo de gravação é destinado a ESP32-S3 com Arduino-ESP32 core 3.x e microfone compatível com INMP441. A configuração padrão usa taxa de amostragem de 16 kHz, áudio WAV PCM mono de 16 bits e GPIOs BCLK/WS/DIN 4/5/9. A duração padrão é 2 segundos e é limitada ao intervalo de 1 a 10 segundos. É possível informar duração e pinos no construtor do gravador.

O sketch aguarda o caractere `G` pela Serial; quando recebido, grava o áudio e envia o cabeçalho WAV seguido das amostras PCM. Confirme se a placa expõe os pinos selecionados e se o microfone está ligado ao canal utilizado pelo exemplo.

Para ligação, configuração e uso da ferramenta complementar de gravação para Windows/Python, consulte [`ferramentas/gravadorVoz/README.md`](ferramentas/gravadorVoz/README.md). Essa ferramenta tem requisitos próprios; não presuma que as instruções se aplicam sem alterações a outros sistemas operacionais ou hardwares.

## Estrutura do repositório

```text
.
├── README.md                     # Documentação em português
├── README.en.md                  # Documentação em inglês
├── GLOSSARIO.md                  # Glossário em português
├── LICENSE
├── library.properties
├── src/
│   ├── Seai.h                    # Cabeçalho público principal
│   ├── Adam/                     # Otimizador Adam
│   ├── Aprendizagem/             # Rotinas de aprendizagem
│   ├── Armazenamento/            # Armazenamento CSV via LittleFS
│   ├── Camadas/                  # Camadas da rede
│   ├── Compilar/                 # Suporte de compilação do modelo/treino
│   ├── Conversor/                # Utilitários de conversão de dados
│   ├── Epocas/                   # Controle de épocas
│   ├── Gradientes/               # Gradientes e retropropagação
│   ├── Gravador/                 # Gravador de áudio I2S
│   ├── Modelo/                   # API do modelo
│   ├── Neuronios/                # Implementação de neurônios
│   ├── Perdas/                   # Funções de perda
│   ├── Pesos/                    # Persistência de pesos
│   ├── Placas/                   # Detecção e suporte de placas
│   ├── Prever/                   # Inferência
│   ├── Treinar/                  # Suporte ao treinamento
│   └── Verbose/                  # Diagnósticos pela Serial
├── examples/                     # Sketches Arduino independentes
├── ferramentas/gravadorVoz/      # Ferramenta de gravação de amostras de voz
└── imagens/                      # Imagens dos exemplos
```

O ponto de inclusão público é `Seai.h`. Os módulos individuais são organizados sob `src/`; módulos específicos de hardware ou recursos podem ser incluídos condicionalmente conforme a configuração da compilação.

## Entrada pelo Monitor Serial

Vários exemplos recebem valores pelo Monitor Serial. Configure a velocidade indicada no sketch e selecione um final de linha aceito, normalmente **Nova linha** ou **NL e CR**. Informe valores no formato solicitado pelo exemplo; as entradas numéricas geralmente usam ponto como separador decimal. O gravador transmite áudio pela Serial a 921600 baud, portanto sua configuração difere dos exemplos básicos de ML.

## Limitações e recomendações

- A Seai é destinada às arquiteturas ESP32 e RP2040 suportadas; não é compatível com placas AVR, como Arduino Uno.
- O treinamento é executado no microcontrolador. Tempo de execução e consumo de RAM aumentam com o conjunto de dados, número de características, largura das camadas e quantidade de épocas.
- Mantenha dimensões de rede e conjunto de treino adequados à memória e à capacidade de processamento da placa.
- Prepare as características de entrada adequadamente para o problema. A biblioteca valida valores finitos e dimensões, mas não prepara nem normaliza automaticamente os dados.
- Os conjuntos de exemplo são demonstrativos e não garantem acurácia. Use dados representativos e avalie o modelo de forma independente para a aplicação pretendida.
- Pesos persistidos dependem da configuração de armazenamento; confirme capacidade e compatibilidade na placa real.
- A captura de áudio depende da combinação especificada de ESP32-S3, versão do core, microfone e configuração de pinos.

## Contribuições

Relatos de problemas, correções e melhorias podem ser enviados pelos fluxos de issues e pull requests do GitHub. Ao relatar um problema, informe o modelo da placa, versão do pacote/core, versão da Arduino IDE, sketch utilizado e a saída completa de compilação ou execução.

## Licença e créditos

Desenvolvido por **Bruno Maker**. Consulte [`LICENSE`](LICENSE) para conhecer os termos aplicáveis.

- Repositório: [brunomakeram/Small-Embedded-Ai](https://github.com/brunomakeram/Small-Embedded-Ai)
- Site do projeto: [small-embedded-ai.netlify.app](https://small-embedded-ai.netlify.app/)
