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
- [Referência da API](#referência-da-api)
  - [Modelo](#1-modelomodelo)
  - [Adam](#2-adamadam)
  - [Neuronios](#3-neuroniosneuronios)
  - [Camadas](#4-camadascamadas)
  - [Compilar](#5-compilarcompilar)
  - [Aprendizagem e Epocas](#6-aprendizagem-e-epocas)
  - [Treinar, Gradientes, Perdas e Prever](#7-treinar-gradientes-perdas-e-prever)
  - [Conversor](#8-conversor)
  - [Armazenamento CSV](#9-armazenamento-csv)
  - [Persistência de pesos](#10-persistência-de-pesos)
  - [Verbose](#11-verbose)
  - [Gravador I2S](#12-gravador-i2s)
  - [Detecção de placa](#13-detecção-de-placa)
- [Exemplos do repositório](#exemplos-do-repositório)
- [Limitações e recomendações](#limitações-e-recomendações)
- [Contribuições](#contribuições)
- [Licença e créditos](#licença-e-créditos)

## Recursos

- Treinamento embarcado de redes neurais feed-forward a partir de matrizes numéricas.
- Camadas ocultas configuráveis, épocas, lotes e taxa de aprendizado.
- Otimização Adam e retropropagação do erro.
- Inferência usando `prever(...)`.
- Erro quadrático médio (MSE), disponível em `perdaFinal`.
- Persistência opcional de pesos compatíveis.
- Leitura e gravação de dados CSV via LittleFS nas configurações compatíveis.
- Captura de áudio PCM mono por I2S no ESP32-S3.
- Logs pela Serial por meio de `Verbose`.

## Hardware e requisitos

| Item | Requisito / situação |
|---|---|
| Arquiteturas suportadas | ESP32 e RP2040 (família Raspberry Pi Pico) |
| Ambiente Arduino | Arduino IDE com a biblioteca Seai instalada |
| Placas AVR | Não suportadas, incluindo Arduino Uno |
| Gravador de áudio | ESP32-S3, Arduino-ESP32 core 3.x e microfone INMP441 compatível |
| Exemplo UF2 para RP2040 | Core Arduino-Pico e placa/perfil compatível com a configuração de flash selecionada |

Selecione o pacote de placas e o perfil corretos antes de compilar. Módulos específicos podem exigir configurações adicionais de pinos, versão do core, flash ou Serial.

## Instalação

Instale a Seai como biblioteca Arduino usando este repositório ou a pasta da biblioteca e reinicie a IDE, se necessário. Os metadados em `library.properties` declaram nome `Seai`, versão `3.0.0` e arquiteturas `esp32,rp2040`.

```bash
git clone https://github.com/brunomakeram/Small-Embedded-Ai.git
```

Abra cada sketch individualmente em `examples/`, selecione a placa e a porta e siga as instruções específicas do exemplo.

## Início rápido

O cabeçalho público `Seai.h` inclui os módulos compatíveis com a arquitetura detectada. A interface recomendada para a maioria dos projetos é `Modelo::Modelo`.

```cpp
#include <Seai.h>
#include <vector>

Modelo::Modelo modelo;

const std::vector<std::vector<float>> entradas = {
    {0.0f, 0.0f}, {0.0f, 1.0f},
    {1.0f, 0.0f}, {1.0f, 1.0f}
};
const std::vector<std::vector<float>> alvos = {
    {0.0f}, {1.0f}, {1.0f}, {0.0f}
};

void setup() {
    Serial.begin(115200);
    modelo.neuroniosPorCamada = {8};
    modelo.epocas = 500;
    modelo.tamanhoLote = 4;

    if (!modelo.executar(entradas, alvos, {0.0f, 1.0f})) {
        Serial.println("Falha no treinamento.");
        return;
    }

    std::vector<float> previsao = modelo.prever({0.0f, 1.0f});
    if (!previsao.empty()) {
        Serial.printf("Saída: %.4f; MSE: %.6f\n", previsao[0], modelo.perdaFinal);
    }
}

void loop() {}
```

Cada amostra de `entradas` deve possuir uma amostra correspondente em `alvos`. Todas as linhas precisam ter as mesmas dimensões e valores finitos. O vetor enviado a `prever` deve ter o mesmo número de características de cada entrada de treino.

# Referência da API

Os nomes abaixo correspondem à API pública declarada nos cabeçalhos da versão 3.0.0. Exemplos de baixo nível são destinados a quem precisa compor componentes manualmente; para uso comum, prefira `Modelo::Modelo`, que encadeia automaticamente treinamento, camadas, gradientes e previsão.

## 1. `Modelo::Modelo`

Cabeçalho: `#include <Seai.h>` ou `#include <Modelo/Modelo.h>`.

### Propriedades públicas

| Propriedade | Tipo | Padrão | Significado |
|---|---:|---:|---|
| `taxaAprendizado` | `float` | `0.05f` | Tamanho da atualização do otimizador Adam. Deve ser finito e maior que zero. Valores maiores tendem a produzir atualizações maiores; ajuste empiricamente. |
| `epocas` | `int` | `500` | Número de passagens completas pelo conjunto de treino. Deve ser pelo menos 1. |
| `tamanhoLote` | `int` | `32` | Número máximo de amostras acumuladas antes de atualizar os parâmetros. Valor menor que 1 é convertido para 1. O último lote pode ser menor. |
| `neuroniosPorCamada` | `std::vector<int>` | `{8}` | Número de neurônios de cada camada oculta, na ordem da entrada para a saída. Cada valor aceito deve estar entre 1 e 64. A largura de entrada e saída é inferida dos dados. |
| `perdaFinal` | `float` | `0.0f` | MSE recalculado sobre todas as amostras depois do treino. |
| `verbose` | `Verbose::Verbose` | opções desligadas | Controla os logs de configuração, número de épocas e perda por época. |

### `bool executar(entradas, saidasEsperadas, entradaPrevisao)`

Valida os dados, constrói a rede, treina, calcula `perdaFinal`, salva pesos se a opção de persistência estiver ligada e processa `entradaPrevisao`. Retorna `true` se o fluxo de treinamento for concluído e `false` se a validação de dados/configuração falhar.

| Parâmetro | Tipo | Descrição |
|---|---|---|
| `entradas` | `const std::vector<std::vector<float>>&` | Matriz de amostras. Cada linha contém as características de uma amostra. Não pode estar vazia. |
| `saidasEsperadas` | `const std::vector<std::vector<float>>&` | Matriz de alvos, com o mesmo número de linhas de `entradas`; a largura de todas as linhas define a dimensão da saída. |
| `entradaPrevisao` | `const std::vector<float>&` | Amostra a processar ao fim do treino. Deve ter a mesma largura de cada linha de `entradas`. |

**Importante:** `executar` treina o modelo mesmo quando carrega pesos persistidos compatíveis. O carregamento não transforma o método em uma operação somente de inferência.

### `std::vector<float> prever(entrada) const`

Executa inferência com os parâmetros atualmente mantidos pelo objeto. `entrada` deve conter uma característica por entrada da rede e valores finitos. Se o modelo ainda não foi treinado ou a entrada for inválida, imprime uma mensagem na Serial e retorna um vetor vazio. A saída pode conter um ou mais valores, conforme o número de colunas dos alvos.

### Exemplo: regressão `y = 2x`

```cpp
#include <Seai.h>
#include <vector>

Modelo::Modelo modelo;
const std::vector<std::vector<float>> x = {{0}, {1}, {2}, {3}};
const std::vector<std::vector<float>> y = {{0}, {2}, {4}, {6}};

void setup() {
    Serial.begin(115200);
    modelo.neuroniosPorCamada = {2};
    modelo.epocas = 100;
    modelo.tamanhoLote = 4;

    if (modelo.executar(x, y, {3.0f})) {
        const auto resultado = modelo.prever({3.0f});
        if (!resultado.empty()) Serial.println(resultado[0], 4); // esperado próximo de 6
    }
}
void loop() {}
```

## 2. `Adam::Adam`

Cabeçalho: `#include <Adam/Adam.h>`. `Modelo` configura Adam internamente; use esta classe diretamente para controlar a atualização de vetores de parâmetros em fluxos de baixo nível.

### Propriedades

| Propriedade | Tipo | Padrão | Significado |
|---|---:|---:|---|
| `taxaAprendizado` | `float` | `0.05f` | Escala das atualizações. |
| `decaimentoPrimeiroMomento` | `float` | `0.9f` | Decaimento da média móvel dos gradientes. |
| `decaimentoSegundoMomento` | `float` | `0.999f` | Decaimento da média móvel dos gradientes ao quadrado. |
| `margemEstabilidadeNumerica` | `float` | `1e-7f` | Termo adicionado ao denominador para estabilidade numérica. |
| `numeroAtualizacoes` | `long` | `0` | Contador de atualizações aplicado na correção de viés dos momentos. |

### Construtor e métodos

- `Adam(taxaAprendizadoPadrao=0.05f, decaimentoPrimeiroMomentoPadrao=0.9f, decaimentoSegundoMomentoPadrao=0.999f, margemEstabilidadeNumericaPadrao=1e-7f)`: cria e inicializa o estado do otimizador. Os quatro parâmetros correspondem às propriedades da tabela.
- `void inicializar(taxaAprendizadoPadrao, decaimentoPrimeiroMomentoPadrao, decaimentoSegundoMomentoPadrao, margemEstabilidadeNumericaPadrao)`: substitui os hiperparâmetros e zera contador e momentos. Use ao iniciar um novo treinamento ou quando mudar o tamanho do vetor de parâmetros.
- `void atualizarParametros(std::vector<float>& parametros, const std::vector<float>& gradientes, int numeroAmostrasLote)`: atualiza `parametros` in-place. `gradientes` deve ter o mesmo tamanho e conter a soma dos gradientes do lote; o método divide pelo número de amostras. Se o tamanho for incompatível, vazio ou `numeroAmostrasLote <= 0`, a operação não atualiza os parâmetros e pode registrar erro pela Serial.

Exemplo de chamada isolada (os gradientes são fornecidos pelo chamador):

```cpp
Adam::Adam adam(0.01f);
std::vector<float> parametros = {0.5f, -0.2f};
const std::vector<float> gradientes = {0.1f, -0.4f};
adam.atualizarParametros(parametros, gradientes, 1);
```

## 3. `Neuronios::Neuronios`

Cabeçalho: `#include <Neuronios/Neuronios.h>`. Representa uma camada totalmente conectada, com uma linha de pesos e um viés por neurônio.

### Constantes, campos e métodos públicos

- `NUMERO_NEURONIOS_MINIMO = 1`, `NUMERO_NEURONIOS_PADRAO = 8`, `NUMERO_NEURONIOS_MAXIMO = 64`.
- `numeroEntradas`: quantidade de valores esperados na entrada da camada.
- `numeroNeuronios`: número de saídas/neuronios da camada.
- `pesosCamada`: matriz `[neurônio][entrada]` de pesos treináveis.
- `viesCamada`: vetor de vieses, um por neurônio.
- `pesos[2]` e `vies`: campos legados da API linear; não são a representação geral da matriz de pesos.
- `configurar(int quantidadeEntradas, int quantidadeNeuronios=8)`: reserva dimensões e zera pesos/vieses. Limita a quantidade de neurônios ao intervalo de 1 a 64; valide `quantidadeEntradas >= 1` no chamador.
- `float gerarNumeroAleatorio()`: devolve valor pseudoaleatório entre 0 e 1 usando a fonte fornecida pela plataforma.
- `inicializar()`: inicializa pesos com escala Glorot e zera vieses. Se a camada não tiver entradas configuradas, usa configuração legada de duas entradas.
- `calcular(const std::vector<float>& entradas, std::vector<float>& saidas, bool ativacaoTanh) const`: calcula soma ponderada mais viés para cada neurônio. Exige `entradas.size() == numeroEntradas`; limpa `saidas` em caso de dimensão incompatível. Se `ativacaoTanh` for `true`, aplica `tanh`; caso contrário, deixa a saída linear.

Exemplo manual para uma camada com duas entradas e três neurônios:

```cpp
Neuronios::Neuronios camada;
camada.configurar(2, 3);
camada.inicializar();
std::vector<float> saidas;
camada.calcular({0.25f, 0.75f}, saidas, true);
```

## 4. `Camadas::Camadas`

Cabeçalho: `#include <Camadas/Camadas.h>`. Contêiner ordenado de objetos `Neuronios::Neuronios`.

- `void adicionar(int quantidadeEntradas, int quantidadeNeuronios)`: cria, configura, inicializa e acrescenta uma camada.
- `void limpar()`: remove todas as camadas.
- `std::size_t tamanho() const`: retorna o número de camadas.
- `operator[](std::size_t indice)`: acesso por índice, na ordem em que `adicionar` foi chamado; a sobrecarga não constante permite alterar pesos. O chamador deve garantir que o índice esteja dentro de `tamanho()`.

```cpp
Camadas::Camadas rede;
rede.adicionar(2, 4); // camada 0: duas entradas, quatro neurônios
rede.adicionar(4, 1); // camada 1: quatro entradas, uma saída
Serial.println((unsigned int)rede.tamanho());
```

## 5. `Compilar::Compilar`

Cabeçalho: `#include <Compilar/Compilar.h>`. Agrupa uma camada `Neuronios::Neuronios` e seu otimizador `Adam::Adam` para o caminho de treino de baixo nível.

- Campo `neuronios`: camada treinável.
- Campo `otimizador`: estado Adam associado à camada.
- `bool configurar(int quantidadeEntradas, int quantidadeSaidas, float taxaAprendizado=0.05f, float decaimentoPrimeiroMomento=0.9f, float decaimentoSegundoMomento=0.999f, float margemEstabilidadeNumerica=1e-7f)`: valida dimensões, configura/inicializa neurônios e reinicializa Adam; retorna `false` se a quantidade de entradas ou saídas não for válida, ou se saídas excederem 64.

Os parâmetros Adam têm os mesmos significados descritos na seção [Adam](#2-adamadam).

```cpp
Compilar::Compilar compilado;
if (!compilado.configurar(2, 1, 0.01f)) {
    Serial.println("Dimensões inválidas");
}
```

## 6. `Aprendizagem` e `Epocas`

### `Aprendizagem::Aprendizagem::treinarModelo`

Cabeçalho: `#include <Aprendizagem/Aprendizagem.h>`.

```cpp
float treinarModelo(
    const std::vector<std::vector<float>>& entradas,
    const std::vector<std::vector<float>>& saidasEsperadas,
    Compilar::Compilar& modelo,
    int quantidadeEpocas,
    int tamanhoLote = 32);
```

Encaminha o treino para `Epocas::Epocas::executar` e retorna a perda MSE da última época. `entradas` e `saidasEsperadas` precisam ter linhas correspondentes e dimensões compatíveis com `modelo.neuronios`; `quantidadeEpocas` deve ser pelo menos 1. `tamanhoLote` é limitado a pelo menos 1 dentro do executor.

### `Epocas::Epocas::executar`

```cpp
float executar(
    int quantidadeEpocas,
    const std::vector<std::vector<float>>& entradas,
    const std::vector<std::vector<float>>& saidasEsperadas,
    Compilar::Compilar& modelo,
    int tamanhoLote = 32);
```

Valida épocas, dimensões, valores finitos e configuração da camada; embaralha índices das amostras a cada época e solicita o processamento dos lotes a `Treinar`. Retorna a perda final calculada. Em dados inválidos, imprime erro e retorna `0.0f` (portanto, não use apenas o valor zero para distinguir falha de uma perda zero).

## 7. `Treinar`, `Gradientes`, `Perdas` e `Prever`

Essas classes são componentes de baixo nível. Normalmente são chamadas pelo executor `Epocas`; use-as diretamente apenas quando precisar controlar o pipeline.

### `Treinar::Treinar::executarLotes`

Processa todos os lotes de uma época e atualiza os parâmetros via Adam. Recebe `entradas`, `saidasEsperadas`, uma permutação de `indices`, `tamanhoLote`, referências para `Compilar`, `Gradientes`, `Perdas` e `Prever`. Retorna a soma das perdas dos lotes ponderada pelas respectivas amostras; dados, índices ou dimensões inválidos geram mensagem Serial e retorno `0.0f`.

### `Gradientes::Gradientes::processarAmostraLote`

```cpp
bool processarAmostraLote(
    const std::vector<float>& entradas,
    const std::vector<float>& saidasEsperadas,
    const Neuronios::Neuronios& neuronios,
    std::vector<std::vector<float>>& gradientesPesos,
    std::vector<float>& gradientesVieses,
    float& somaErroQuadrado,
    Prever::Prever& previsor);
```

Calcula a previsão de uma amostra, acumula gradientes de pesos/vieses e soma os erros quadráticos em `somaErroQuadrado`. Os acumuladores devem estar previamente dimensionados para uma linha por saída e uma coluna por entrada. Retorna `false` se previsão, dimensões ou valores forem inválidos.

### `Perdas::Perdas::calcularMSE`

`float calcularMSE(float somaErrosQuadrados, int totalAmostras)` retorna `somaErrosQuadrados / totalAmostras`. Retorna `0.0f` se `totalAmostras <= 0`. No fluxo de baixo nível, passe o denominador adequado ao conjunto de valores cujo erro foi acumulado.

### `Prever::Prever::calcular`

`bool calcular(const std::vector<float>& entradas, const Neuronios::Neuronios& neuronios, std::vector<float>& saidas) const` executa uma camada linear (sem `tanh`), valida dimensão e finitude e preenche `saidas`. Em caso de erro, limpa o vetor e retorna `false`.

## 8. `Conversor::Conversor`

Cabeçalho: `#include <Conversor/Conversor.h>`. Converte buffers tipados, representados por bytes. A memória apontada por `entrada` e `saida` deve ter espaço suficiente para `quantidade` elementos com as larguras declaradas.

### Tipos de dados

| Constante | Valor | Bytes por elemento | Observação |
|---|---:|---:|---|
| `FLOAT32` | 0 | 4 | `float` de 32 bits; tipo de origem padrão quando o código de tipo recebido é inválido. |
| `INT32` | 1 | 4 | Inteiro com sinal de 32 bits. |
| `INT16` | 2 | 2 | Inteiro com sinal de 16 bits. |
| `INT8` | 3 | 1 | Inteiro com sinal de 8 bits. |
| `UINT8` | 4 | 1 | Inteiro sem sinal de 8 bits. |
| `STRING` | 5 | largura configurada | Texto em campos de largura fixa. |

`TETO_BYTES_TEXTO` é o limite de 100000 bytes usado nas validações para operações com texto.

### Construtor e métodos

- `Conversor(uint8_t tipoDadoEscolhido)`: define o tipo de origem do objeto. Código acima de `STRING` é substituído por `FLOAT32`.
- `uint8_t tamanhoBytes() const`: retorna 4, 2, 1 ou 0 para texto/tipo sem tamanho fixo.
- `bool converterDado(uint8_t destino, const uint8_t* entrada, uint8_t* saida, size_t quantidade, size_t larguraEntrada=0, size_t larguraSaida=0) const`: converte `quantidade` elementos do tipo de origem do objeto para `destino`.
  - `destino`: uma das constantes de tipo acima; destino fora do intervalo é recusado.
  - `entrada`/`saida`: buffers binários de origem e destino, sem conversão automática de endianness.
  - `quantidade`: número de elementos a processar.
  - `larguraEntrada`: bytes por elemento quando a origem é `STRING`.
  - `larguraSaida`: bytes por elemento quando destino é `STRING`.
  - Texto para texto copia o menor entre as larguras e preenche o restante da saída com zero. Texto numérico para tipo numérico é interpretado com `atof`; texto não numérico vira zero. Conversões para inteiros arredondam; `INT8`, `INT16` e `UINT8` são limitados à faixa representável.
- `static size_t larguraMinima(const uint8_t* entrada, size_t quantidade, size_t larguraEntrada)`: calcula o comprimento da maior string em campos fixos, incluindo um byte adicional para terminador NUL. Cada campo deve ter até `larguraEntrada` bytes.

### Exemplo: `float` para `int16_t`

```cpp
Conversor::Conversor conversor(Conversor::Conversor::FLOAT32);
float valores[] = {1.2f, -2.7f};
int16_t inteiros[2] = {};
bool ok = conversor.converterDado(
    Conversor::Conversor::INT16,
    reinterpret_cast<const uint8_t*>(valores),
    reinterpret_cast<uint8_t*>(inteiros),
    2);
```

Garanta que os buffers estejam alinhados/adequadamente dimensionados para o tipo e a quantidade usados. O conversor trabalha sobre bytes e não substitui validação semântica ou normalização dos dados.

## 9. Armazenamento CSV

Cabeçalho: `#include <Armazenamento/Armazenamento.h>`. O módulo depende de LittleFS e é incluído condicionalmente pela Seai nas configurações suportadas (automaticamente habilitado para ESP32 pelas macros atuais).

A classe `Armazenamento::Armazenamento` oferece:

- `bool montar(const char* caminho)`: monta LittleFS e, se o caminho ainda não existir, cria-o com `DATASET_PADRAO`. `caminho` não pode ser nulo ou vazio. Como a montagem usa `LittleFS.begin(true)`, verifique o comportamento de formatação/recuperação do core antes de usar dados importantes.
- `bool lerDados(const char* caminho, std::size_t quantidadeEntradas, std::vector<std::vector<float>>& entradas, std::vector<std::vector<float>>& saidas)`: lê um CSV com cabeçalho; as primeiras `quantidadeEntradas` colunas numéricas são entradas e as colunas restantes são saídas. Requer caminho válido, ao menos uma coluna de cada tipo, cabeçalho e uma ou mais amostras. Em sucesso substitui os vetores de saída.
- `bool gravarDados(const char* caminho, const std::vector<String>& nomesEntradas, const std::vector<String>& nomesSaidas, const std::vector<std::vector<float>>& entradas, const std::vector<std::vector<float>>& saidas)`: grava/substitui o CSV com os nomes de colunas e matrizes informados. Exige nomes e dados não vazios, mesmo número de amostras, dimensões correspondentes aos nomes e valores finitos. Campos de cabeçalho com vírgula/aspas são escapados.
- `bool gravarRegistro(const char* caminho, const std::vector<float>& entradas, const std::vector<float>& saidas)`: acrescenta uma linha ao CSV existente, validando a quantidade de colunas pelo cabeçalho. Não cria o arquivo nem o cabeçalho.

Exemplo de ciclo de dados (ESP32 com suporte LittleFS):

```cpp
#include <Seai.h>

Armazenamento::Armazenamento csv;
std::vector<std::vector<float>> entradas, saidas;

void setup() {
    Serial.begin(115200);
    if (!csv.montar("/treino.csv")) return; // cria dataset de exemplo se ausente
    if (!csv.lerDados("/treino.csv", 2, entradas, saidas)) return;
    // As matrizes lidas podem ser passadas a modelo.executar(...).
}
void loop() {}
```

O arquivo precisa ser CSV numérico (exceto cabeçalho), com o mesmo número de campos em todas as linhas. Números inválidos, `NaN` e infinitos são recusados.

## 10. Persistência de pesos

Cabeçalho: `#include <Pesos/Pesos.h>`. Namespace `PersistenciaPesos`; classe `Pesos`; enum `ResultadoCarregamento` com `PESOS_CARREGADOS`, `PESOS_AUSENTES` e `PESOS_INVALIDOS`.

- `static bool salvar(const Camadas::Camadas& camadas)`: grava pesos e vieses. ESP32 usa LittleFS; o caminho interno é `/seai_pesos.csv`. No RP2040 usa EEPROM emulada, inicializada pelo módulo com 4096 bytes e gravada com `EEPROM.commit()`.
- `static ResultadoCarregamento carregar(Camadas::Camadas& camadas)`: carrega somente quando versão, número de camadas, dimensões e valores são compatíveis; distingue pesos ausentes de inválidos.

No uso normal, habilite a opção antes de incluir `Seai.h`:

```cpp
#define SALVAR_PESO SALVAR_PESO_LIGADO
#include <Seai.h>
```

Sem essa definição, `SALVAR_PESO` assume `SALVAR_PESO_DESLIGADO`. A arquitetura e o perfil de flash determinam o espaço real disponível; dimensione a rede para caber na área persistente.

## 11. `Verbose::Verbose`

Acessível em `modelo.verbose` ou por `#include <Verbose/Verbose.h>`.

| Campo | Padrão | Efeito quando `true` |
|---|---:|---|
| `mostrarConfiguracao` | `false` | Imprime taxa, épocas, lote, camadas ocultas e largura de saída no começo do treinamento. |
| `mostrarNumeroEpoca` | `false` | Imprime o número de cada época concluída. |
| `mostrarPerdaPorEpoca` | `false` | Imprime a perda por época. No `Modelo`, também ativa o cálculo da perda completa a cada época, aumentando o custo de execução. |

Os métodos públicos `registrarConfiguracao(taxaAprendizado, epocas, tamanhoLote, neuroniosPorCamada, numeroSaidas)`, `registrarEpoca(epoca)`, `registrarPerdaEpoca(perda)` e `mostrarResultados(...)` são principalmente pontos de integração usados internamente pelo modelo. A perda final é impressa pela implementação atual mesmo sem ativar as opções de log.

```cpp
modelo.verbose.mostrarConfiguracao = true;
modelo.verbose.mostrarNumeroEpoca = true;
modelo.verbose.mostrarPerdaPorEpoca = false; // evita recalcular MSE a cada época
```

## 12. Gravador I2S

Disponível condicionalmente em ESP32 com Arduino-ESP32 core 3.x. Cabeçalho: `#include <Gravador/Gravador.h>` (o include pode também ser feito pelo cabeçalho principal quando habilitado).

### Construtor

`Gravador::Gravador(int tempoGravacaoSegundos=2, int pinoBclk=4, int pinoWs=5, int pinoDin=9)`:

- `tempoGravacaoSegundos`: duração em segundos, limitada automaticamente a 1–10.
- `pinoBclk`: GPIO do clock BCLK.
- `pinoWs`: GPIO word-select / WS.
- `pinoDin`: GPIO de dados recebido do microfone.

### Métodos

- `void iniciar()`: configura Serial a 921600 baud, cabeçalho WAV PCM mono 16 kHz/16-bit e canal I2S de recepção. Chame uma vez em `setup()`.
- `void gravar()`: retorna se não há dados disponíveis; consome um byte e inicia captura somente se esse byte for `G`. Captura a duração configurada e envia 44 bytes de cabeçalho WAV seguidos pelos dados PCM. No hardware esperado, usa também o LED RGB integrado do GPIO 48.

```cpp
#include <Arduino.h>
#include <Gravador/Gravador.h>

Gravador::Gravador gravador(2, 4, 5, 9);
void setup() { gravador.iniciar(); }
void loop() { gravador.gravar(); }
```

Requisitos adicionais e ligação do INMP441 estão em [`ferramentas/gravadorVoz/README.md`](ferramentas/gravadorVoz/README.md). O gravador Python incluso é documentado como ferramenta para Windows. Confirme pinos, USB CDC e taxa serial na placa utilizada.

## 13. Detecção de placa

`Placas/Placas.h` é incluído automaticamente por `Seai.h` e define as macros de plataforma:

- `SEAI_ESP32`: definido como 1 quando detecta macros ESP32 compatíveis.
- `SEAI_RP2040`: definido como 1 quando detecta `ARDUINO_ARCH_RP2040`.
- `SEAI_TEM_GRAVADOR`: habilitado automaticamente para ESP32 quando `ESP_ARDUINO_VERSION_MAJOR >= 3`.
- `SEAI_TEM_ARMAZENAMENTO`: habilitado automaticamente para ESP32.
- `SALVAR_PESO_LIGADO` / `SALVAR_PESO_DESLIGADO`: valores de configuração; `SALVAR_PESO` usa desligado como padrão.

A compilação gera erro quando nenhuma plataforma ou mais de uma plataforma é detectada. Em uma placa compatível cuja plataforma não seja identificada pelo core, a macro adequada deve ser definida **antes** do include de `Seai.h`, conforme a mensagem de compilação. `esp_random()` é fornecida pelo ESP32 ou adaptada via `random()` no RP2040 para inicialização/embaralhamento.

## Exemplos do repositório

| Exemplo | Sketch | Demonstração |
|---|---|---|
| Modelo básico | [`examples/Seai/Seai.ino`](examples/Seai/Seai.ino) | Treinamento e inferência com duas características. |
| Classificação de texto | [`examples/ClassificacaoPLN/ClassificacaoPLN.ino`](examples/ClassificacaoPLN/ClassificacaoPLN.ino) | Classificação baseada em vocabulário explícito, não um modelo geral de linguagem. |
| Sentimentos | [`examples/ClassificacaoSentimentos/ClassificacaoSentimentos.ino`](examples/ClassificacaoSentimentos/ClassificacaoSentimentos.ino) | Demonstração de classificação de sentimento. |
| Previsão de bateria | [`examples/PrevisaoBateria/PrevisaoBateria.ino`](examples/PrevisaoBateria/PrevisaoBateria.ino) | Estimativa/regressão com dados relacionados à bateria. |
| Anomalia em hélice | [`examples/AnomaliaHelice/AnomaliaHelice.ino`](examples/AnomaliaHelice/AnomaliaHelice.ino) | Análise demonstrativa da corrente do motor. |
| Gravador | [`examples/GravadorExemplo/GravadorExemplo.ino`](examples/GravadorExemplo/GravadorExemplo.ino) | Captura de áudio em ESP32-S3 compatível. |
| Persistência RP2040 | [`examples/RP2040_16MB/RP2040_16MB.ino`](examples/RP2040_16MB/RP2040_16MB.ino) | Persistência de pesos e regressão `y = 2x`. |

Os dados são compactos e demonstrativos; a qualidade em uma aplicação real depende de dados representativos. Imagens estão em [`imagens/`](imagens/).

### RP2040 e EEPROM emulada

O sketch `RP2040_16MB` habilita persistência por `SALVAR_PESO`. A EEPROM emulada reserva 4096 bytes; os 16 MB de flash não são usados como EEPROM de 16 MB. Use o core Arduino-Pico e um perfil compatível com o hardware/flash selecionado.

## Estrutura do repositório

```text
.
├── README.md
├── README.en.md
├── GLOSSARIO.md
├── LICENSE
├── library.properties
├── src/                 # API e módulos C++
├── examples/            # sketches Arduino independentes
├── ferramentas/         # ferramenta de gravação de voz
└── imagens/             # imagens dos exemplos
```

## Limitações e recomendações

- O treinamento roda no microcontrolador; tempo e RAM aumentam com dados, características, camadas e épocas. Use dimensões compatíveis com os recursos da placa.
- A biblioteca valida formato/dimensões e valores finitos, mas não normaliza os dados automaticamente.
- Saída linear é adequada a regressão, mas tarefas de classificação podem exigir codificação de alvos e interpretação do resultado feita pela aplicação.
- Exemplos não garantem acurácia. Avalie o modelo com amostras representativas e separadas do treino.
- Persistência depende da configuração de armazenamento e do perfil da placa. Não presuma que a memória flash total esteja disponível para pesos.
- O gravador requer a combinação de hardware, core, pinos e configuração Serial descrita nesta documentação.

## Contribuições

Issues, correções e melhorias podem ser enviados pelo GitHub. Em relatos de erro, inclua placa, versão do core/pacote, Arduino IDE, sketch e saída completa de compilação ou execução.

## Licença e créditos

Desenvolvido por **Bruno Maker**. Consulte [`LICENSE`](LICENSE) para os termos aplicáveis.

- Repositório: [brunomakeram/Small-Embedded-Ai](https://github.com/brunomakeram/Small-Embedded-Ai)
- Site: [small-embedded-ai.netlify.app](https://small-embedded-ai.netlify.app/)
