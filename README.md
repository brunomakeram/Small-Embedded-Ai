# Maker

Biblioteca C++ para treinar e executar redes neurais em microcontroladores
compatíveis com Arduino.

## Uso

O modelo recebe as amostras de treinamento como matrizes: uma linha por amostra
e uma coluna por característica ou saída. O terceiro parâmetro é a entrada usada
para a previsão depois do treinamento.

```cpp
#include <Seai.h>

Modelo::Modelo modelo;

const std::vector<std::vector<float>> entradas = {
    {0.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 0.0f},
    {1.0f, 1.0f}
};

const std::vector<std::vector<float>> saidas = {
    {0.0f},
    {1.0f},
    {1.0f},
    {0.0f}
};

const std::vector<float> entradaPrevisao = {1.0f, 0.0f};

void setup() {
    Serial.begin(115200);
    modelo.executar(entradas, saidas, entradaPrevisao);
}
```

`executar` retorna `false` e informa o erro pelo Serial quando as dimensões ou os
parâmetros não são válidos. O método de resultados exibe somente a perda final.

## Logs do treino (Verbose)

As opções de log podem ser configuradas separadamente no modelo. Todas começam
desativadas:

```cpp
modelo.verbose.mostrarConfiguracao = true;
modelo.verbose.mostrarNumeroEpoca = true;
modelo.verbose.mostrarPerdaPorEpoca = false;
modelo.executar(entradas, saidas, entradaPrevisao);
```

`mostrarConfiguracao` registra os parâmetros configurados, `mostrarNumeroEpoca`
registra o número de cada época e `mostrarPerdaPorEpoca` registra sua perda.
Esses logs não incluem entradas, saídas, pesos, vieses ou previsões; o método de
resultados continua exibindo apenas a perda final.

## Configuração da rede

- `taxaAprendizado`, `epocas` e `tamanhoLote` ajustam o treinamento.
- `neuroniosPorCamada` configura as camadas ocultas; seu padrão é `{8}`.
- Cada camada aceita de 1 a 64 neurônios. A camada de saída é dimensionada
  automaticamente conforme o número de saídas dos dados de treinamento.
- A classe `Camadas::Camadas` organiza instâncias de `Neuronios::Neuronios`; cada
  instância armazena os pesos e vieses de uma camada.

As demais classes também ficam em namespaces com o nome da classe. Por exemplo:

```cpp
#include <Armazenamento/Armazenamento.h>
#include <Dataset/Dataset.h>

Dataset::Dataset dados;
Armazenamento::Armazenamento armazenamento;
```

Esse padrão também se aplica a `Aprendizagem::Aprendizagem`,
`Compilar::Compilar`, `Epocas::Epocas`, `Gradientes::Gradientes`,
`Perdas::Perdas`, `Prever::Prever` e `Treinar::Treinar`. `Adam::Adam`,
`Camadas::Camadas`, `Modelo::Modelo` e `Verbose::Verbose` já seguem esse padrão.

## Exemplos por arquivo e classe

Os exemplos abaixo mostram como incluir cada header e chamar os principais
métodos ou acessar os dados oferecidos por sua classe. Os componentes de treino
de baixo nível são apresentados para integração personalizada; para o uso
habitual, prefira a interface `Modelo::Modelo` descrita acima.
Cada bloco é um fragmento independente: coloque instruções executáveis dentro
de `setup()`, `loop()` ou de outra função do seu sketch. O exemplo de `Gravador`
já mostra a estrutura completa com `setup()` e `loop()`.

### `src/Adam/Adam.h` — `Adam::Adam`

Inicialize o otimizador e aplique uma atualização aos dois pesos e ao viés:

```cpp
#include <Adam/Adam.h>

Adam::Adam adam;
float pesos[2] = {0.1f, 0.2f};
float vies = 0.0f;
float gradientes[3] = {0.3f, -0.1f, 0.2f};

adam.inicializar(0.05f);
adam.atualizarParametros(pesos, vies, gradientes, 1);
```

### `src/Aprendizagem/Aprendizagem.h` — `Aprendizagem::Aprendizagem`

Passe um dataset e um modelo configurado para treinar por várias épocas:

