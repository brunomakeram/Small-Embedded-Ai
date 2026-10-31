# Glossário da biblioteca Maker

Este documento descreve cada classe da pasta `src` com base no código real da biblioteca. A ideia é explicar o papel de cada módulo, os atributos e métodos públicos, como usar cada classe e exemplos de aplicação simples.

---

## Adam

### Papel da classe
A classe `Adam` implementa o otimizador Adam, usado para ajustar pesos e viés durante o treinamento do modelo.

### Atributos públicos
- `float taxaAprendizado;`
  - Armazena a taxa de aprendizado aplicada pelo algoritmo.
- `float decaimentoPrimeiroMomento;`
  - Valor do parâmetro `beta1` do Adam.
- `float decaimentoSegundoMomento;`
  - Valor do parâmetro `beta2` do Adam.
- `float margemEstabilidadeNumerica;`
  - Valor do parâmetro `eps`, usado para evitar divisão por zero.
- `long numeroAtualizacoes;`
  - Conta quantas atualizações já foram feitas.
- `float mediaMovelGradientes[3];`
  - Média móvel do gradiente para os parâmetros `w1`, `w2` e `b`.
- `float mediaMovelGradientesQuadrados[3];`
  - Média móvel do quadrado do gradiente para os mesmos parâmetros.

### Métodos públicos
- `void inicializar(float lr = 0.05f, float beta1 = 0.9f, float beta2 = 0.999f, float eps = 1e-7f)`
  - Inicializa os valores do otimizador.
  - Parâmetros:
    - `lr`: taxa de aprendizado
    - `beta1`: decaimento do primeiro momento
    - `beta2`: decaimento do segundo momento
    - `eps`: margem de estabilidade numérica
  - Retorno: `void`
- `void atualizarParametros(float *pesos, float &vies, float *gradientes, int numeroAmostrasLote)`
  - Ajusta os pesos e o viés com base nos gradientes calculados no lote atual.
  - Parâmetros:
    - `pesos`: vetor com `w1` e `w2`
    - `vies`: valor de `b`
    - `gradientes`: gradientes acumulados de `w1`, `w2` e `b`
    - `numeroAmostrasLote`: quantidade de amostras do lote
  - Retorno: `void`

### Como chamar a classe
```cpp
#include <Adam.h>

Adam otimizador;

void setup() {
    Serial.begin(115200);
    otimizador.inicializar(0.05f, 0.9f, 0.999f, 1e-7f);
}
```

### Exemplo de uso
```cpp
float pesos[2] = {0.1f, 0.2f};
float vies = 0.0f;
float gradientes[3] = {0.15f, -0.05f, 0.02f};

Adam otimizador;
otimizador.inicializar();

// Atualiza os parâmetros do modelo usando o lote atual
otimizador.atualizarParametros(pesos, vies, gradientes, 32);
```

---

## Aprendizagem

### Papel da classe
A classe `Aprendizagem` é a porta de entrada para o treinamento do modelo. Ela apenas delega ao fluxo de épocas.

### Atributos públicos
- Nenhum atributo público.

### Métodos públicos
- `float treinarModelo(Dataset &dados, Compilar &modelo, int totalEpocas, int tamanhoLote = 32)`
  - Executa o treinamento do modelo com os dados informados.
  - Parâmetros:
    - `dados`: conjunto de treinamento
    - `modelo`: modelo compilado
    - `totalEpocas`: número de épocas
    - `tamanhoLote`: tamanho do lote
  - Retorno: `float` com a perda final (MSE)

### Como chamar a classe
```cpp
#include <Aprendizagem.h>

Aprendizagem aprendizagem;
```

### Exemplo de uso
```cpp
Dataset dados;
Compilar modelo;
Aprendizagem aprendizagem;

void setup() {
    Serial.begin(115200);
    modelo.configurar(0.05f);

    float perda = aprendizagem.treinarModelo(dados, modelo, 500, 32);
    Serial.printf("Perda final: %.5f\n", perda);
}
```

---

## Armazenamento

### Papel da classe
A classe `Armazenamento` cuida do sistema de arquivos do ESP32, lê o CSV com os dados, cria o arquivo padrão quando necessário e grava registros de previsão.

### Atributos públicos
- Nenhum atributo público.

### Métodos públicos
- `bool montar(const char *caminho)`
  - Monta o sistema de arquivos LittleFS.
  - Se o arquivo informado não existir, ele é criado com o dataset padrão.
  - Parâmetros:
    - `caminho`: caminho do CSV no sistema de arquivos
  - Retorno: `bool`
