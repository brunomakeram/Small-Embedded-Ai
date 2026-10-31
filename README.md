# Maker

Biblioteca em C++ para treinar e usar modelos de Machine Learning direto no microcontrolador (ESP32), sem converter modelo de outro lugar. O treino acontece no próprio chip, com dados guardados na memória flash (LittleFS).

## Open Source e contribuições

Este projeto é open source e aceita contribuições da comunidade. Se você encontrar bugs, tiver sugestões de melhorias, quiser adicionar exemplos ou ajudar a documentar o projeto, fique à vontade para abrir uma issue ou enviar um pull request.

A ideia é manter a biblioteca acessível, útil para aprendizado e aberta para evolução colaborativa.

Cada módulo faz o mesmo papel que o equivalente em Python (Keras/TensorFlow), só traduzido para C++, com nomes em português.

O primeiro projeto que usa a biblioteca é a previsão de gasto de bateria: dados a carga (kg) e a distância (km), o modelo prevê quantos % de bateria a viagem gasta.

## Instalação

1. Baixe o `.zip` da biblioteca.
2. Na Arduino IDE: Sketch > Incluir Biblioteca > Adicionar biblioteca .ZIP.
3. Selecione a placa ESP32 (core ESP32 Arduino v2.x ou superior, que traz o LittleFS).
4. No código: `#include <Modelo.h>`

O exemplo completo está em Arquivo > Exemplos > Maker > Maker.

## Como funciona a conta

O modelo é uma regressão linear múltipla:

```
gasto = (w1 * carga) + (w2 * distancia) + b
```

- `w1`, `w2`: pesos. Dizem o quanto cada entrada pesa no resultado.
- `b`: viés. Ajuste de base da conta.

O treino procura os valores de `w1`, `w2` e `b` que deixam o erro (MSE) o menor possível.

Exemplo com pesos já treinados `w1 = 0.5`, `w2 = 4.0`, `b = -2.0`, para 10 kg e 5 km:

```
0.5 * 10  =  5.0
4.0 * 5   = 20.0
5.0 + 20.0 - 2.0 = 23.0   → o modelo diz que a viagem gasta 23% da bateria
```

## Mapa dos módulos

| Módulo | Equivalente no Keras | Papel |
|---|---|---|
| Modelo | `Model` | Junta tudo e roda o fluxo completo |
| Compilar | `model.compile()` | Prepara a rede e o otimizador |
| Aprendizagem | `model.fit()` (chamada) | Ponto de entrada do treino |
| Epocas | `epochs` | Laço das épocas |
| Treinar | lotes do `fit` | Percorre os lotes dentro de uma época |
| Gradientes | `tf.GradientTape` | Derivadas do erro |
| Perdas | `keras.losses` | Cálculo do erro (MSE) |
| Adam | `keras.optimizers.Adam` | Atualiza pesos e viés |
| Neuronios | `layers.Dense` | Pesos, viés e inicialização |
| Prever | `model.predict()` | Calcula a previsão |
| Dataset | `DataFrame` | Guarda os dados na RAM |
| Armazenamento | leitura de CSV | Lê e grava o CSV na flash |
| Verbose | `verbose` | Lê números e mostra resultados no Serial |

Fluxo de um treino:

```
Modelo.executar()
  Armazenamento.lerDataset()        flash → Dataset (RAM)
  Compilar.configurar()             Neuronios.inicializar() + Adam.inicializar()
  Aprendizagem.treinarModelo()
    Epocas.executar()               repete N épocas
      embaralha os índices
      Treinar.executarLotes()       lotes de 32 amostras
        Gradientes.processarAmostraLote()   (usa Prever)
        Perdas.calcularMSE()
        Adam.atualizarParametros()
  Prever.calcular()                 previsão final
  Armazenamento.gravarRegistro()    salva a nova linha no CSV
```

---

## Modelo

Classe que faz tudo de uma vez. Ideal para uso rápido.

```cpp
#include <Modelo.h>

Modelo modelo;

void loop() {
    modelo.executar();
}
```

`executar()` não recebe parâmetros. Os ajustes do treino são atributos que você muda antes de chamar:

| Atributo | O que controla | Padrão |
|---|---|---|
| `taxaAprendizado` | tamanho do passo do Adam | `0.05` |
| `epocas` | quantas passadas pelo dataset | `500` |
| `tamanhoLote` | amostras por lote | `32` |

```cpp
Modelo modelo;

void setup() {
    modelo.taxaAprendizado = 0.01f;
    modelo.epocas = 300;
}
```

O que `executar()` faz:

1. Lê o CSV `/Bateria.csv`.
2. Pede carga e distância pelo Serial.
3. Valida os limites (carga de 1 a 100 kg, distância de 1 a 9 km). Fora disso, não treina e não salva.
4. Treina com os valores dos atributos acima.
5. Prevê o gasto. Se der fora de 0% a 100%, descarta o dado.
6. Se estiver certo, grava a nova linha no CSV e mostra o relatório.

Os limites e o caminho do CSV estão dentro do `Modelo.h`. Para outro projeto, é ali que se muda.