```cpp
#include <Aprendizagem/Aprendizagem.h>
#include <Compilar/Compilar.h>
#include <Dataset/Dataset.h>

Dataset::Dataset dados;
dados.carga.push_back(1.0f);
dados.distancia.push_back(2.0f);
dados.gasto.push_back(3.0f);

Compilar::Compilar modelo;
modelo.neuronios.configurar(2, 1);
modelo.configurar();

Aprendizagem::Aprendizagem aprendizagem;
float perda = aprendizagem.treinarModelo(dados, modelo, 10, 1);
```

### `src/Armazenamento/Armazenamento.h` — `Armazenamento::Armazenamento`

Monte o LittleFS, leia o CSV para um dataset e acrescente um registro:

```cpp
#include <Armazenamento/Armazenamento.h>
#include <Dataset/Dataset.h>

Armazenamento::Armazenamento armazenamento;
Dataset::Dataset dados;
const char *caminho = "/dados.csv";

if (armazenamento.montar(caminho)) {
    armazenamento.lerDataset(caminho, dados);
    armazenamento.gravarRegistro(caminho, 5.0f, 2.0f, 7.0f);
}
```

### `src/Camadas/Camadas.h` — `Camadas::Camadas`

Adicione camadas e acesse uma delas pela ordem de inserção:

```cpp
#include <Camadas/Camadas.h>

Camadas::Camadas camadas;
camadas.adicionar(2, 8);
camadas.adicionar(8, 1);

camadas[0].inicializar();
std::size_t quantidade = camadas.tamanho();
```

### `src/Compilar/Compilar.h` — `Compilar::Compilar`

Configure os neurônios e o otimizador agrupados pelo componente:

```cpp
#include <Compilar/Compilar.h>

Compilar::Compilar modelo;
modelo.neuronios.configurar(2, 1);
modelo.configurar(0.01f);
```

### `src/Conversor/Conversor.h` — `Conversor::Conversor`

Converta um valor `float` armazenado em bytes para um inteiro de 16 bits:

```cpp
#include <Conversor/Conversor.h>
#include <cstring>

Conversor::Conversor conversor(Conversor::Conversor::FLOAT32);
float valor = 23.6f;
uint8_t entrada[sizeof(valor)];
uint8_t saida[2];
std::memcpy(entrada, &valor, sizeof(valor));

bool convertido = conversor.converterDado(
    Conversor::Conversor::INT16, entrada, saida, 1
);
```

Para textos, informe a largura de cada elemento ao chamar `converterDado`;
`larguraMinima` pode calcular a largura necessária para uma lista de palavras.

### `src/Dataset/Dataset.h` — `Dataset::Dataset`

Os três vetores guardam valores relacionados pela mesma posição:

```cpp
#include <Dataset/Dataset.h>

Dataset::Dataset dados;
dados.carga.push_back(5.0f);
dados.distancia.push_back(2.0f);
dados.gasto.push_back(7.0f);
```

### `src/Epocas/Epocas.h` — `Epocas::Epocas`

Execute o treinamento de um modelo compilado por uma quantidade de épocas:

```cpp
#include <Compilar/Compilar.h>
#include <Dataset/Dataset.h>
#include <Epocas/Epocas.h>

Dataset::Dataset dados;
dados.carga.push_back(1.0f);
dados.distancia.push_back(2.0f);
dados.gasto.push_back(3.0f);

Compilar::Compilar modelo;
modelo.neuronios.configurar(2, 1);
modelo.configurar();

Epocas::Epocas treinamento;
float perda = treinamento.executar(10, dados, modelo, 1);
```

### `src/Gradientes/Gradientes.h` — `Gradientes::Gradientes`

Acumule gradientes para uma amostra usando os parâmetros atuais:

```cpp
#include <Gradientes/Gradientes.h>
#include <Prever/Prever.h>

Gradientes::Gradientes calculador;
Prever::Prever previsor;
float pesos[2] = {0.1f, 0.2f};
float gradientes[3] = {0.0f, 0.0f, 0.0f};
float somaErroQuadrado = 0.0f;

calculador.processarAmostraLote(
    1.0f, 2.0f, 3.0f, pesos, 0.0f, gradientes, somaErroQuadrado, previsor
);
```