- `bool lerDataset(const char *caminho, Dataset &dados)`
  - Lê o CSV, identifica as colunas `Carga`, `Distância` e `Gasto` e preenche o `Dataset`.
  - Parâmetros:
    - `caminho`: caminho do CSV
    - `dados`: estrutura que receberá os valores
  - Retorno: `bool`
- `String formatarNumero(float valor)`
  - Converte um número para texto em um formato compatível com o CSV.
  - Parâmetros:
    - `valor`: número a ser convertido
  - Retorno: `String`
- `bool gravarRegistro(const char *caminho, float carga, float distancia, float gasto)`
  - Adiciona uma nova linha com os valores de entrada e a previsão calculada.
  - Parâmetros:
    - `caminho`: caminho do CSV
    - `carga`: valor de carga em kg
    - `distancia`: valor em km
    - `gasto`: valor de gasto estimado
  - Retorno: `bool`

### Como chamar a classe
```cpp
#include <Armazenamento.h>

Armazenamento armazenamento;
```

### Exemplo de uso
```cpp
Armazenamento armazenamento;
Dataset dados;

void setup() {
    Serial.begin(115200);

    if (armazenamento.montar("/Bateria.csv")) {
        Serial.println("CSV pronto.");
    }

    if (armazenamento.lerDataset("/Bateria.csv", dados)) {
        Serial.println("Dataset carregado com sucesso.");
    }

    armazenamento.gravarRegistro("/Bateria.csv", 12.0f, 3.5f, 21.5f);
}
```

---

## Compilar

### Papel da classe
A classe `Compilar` prepara o modelo. Ela inicializa os neurônios e o otimizador Adam.

### Atributos públicos
- `Neuronios neuronios;`
  - Guarda os pesos e o viés do modelo.
- `Adam otimizador;`
  - Otimizador usado para ajustar o modelo durante o treino.

### Métodos públicos
- `void configurar(float taxaAprendizado = 0.05f, float beta1 = 0.9f, float beta2 = 0.999f, float eps = 1e-7f)`
  - Inicializa os parâmetros do modelo e do Adam.
  - Parâmetros:
    - `taxaAprendizado`: passo de ajuste
    - `beta1`: decaimento do primeiro momento
    - `beta2`: decaimento do segundo momento
    - `eps`: estabilidade numérica
  - Retorno: `void`

### Como chamar a classe
```cpp
#include <Compilar.h>

Compilar modeloCompilado;
```

### Exemplo de uso
```cpp
Compilar modelo;

void setup() {
    Serial.begin(115200);
    modelo.configurar(0.05f, 0.9f, 0.999f, 1e-7f);
    Serial.println("Modelo pronto para treino.");
}
```

---

## Dataset

### Papel da classe
A classe `Dataset` guarda os dados de treinamento em memória. Ela separa as entradas `carga`, `distancia` e a saída `gasto` em vetores.

### Atributos públicos
- `std::vector<float> carga;`
  - Guarda os valores de carga em kg.
- `std::vector<float> distancia;`
  - Guarda os valores de distância em km.
- `std::vector<float> gasto;`
  - Guarda os valores reais de gasto da bateria.

### Métodos públicos
- Nenhum método público.

### Como chamar a classe
```cpp
#include <Dataset.h>

Dataset dados;
```

### Exemplo de uso
```cpp
Dataset bateria;

bateria.carga.push_back(10.0f);
bateria.distancia.push_back(5.0f);
bateria.gasto.push_back(23.0f);

// Os dados podem ser usados depois para treinamento
```

---

## Epocas

### Papel da classe
A classe `Epocas` repete o treinamento ao longo de várias épocas, embaralha as amostras e chama o treinamento em lotes.

### Atributos públicos
- Nenhum atributo público.

### Métodos públicos
- `float executar(int epochs, Dataset &dados, Compilar &modelo, int tamanhoLote = 32)`
  - Executa o treinamento por `epochs` épocas.
  - Embaralha os índices das amostras antes de cada época.
  - Parâmetros:
    - `epochs`: número total de épocas
    - `dados`: conjunto de treinamento
    - `modelo`: modelo compilado
    - `tamanhoLote`: tamanho do lote
  - Retorno: `float` com a perda final

### Como chamar a classe
```cpp
#include <Epocas.h>

Epocas epocas;
```

### Exemplo de uso
```cpp
Dataset dados;
Compilar modelo;
Epocas epocas;

void setup() {
    Serial.begin(115200);
    modelo.configurar(0.05f);

    float perdaFinal = epocas.executar(500, dados, modelo, 32);
    Serial.printf("Perda final: %.5f\n", perdaFinal);
}
```

---

## Gradientes

### Papel da classe
A classe `Gradientes` calcula os gradientes do erro em relação aos parâmetros do modelo.