---

## Compilar

Prepara o modelo para treinar: escolhe a rede e o otimizador.

Atributos:
- `neuronios` (classe `Neuronios`): pesos e viés.
- `otimizador` (classe `Adam`): otimizador.

Método:
- `void configurar(float taxaAprendizado = 0.05f, float beta1 = 0.9f, float beta2 = 0.999f, float eps = 1e-7f)`: inicializa `neuronios` e o `otimizador`. Todos os parâmetros são opcionais.
  - `taxaAprendizado`: tamanho do passo. Menor aprende mais devagar e estável, maior aprende mais rápido mas pode oscilar.
  - `beta1`, `beta2`, `eps`: parâmetros do Adam. Na maioria dos casos não precisa mexer.

```cpp
Compilar modelo;
modelo.configurar();        // taxa 0.05
modelo.configurar(0.01f);   // taxa 0.01
// depois do treino: modelo.neuronios.pesos[0], modelo.neuronios.vies
```

---

## Aprendizagem

Ponto de entrada do treino. Chama o módulo `Epocas`.

- `float treinarModelo(Dataset &dados, Compilar &modelo, int totalEpocas, int tamanhoLote = 32)`
  - `dados`: dataset já lido da flash.
  - `modelo`: objeto `Compilar` já configurado.
  - `totalEpocas`: quantas passadas completas pelo dataset (ex.: `500`).
  - `tamanhoLote`: amostras por lote (opcional, padrão `32`).
  - Retorna: o MSE final.

```cpp
Aprendizagem aprendizagem;
float perdas = aprendizagem.treinarModelo(bateria, modeloCompilado, 500);
float perdas2 = aprendizagem.treinarModelo(bateria, modeloCompilado, 500, 16);
```

---

## Epocas

Controla o laço das épocas.

- `float executar(int epochs, Dataset &dados, Compilar &modelo, int tamanhoLote = 32)`
  - A cada época, embaralha a ordem das amostras (Fisher-Yates, usando `esp_random()`) e chama `Treinar.executarLotes()`.
  - `tamanhoLote`: amostras por lote. Valor menor que 1 vira 1.
  - Retorna: o MSE da última época.

Exemplo: o dataset padrão tem 179 amostras. Com lote 32, cada época tem 6 lotes (5 de 32 e 1 de 19). Com lote 16, são 12 lotes.

Normalmente você não chama `Epocas` direto, usa a `Aprendizagem`.

---

## Treinar

Percorre os lotes de uma época. Uso interno.

- `float executarLotes(Dataset &dados, const std::vector<int> &indices, int tamanho_lote, Compilar &modelo, Gradientes &gradientesCalc, Perdas &perdasCalc, Prever &previsor)`
  - `indices`: ordem embaralhada das amostras.
  - `tamanho_lote`: quantas amostras por lote (32).
  - Para cada lote: soma os gradientes das amostras, calcula o MSE do lote e chama o `Adam` para atualizar os pesos.
  - Retorna: a soma da perda da época (já ponderada pelo tamanho de cada lote). Quem divide pelo total de amostras é a `Epocas`.

---

## Gradientes

Calcula as derivadas do erro para uma amostra e acumula nos vetores.

- `void processarAmostraLote(float carga, float distancia, float gastoReal, const float *pesos, float vies, float *gradientes, float &somaErroQuadrado, Prever &previsor)`
  - `gradientes`: vetor de 3 posições `[dw1, dw2, db]`, acumulado a cada amostra.
  - `somaErroQuadrado`: acumula `erro²` do lote.

As contas:

```
erro = previsto - real
dw1 += 2 * erro * carga
dw2 += 2 * erro * distancia
db  += 2 * erro
```

Exemplo: carga 10, distância 5, gasto real 22, com `w1 = 0.5`, `w2 = 4.0`, `b = -2.0`:

```
previsto = 23.0
erro     = 23.0 - 22.0 = 1.0
dw1 = 2 * 1.0 * 10 = 20
dw2 = 2 * 1.0 * 5  = 10
db  = 2 * 1.0      = 2
erro²    = 1.0
```

---

## Perdas

Funções de perda (o erro do modelo).

- `float calcularMSE(float somaErrosQuadrados, int totalAmostras)`
  - Retorna a média dos erros ao quadrado. Se `totalAmostras <= 0`, retorna `0`.

Exemplo: soma dos erros² = 12.5 em 5 amostras → MSE = 2.5.

```cpp
Perdas perdas;
float mse = perdas.calcularMSE(12.5f, 5);   // 2.5
```

---

## Adam

Otimizador. Ajusta `w1`, `w2` e `b` usando a média dos gradientes (1º momento) e a média dos gradientes ao quadrado (2º momento).

Atributos:
- `taxaAprendizado`: tamanho do passo (lr).
- `decaimentoPrimeiroMomento`: beta1.
- `decaimentoSegundoMomento`: beta2.
- `margemEstabilidadeNumerica`: eps, evita divisão por zero.
- `numeroAtualizacoes`: quantas vezes já atualizou (o `t` da fórmula).
- `mediaMovelGradientes[3]` e `mediaMovelGradientesQuadrados[3]`: memória de cada parâmetro (`w1`, `w2`, `b`).