### `src/Gravador/Gravador.h` — `Gravador::Gravador`

No ESP32 compatível com o driver I2S e o LED RGB usados pela biblioteca,
inicialize o gravador e chame `gravar()` no laço:

```cpp
#include <Gravador/Gravador.h>

Gravador::Gravador gravador(2, 4, 5, 9);

void setup() {
    gravador.iniciar();
}

void loop() {
    gravador.gravar();
}
```

O construtor recebe duração em segundos e os pinos BCLK, WS e DIN; a duração é
limitada a 10 segundos.

### `src/Modelo/Modelo.h` — `Modelo::Modelo`

Configure camadas ocultas, treine com matrizes e informe uma entrada para
previsão:

```cpp
#include <Seai.h>

Modelo::Modelo modelo;
modelo.neuroniosPorCamada = {8};

std::vector<std::vector<float>> entradas = {{0.0f, 1.0f}, {1.0f, 0.0f}};
std::vector<std::vector<float>> alvos = {{1.0f}, {1.0f}};
std::vector<float> entradaPrevisao = {1.0f, 1.0f};

bool executado = modelo.executar(entradas, alvos, entradaPrevisao);
```

### `src/Neuronios/Neuronios.h` — `Neuronios::Neuronios`

Configure, inicialize e calcule as ativações de uma camada:

```cpp
#include <Neuronios/Neuronios.h>
#include <vector>

Neuronios::Neuronios camada;
camada.configurar(2, 8);
camada.inicializar();

std::vector<float> entradas = {0.5f, 0.25f};
std::vector<float> saidas;
camada.calcular(entradas, saidas, true);
```

O terceiro argumento de `calcular` ativa `tanh` para camadas ocultas; use
`false` para uma camada de saída linear.

### `src/Perdas/Perdas.h` — `Perdas::Perdas`

Calcule a perda média quadrática a partir da soma dos erros:

```cpp
#include <Perdas/Perdas.h>

Perdas::Perdas perdas;
float mse = perdas.calcularMSE(2.0f, 4);
```

### `src/Prever/Prever.h` — `Prever::Prever`

Calcule uma previsão linear para carga e distância:

```cpp
#include <Prever/Prever.h>

Prever::Prever previsor;
float pesos[2] = {0.5f, 1.25f};
float resultado = previsor.calcular(2.0f, 3.0f, pesos, 0.1f);
```

### `src/Treinar/Treinar.h` — `Treinar::Treinar`

Para controlar o treinamento por lotes diretamente, prepare o dataset, o
modelo e os componentes usados no cálculo:

```cpp
#include <Compilar/Compilar.h>
#include <Dataset/Dataset.h>
#include <Gradientes/Gradientes.h>
#include <Perdas/Perdas.h>
#include <Prever/Prever.h>
#include <Treinar/Treinar.h>
#include <vector>

Dataset::Dataset dados;
dados.carga.push_back(1.0f);
dados.distancia.push_back(2.0f);
dados.gasto.push_back(3.0f);

Compilar::Compilar modelo;
modelo.neuronios.configurar(2, 1);
modelo.configurar();

std::vector<int> indices = {0};
Gradientes::Gradientes gradientes;
Perdas::Perdas perdas;
Prever::Prever previsor;
Treinar::Treinar treinador;

float perdaPonderada = treinador.executarLotes(
    dados, indices, 1, modelo, gradientes, perdas, previsor
);
```

### `src/Verbose/Verbose.h` — `Verbose::Verbose`

Ative separadamente as categorias de log e encaminhe a configuração e os
eventos do treinamento:

```cpp
#include <Verbose/Verbose.h>
#include <vector>

Verbose::Verbose verbose;
verbose.mostrarConfiguracao = true;
verbose.mostrarNumeroEpoca = true;
verbose.mostrarPerdaPorEpoca = true;

std::vector<int> camadasOcultas = {8};
verbose.registrarConfiguracao(0.05f, 10, 4, camadasOcultas, 1);
verbose.registrarEpoca(1);
verbose.registrarPerdaEpoca(0.25f);
```