### Atributos públicos
- Nenhum atributo público.

### Métodos públicos
- `void processarAmostraLote(float carga, float distancia, float gastoReal, const float *pesos, float vies, float *gradientes, float &somaErroQuadrado, Prever &previsor)`
  - Calcula o erro da amostra e acumula gradientes para `w1`, `w2` e `b`.
  - Parâmetros:
    - `carga`: entrada de carga
    - `distancia`: entrada de distância
    - `gastoReal`: valor alvo real
    - `pesos`: vector com `w1` e `w2`
    - `vies`: viés do modelo
    - `gradientes`: vetor para acumular os gradientes
    - `somaErroQuadrado`: somatória dos erros ao quadrado
    - `previsor`: objeto usado para calcular a previsão
  - Retorno: `void`

### Como chamar a classe
```cpp
#include <Gradientes.h>

Gradientes gradientes;
```

### Exemplo de uso
```cpp
float pesos[2] = {0.5f, 4.0f};
float vies = -2.0f;
float gradientesAcumulados[3] = {0.0f, 0.0f, 0.0f};
float somaErroQuadrado = 0.0f;
Prever previsor;

Gradientes calcGradientes;
calcGradientes.processarAmostraLote(
    10.0f,
    5.0f,
    23.0f,
    pesos,
    vies,
    gradientesAcumulados,
    somaErroQuadrado,
    previsor
);
```

---

## Modelo

### Papel da classe
A classe `Modelo` é a interface principal da biblioteca. Ela coordena a leitura do CSV, treinamento, previsão, gravação e exibição dos resultados.

### Atributos públicos
- `float taxaAprendizado = 0.05f;`
  - Taxa usada pelo otimizador Adam.
- `int epocas = 500;`
  - Quantidade de épocas do treinamento.
- `int tamanhoLote = 32;`
  - Quantidade de amostras por lote.

### Métodos públicos
- `void executar()`
  - Executa o fluxo completo da biblioteca:
    - lê o dataset
    - solicita valores ao usuário
    - treina o modelo
    - calcula a previsão
    - salva uma nova linha no CSV
    - mostra resultados no Serial Monitor
  - Retorno: `void`

### Como chamar a classe
```cpp
#include <Modelo.h>

Modelo modelo;
```

### Exemplo de uso
```cpp
Modelo modeloMachineLearning;

void setup() {
    Serial.begin(115200);
    modeloMachineLearning.taxaAprendizado = 0.03f;
    modeloMachineLearning.epocas = 750;
    modeloMachineLearning.tamanhoLote = 16;
}

void loop() {
    modeloMachineLearning.executar();
    delay(500);
}
```

---

## Neuronios

### Papel da classe
A classe `Neuronios` conserva os pesos e o viés do modelo e inicializa esses parâmetros.

### Atributos públicos
- `float pesos[2];`
  - Guarda os pesos `w1` e `w2`.
- `float vies;`
  - Guarda o valor do viés `b`.

### Métodos públicos
- `float gerarNumeroAleatorio()`
  - Gera um número aleatório em um intervalo apropriado para inicialização.
  - Retorno: `float`
- `void inicializar()`
  - Inicializa os pesos e o viés do modelo.
  - Retorno: `void`

### Como chamar a classe
```cpp
#include <Neuronios.h>

Neuronios neuronios;
```

### Exemplo de uso
```cpp
Neuronios neuronios;

void setup() {
    Serial.begin(115200);
    neuronios.inicializar();

    Serial.printf("w1 = %.5f, w2 = %.5f, b = %.5f\n",
                  neuronios.pesos[0],
                  neuronios.pesos[1],
                  neuronios.vies);
}
```

---

## Perdas

### Papel da classe
A classe `Perdas` calcula a perda do modelo, usando a métrica MSE (Mean Squared Error).

### Atributos públicos
- Nenhum atributo público.

### Métodos públicos
- `float calcularMSE(float somaErrosQuadrados, int totalAmostras)`
  - Calcula o MSE dividindo a soma dos erros quadrados pelo número total de amostras.
  - Parâmetros:
    - `somaErrosQuadrados`: soma dos erros ao quadrado
    - `totalAmostras`: quantidade total de amostras
  - Retorno: `float`

### Como chamar a classe
```cpp
#include <Perdas.h>

Perdas perdas;
```

### Exemplo de uso
```cpp
Perdas perdas;

void setup() {
    Serial.begin(115200);
    float mse = perdas.calcularMSE(25.0f, 10);
    Serial.printf("MSE = %.5f\n", mse);
}
```

---

## Prever