Métodos:
- `void inicializar(float lr = 0.05f, float beta1 = 0.9f, float beta2 = 0.999f, float eps = 1e-7f)`: define os parâmetros e zera a memória.
- `void atualizarParametros(float *pesos, float &vies, float *gradientes, int numeroAmostrasLote)`
  - Divide os gradientes pelo tamanho do lote (média), aplica a correção de viés dos momentos e atualiza `pesos[0]`, `pesos[1]` e `vies`.

Exemplo numérico, primeira atualização (`t = 1`, `lr = 0.05`): o passo efetivo de cada parâmetro é de aproximadamente 0.05, na direção oposta ao gradiente. Nas atualizações seguintes o passo se ajusta conforme o histórico dos gradientes.

---

## Neuronios

Guarda os parâmetros da rede (equivalente a uma camada `Dense` com 2 entradas e 1 saída).

Atributos:
- `float pesos[2]`: `[w1, w2]`.
- `float vies`: `b`.

Métodos:
- `void inicializar()`: sorteia os pesos pelo método Glorot/Xavier e zera o viés.
- `float gerarNumeroAleatorio()`: devolve um número entre 0 e 1 usando o gerador de hardware do ESP32 (`esp_random()`).

Exemplo: com 2 entradas e 1 saída, o limite Glorot é `sqrt(6 / 3) ≈ 1.414`. Cada peso começa sorteado entre `-1.414` e `+1.414`.

---

## Prever

Faz a inferência: aplica os parâmetros já treinados em uma entrada.

- `float calcular(float carga, float distancia, const float *pesos, float vies)`
  - Retorna: `pesos[0] * carga + pesos[1] * distancia + vies`.

```cpp
Prever previsor;
float gasto = previsor.calcular(
    10.0f,                          // carga (kg)
    5.0f,                           // distância (km)
    modeloCompilado.neuronios.pesos,
    modeloCompilado.neuronios.vies
);
// com w1=0.5, w2=4.0, b=-2.0 → 23.0
```

---

## Dataset

Guarda os dados na RAM, depois de lidos da flash.

Atributos (vetores com uma posição por amostra):
- `std::vector<float> carga`: kg.
- `std::vector<float> distancia`: km.
- `std::vector<float> gasto`: % de bateria gasta (o valor real, que o modelo tenta acertar).

Também define `DATASET_PADRAO`: o CSV inicial (179 linhas, com cabeçalho `Carga,Distância,Gasto`) guardado em `PROGMEM`. Ele é gravado na flash só na primeira execução, quando o arquivo ainda não existe.

---

## Armazenamento

Lê e grava o CSV na memória flash (LittleFS). Todos os métodos recebem o caminho do arquivo, por exemplo `"/Bateria.csv"`.

- `bool montar(const char *caminho)`
  - Monta o LittleFS e, se o arquivo não existir, cria com o `DATASET_PADRAO`. Chame uma vez no `setup()`.
- `bool lerDataset(const char *caminho, Dataset &dados)`
  - Lê o CSV para o `Dataset`. Procura as colunas pelo nome no cabeçalho (`Carga`, `Distância`, `Gasto`), então a ordem delas não importa. Retorna `false` se o arquivo não abrir, se faltar coluna ou se não tiver nenhuma linha de dados.
- `bool gravarRegistro(const char *caminho, float carga, float distancia, float gasto)`
  - Acrescenta uma linha no fim do arquivo, sem apagar as anteriores.
- `String formatarNumero(float valor)`
  - Converte o número em texto para o CSV (ex.: `5` vira `"5.0"`).

```cpp
Armazenamento armazenamento;
Dataset bateria;

armazenamento.montar("/Bateria.csv");
armazenamento.lerDataset("/Bateria.csv", bateria);
armazenamento.gravarRegistro("/Bateria.csv", 49.0f, 5.0f, 44.4f);
```

---

## Verbose

Conversa com o usuário pelo Serial Monitor (115200 baud).

- `float lerNumero(const char *mensagem)`
  - Mostra a mensagem, espera o usuário digitar e devolve o valor como `float`.
- `void mostrarResultados(float carga, float distancia, float perdas, const float *pesos, float vies, float gasto_previsto)`
  - Imprime o relatório final do treino.

Saída de exemplo:

```
Digite a carga (kg): 49
Digite a distância (km): 5

===== RESULTADOS DO TREINAMENTO =====
Carga inputada: 49.00 kg
Distância inputada: 5.00 km
Pesos Ajustados (w1, w2): [0.51986, 4.77821]
Viés Ajustado (b): -4.96010
MSE (Perda Final): 0.00023
-> Previsão de Gasto de Bateria: 44.40%
=====================================
```

---

## Estrutura da biblioteca

```
Maker/
  library.properties
  README.md
  src/          os módulos (.h)
  examples/
    Maker/
      Maker.ino exemplo de uso
```

As pastas `src` e `examples` são nomes fixos do formato de biblioteca do Arduino.