### Papel da classe
A classe `Prever` calcula a previsão do modelo para uma entrada composta por carga e distância.

### Atributos públicos
- Nenhum atributo público.

### Métodos públicos
- `float calcular(float carga, float distancia, const float *pesos, float vies)`
  - Calcula a saída do modelo:
    ```cpp
    return pesos[0] * carga + pesos[1] * distancia + vies;
    ```
  - Parâmetros:
    - `carga`: entrada de carga em kg
    - `distancia`: entrada de distância em km
    - `pesos`: vetor com os pesos `w1` e `w2`
    - `vies`: valor do viés 
  - Retorno: `float`

### Como chamar a classe
```cpp
#include <Prever.h>

Prever previsor;
```

### Exemplo de uso
```cpp
float pesos[2] = {0.5f, 4.0f};
float vies = -2.0f;
Prever previsor;

void setup() {
    Serial.begin(115200);
    float gastoPrevisto = previsor.calcular(10.0f, 5.0f, pesos, vies);
    Serial.printf("Gasto previsto: %.2f%%\n", gastoPrevisto);
}
```

---

## Treinar

### Papel da classe
A classe `Treinar` processa os lotes do conjunto de treinamento e aplica os ajustes do modelo.

### Atributos públicos
- Nenhum atributo público.

### Métodos públicos
- `float executarLotes(Dataset &dados, const std::vector<int> &indices, int tamanho_lote, Compilar &modelo, Gradientes &gradientesCalc, Perdas &perdasCalc, Prever &previsor)`
  - Percorre os lotes do dataset, calcula os gradientes e atualiza os parâmetros do modelo.
  - Parâmetros:
    - `dados`: dataset de treino
    - `indices`: vetor com índices das amostras embaralhadas
    - `tamanho_lote`: número de amostras por lote
    - `modelo`: objeto compilado
    - `gradientesCalc`: instância de `Gradientes`
    - `perdasCalc`: instância de `Perdas`
    - `previsor`: instância de `Prever`
  - Retorno: `float` com a soma da perda por época

### Como chamar a classe
```cpp
#include <Treinar.h>

Treinar treinador;
```

### Exemplo de uso
```cpp
Dataset dados;
Compilar modelo;
Gradientes gradientes;
Perdas perdas;
Prever previsor;
Treinar treinador;

void setup() {
    Serial.begin(115200);
    std::vector<int> indices = {0, 1, 2, 3, 4};

    float perdaEpoca = treinador.executarLotes(
        dados,
        indices,
        32,
        modelo,
        gradientes,
        perdas,
        previsor
    );

    Serial.printf("Perda da época: %.5f\n", perdaEpoca);
}
```

---

## Verbose

### Papel da classe
A classe `Verbose` cuida da interação com o usuário via Serial Monitor, le os valores digitados e imprime os resultados finais do treinamento.

### Atributos públicos
- Nenhum atributo público.

### Métodos públicos
- `float lerNumero(const char *mensagem)`
  - Exibe uma mensagem no Serial Monitor e espera um valor em ponto flutuante.
  - Parâmetros:
    - `mensagem`: texto exibido antes da leitura
  - Retorno: `float`
- `void mostrarResultados(float carga, float distancia, float perdas, const float *pesos, float vies, float gasto_previsto)`
  - Mostra os valores de entrada, pesos, viés, perda final e previsão calculada.
  - Parâmetros:
    - `carga`: valor de carga informado pelo usuário
    - `distancia`: valor de distância informado
    - `perdas`: MSE final
    - `pesos`: vetor com `w1` e `w2`
    - `vies`: valor do viés
    - `gasto_previsto`: saída estimada pelo modelo
  - Retorno: `void`

### Como chamar a classe
```cpp
#include <Verbose.h>

Verbose verbose;
```

### Exemplo de uso
```cpp
Verbose verbose;

void setup() {
    Serial.begin(115200);
    float carga = verbose.lerNumero("Digite a carga (kg): ");
    float distancia = verbose.lerNumero("Digite a distância (km): ");

    Serial.printf("Carga lida: %.2f\n", carga);
    Serial.printf("Distancia lida: %.2f\n", distancia);
}
```

---

## Resumo do fluxo da biblioteca

O relacionamento entre as classes pode ser resumido assim:

```text
Modelo -> Compilar -> Neuronios + Adam
       -> Aprendizagem -> Epocas -> Treinar
                            -> Gradientes + Perdas + Prever
       -> Armazenamento -> Dataset
       -> Verbose -> Serial Monitor
```

Esse fluxo permite que a biblioteca leia os dados, ajuste os pesos e o viés no próprio ESP32 e use o modelo para prever a quantidade de bateria gasta em uma viagem.
