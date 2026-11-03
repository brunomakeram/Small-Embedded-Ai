# Small Embedded AI

Biblioteca C++ para treinamento e inferência de modelos em microcontroladores compatíveis com Arduino. O projeto contém uma API de rede neural multicamada, utilitários de dados e conversão, persistência LittleFS e captura de áudio I2S.

**Site da biblioteca:** https://small-embedded-ai.netlify.app/

## Requisitos e instalação

Instale a biblioteca pelo Arduino IDE ou disponibilize a pasta `src` no caminho de bibliotecas do Arduino. Os exemplos de gravador descritos mais abaixo são direcionados a ESP32-S3, microfone INMP441 e pacote de placas ESP32 da Espressif. As classes de rede e de captura têm dependências de hardware distintas; confira os includes e a compatibilidade da placa para cada exemplo.

## Uso rápido: treino e previsão

As linhas das matrizes `entradas` e `saidasEsperadas` formam amostras correspondentes. A entrada de previsão tem a mesma quantidade de características de cada linha de treino. Este sketch ilustra o ponto de entrada recomendado, `Modelo::Modelo::executar`:

```cpp
#include <Seai.h> // Importa a API central da biblioteca e suas classes públicas.
#include <vector> // Declara os vetores dinâmicos usados nas matrizes de amostras.

Modelo::Modelo modelo; // Cria o modelo que treinará, validará e calculará a previsão.

void setup() { // Executa o fluxo uma vez ao iniciar a placa.
    Serial.begin(115200); // Abre a serial para mensagens de validação e resultado.

    const std::vector<std::vector<float>> entradas = { // Cada linha é uma amostra e cada coluna uma característica.
        {0.0f, 0.0f}, // Amostra de referência: característica 1 e característica 2.
        {0.0f, 1.0f}, // Amostra com a segunda característica ativa.
        {1.0f, 0.0f}, // Amostra com a primeira característica ativa.
        {1.0f, 1.0f}  // Amostra com ambas as características.
    };
    const std::vector<std::vector<float>> saidasEsperadas = { // Uma saída alvo corresponde a cada linha de entrada.
        {0.0f}, {1.0f}, {1.0f}, {0.0f} // Valores de exemplo para demonstrar o formato, não uma aplicação de regressão real.
    };
    const std::vector<float> entradaPrevisao = {1.0f, 0.0f}; // Consulta após o treino, alinhada às duas características.

    modelo.epocas = 500; // Define o número de passagens completas pelos dados.
    modelo.tamanhoLote = 2; // Define quantas amostras contribuem para cada atualização.
    modelo.neuroniosPorCamada = {8}; // Configura uma camada oculta com oito neurônios.
    bool executado = modelo.executar(entradas, saidasEsperadas, entradaPrevisao); // Valida, treina e prevê.
    if (!executado) { // Distingue entrada inválida de uma execução concluída.
        Serial.println("A execução foi recusada; confira os dados e os parâmetros informados."); // Mostra a ação corretiva.
    }
}

void loop() { // Não há trabalho periódico neste exemplo.
}
```

`executar` retorna `false` e descreve o primeiro erro no Serial quando as dimensões, valores ou parâmetros são inválidos. Configure logs por categoria antes da chamada:

```cpp
modelo.verbose.mostrarConfiguracao = true; // Solicita a impressão da arquitetura e dos hiperparâmetros.
modelo.verbose.mostrarNumeroEpoca = true; // Solicita a impressão dos números das épocas.
modelo.verbose.mostrarPerdaPorEpoca = false; // Evita imprimir a perda em cada passagem, reduzindo tráfego serial.
```

## Classes e métodos públicos

Todos os componentes abaixo estão no namespace que corresponde ao nome da classe, salvo `Dataset`, que também usa `Dataset::Dataset`. Os exemplos são sketches Arduino completos ou trechos que devem ser inseridos na função indicada; para reduzir duplicação, os blocos de cada classe demonstram chamadas a todos os seus métodos públicos relevantes. Os membros públicos de dados também são documentados.

### `Adam::Adam` — otimizador Adam

**Métodos públicos**

- `void inicializar(float lr = 0.05f, float beta1 = 0.9f, float beta2 = 0.999f, float eps = 1e-7f)`: inicia hiperparâmetros, contador e momentos. `lr` define a taxa de aprendizado; `beta1` e `beta2`, os decaimentos dos momentos; `eps`, a estabilidade numérica. Não retorna valor. Use antes da primeira atualização ou ao reiniciar o estado do otimizador.
- `void atualizarParametros(float *pesos, float &vies, float *gradientes, int numeroAmostrasLote)`: normaliza e aplica os gradientes do lote aos dois pesos e ao viés. `pesos` aponta para dois coeficientes, `vies` é o viés por referência, `gradientes` contém gradientes na ordem peso 0, peso 1, viés, e `numeroAmostrasLote` é a quantidade de amostras. Não retorna valor; lote menor ou igual a zero não altera os parâmetros. Use após acumular gradientes.

Membros públicos: `taxaAprendizado`, `decaimentoPrimeiroMomento`, `decaimentoSegundoMomento`, `margemEstabilidadeNumerica`, `numeroAtualizacoes`, `mediaMovelGradientes[3]` e `mediaMovelGradientesQuadrados[3]` expõem o estado do otimizador; em geral, deixe a classe mantê-los.

```cpp
#include <Adam/Adam.h> // Declara Adam::Adam e seus parâmetros.
Adam::Adam otimizador; // Cria o estado persistente do otimizador.
float pesos[2] = {0.1f, 0.2f}; // Coeficientes correspondentes às duas características do modelo.
float vies = 0.0f; // Intercepto inicial do modelo, definido como zero neste exemplo.
float gradientes[3] = {0.3f, -0.1f, 0.2f}; // Gradientes dos dois pesos e do viés acumulados para um exemplo.
void setup() { // Configura e atualiza uma vez para demonstrar a interface.
    Serial.begin(115200); // Inicializa a serial para diagnóstico opcional.
    otimizador.inicializar(0.05f, 0.9f, 0.999f, 1e-7f); // Define taxa, decaimentos e epsilon explicitamente.
    otimizador.atualizarParametros(pesos, vies, gradientes, 1); // Aplica o gradiente médio de um exemplo.
}
void loop() { // Sem processamento repetitivo neste exemplo.
}
```

### `Aprendizagem::Aprendizagem` — entrada simplificada do treino

`treinarModelo(Dataset::Dataset &dados, Compilar::Compilar &modelo, int totalEpocas, int tamanhoLote = 32)` encaminha dataset, modelo, total de épocas e tamanho do lote ao executor. Retorna `float` com a MSE final. Use quando quiser treinar pelo conjunto de dados e sem controlar manualmente cada época.

```cpp
#include <Aprendizagem/Aprendizagem.h> // Inclui a interface de alto nível do treino tradicional.
#include <Compilar/Compilar.h> // Fornece o estado treinável do modelo linear.
#include <Dataset/Dataset.h> // Fornece os vetores paralelos de amostras.
Dataset::Dataset dados; // Cria o conjunto que será preenchido antes do treino.
Compilar::Compilar modelo; // Cria os parâmetros e o otimizador associados.
Aprendizagem::Aprendizagem aprendizagem; // Cria o encaminhador do treinamento.
void setup() { // Configura os dados e inicia o treino uma vez.
    Serial.begin(115200); // Habilita a saída da perda.
    dados.carga.push_back(1.0f); // Guarda carga da primeira amostra.
    dados.distancia.push_back(2.0f); // Guarda a distância da mesma amostra, na mesma posição.
    dados.gasto.push_back(3.0f); // Guarda o alvo correspondente àquela amostra.
    modelo.neuronios.configurar(2, 1); // Configura duas entradas e um neurônio de saída.
    modelo.configurar(); // Inicializa parâmetros e Adam antes de treinar.
    float perdaFinal = aprendizagem.treinarModelo(dados, modelo, 10, 1); // Executa dez épocas com lote unitário.
    Serial.printf("MSE final: %.5f\\n", perdaFinal); // Exibe o valor numérico retornado.
}
void loop() { // Sem processamento contínuo neste exemplo.
}
```

### `Armazenamento::Armazenamento` — CSV em LittleFS

- `bool montar(const char *caminho)`: monta LittleFS e cria o CSV com dataset padrão se ausente. `caminho` é o caminho absoluto no sistema de arquivos. Retorna `true` em sucesso; `false` em falha. Use antes das operações de leitura e gravação.
- `bool lerDataset(const char *caminho, Dataset::Dataset &dados)`: lê as colunas `Carga`, `Distância` e `Gasto`, anexando valores aos vetores paralelos. Retorna se o arquivo foi lido e há amostras. Use após montar o sistema de arquivos.
- `String formatarNumero(float valor)`: formata número como texto adequado a um campo CSV. Retorna `String`; use ao preparar valores para persistência textual.
- `bool gravarRegistro(const char *caminho, float carga, float distancia, float gasto)`: anexa uma amostra ao arquivo. Os três valores são carga, distância e alvo/medição de gasto. Retorna sucesso da abertura e gravação. Use para preservar novas observações sem substituir as linhas anteriores.

```cpp
#include <Armazenamento/Armazenamento.h> // Inclui LittleFS e as operações CSV.
#include <Dataset/Dataset.h> // Define a estrutura que receberá as amostras.
Armazenamento::Armazenamento armazenamento; // Cria o serviço de persistência.
Dataset::Dataset dados; // Cria o destino da leitura.
const char *caminhoCsv = "/dados.csv"; // Identifica o arquivo no LittleFS.
void setup() { // Monta, lê, formata e acrescenta um registro de exemplo.
    Serial.begin(115200); // Abre o canal para informar falhas.
    if (armazenamento.montar(caminhoCsv)) { // Só acessa o CSV após montagem bem-sucedida.
        bool lido = armazenamento.lerDataset(caminhoCsv, dados); // Lê as amostras para os vetores.
        String numeroFormatado = armazenamento.formatarNumero(7.25f); // Converte uma medição para texto CSV.
        bool gravado = armazenamento.gravarRegistro(caminhoCsv, 5.0f, 2.0f, 7.25f); // Anexa carga, distância e gasto.
        Serial.printf("Leitura=%d, número=%s, gravação=%d\\n", lido, numeroFormatado.c_str(), gravado); // Relata resultados.
    } else { // Trata a falha sem tentar abrir arquivo não montado.
        Serial.println("Não foi possível montar o LittleFS."); // Informa a causa operacional.
    }
}
void loop() { // Sem processamento repetitivo neste exemplo.
}
```

### `Camadas::Camadas` — coleção ordenada de camadas

- `adicionar(int quantidadeEntradas, int quantidadeNeuronios)`: cria, configura e inicializa uma camada; não retorna valor. Use na ordem de execução da rede.
- `limpar()`: remove todas as camadas; não retorna valor. Use antes de reconstruir a arquitetura.
- `tamanho() const`: retorna `std::size_t`, a quantidade configurada. Use para percorrer a coleção com segurança.
- `operator[](std::size_t indice)` retorna referência modificável a `Neuronios::Neuronios`; a sobrecarga constante retorna referência somente leitura. `indice` é a posição de inserção, começando em zero. Use após garantir que o índice é menor que `tamanho()`.

```cpp
#include <Camadas/Camadas.h> // Declara o contêiner e inclui o tipo de camada.
Camadas::Camadas rede; // Cria a arquitetura vazia.
void setup() { // Monta duas camadas e consulta uma delas.
    Serial.begin(115200); // Permite imprimir a quantidade criada.
    rede.adicionar(2, 8); // Cria camada oculta para duas entradas e oito neurônios.
    rede.adicionar(8, 1); // Liga as oito ativações à saída de um neurônio.
    std::size_t quantidadeCamadas = rede.tamanho(); // Lê a contagem para validar a posição.
    if (quantidadeCamadas > 0) { // Evita acessar uma coleção vazia.
        rede[0].inicializar(); // Acessa a primeira camada e reinicializa seus parâmetros.
    }
    Serial.printf("Camadas: %u\\n", (unsigned int)quantidadeCamadas); // Exibe a dimensão da arquitetura.
    rede.limpar(); // Remove a arquitetura quando ela não será mais usada.
}
void loop() { // Sem processamento contínuo neste exemplo.
}
```

### `Compilar::Compilar` — inicialização dos parâmetros tradicionais

`configurar(float taxaAprendizado = 0.05f, float beta1 = 0.9f, float beta2 = 0.999f, float eps = 1e-7f)` inicializa `neuronios` e `otimizador` com os hiperparâmetros Adam. Não retorna valor. Use antes de `Aprendizagem`, `Epocas` ou `Treinar`. Os membros `neuronios` e `otimizador` são públicos para integração avançada.

```cpp
#include <Compilar/Compilar.h> // Declara o agrupador dos parâmetros e otimizador.
Compilar::Compilar modeloTreinavel; // Reserva a instância de treinamento.
void setup() { // Prepara os componentes uma única vez.
    Serial.begin(115200); // Disponibiliza diagnóstico serial.
    modeloTreinavel.neuronios.configurar(2, 1); // Define dimensões antes da inicialização aleatória.
    modeloTreinavel.configurar(0.01f, 0.9f, 0.999f, 1e-7f); // Inicializa pesos e estado de Adam.
}
void loop() { // Sem processamento contínuo neste exemplo.
}
```

### `Conversor::Conversor` — representação compacta e conversão

Constantes públicas de tipo: `FLOAT32` (0), `INT32` (1), `INT16` (2), `INT8` (3), `UINT8` (4) e `STRING` (5). `TETO_BYTES_TEXTO` define o limite de memória de texto. O construtor `Conversor(uint8_t tipoDadoEscolhido)` fixa o tipo dos dados de entrada; tipo não reconhecido seleciona `FLOAT32`.

- `uint8_t tamanhoBytes() const`: retorna bytes fixos por elemento; para texto retorna zero, pois a largura é fornecida pelo chamador. Use para dimensionar buffers numéricos.
- `bool converterDado(uint8_t destino, const uint8_t *entrada, uint8_t *saida, size_t quantidade, size_t larguraEntrada = 0, size_t larguraSaida = 0) const`: converte `quantidade` elementos; `destino` é o tipo alvo, ponteiros identificam buffers e larguras são necessárias para texto fixo. Retorna `true` se processado, `false` para tipo inválido ou limite de texto excedido. Use buffers de tamanho compatível com origem e destino.
- `static uint8_t tipoDestino(uint8_t tipo)`: retorna o tamanho fixo em bytes do tipo numérico; para texto/valor não reconhecido retorna zero. Use sem instanciar um conversor.
- `static size_t larguraMinima(const uint8_t *entrada, size_t quantidade, size_t larguraEntrada)`: retorna o tamanho da maior palavra mais espaço para terminador zero. Use para converter registros de texto sem truncar a maior palavra.

```cpp
#include <Conversor/Conversor.h> // Fornece os tipos e funções de conversão.
#include <cstring> // Fornece memcpy para representar um float como bytes de entrada.
Conversor::Conversor conversorFloat(Conversor::Conversor::FLOAT32); // Define FLOAT32 como tipo de origem.
float valorOriginal = 23.6f; // Valor numérico representativo a converter.
uint8_t bytesEntrada[sizeof(float)]; // Reserva exatamente quatro bytes para a origem float.
uint8_t bytesSaida[2]; // Reserva dois bytes para o inteiro de 16 bits.
void setup() { // Converte e consulta os tamanhos.
    Serial.begin(115200); // Abre a serial para relatar o estado.
    memcpy(bytesEntrada, &valorOriginal, sizeof(valorOriginal)); // Copia a representação binária para o buffer.
    uint8_t bytesFloat = conversorFloat.tamanhoBytes(); // Retorna quatro para FLOAT32.
    uint8_t bytesInt16 = Conversor::Conversor::tipoDestino(Conversor::Conversor::INT16); // Retorna dois para INT16.
    bool convertido = conversorFloat.converterDado(Conversor::Conversor::INT16, bytesEntrada, bytesSaida, 1); // Arredonda e grava um valor.
    Serial.printf("origem=%u, destino=%u, conversão=%d\\n", bytesFloat, bytesInt16, convertido); // Exibe metadados.
    const uint8_t palavras[] = {'l','i','g','a','r',0,'o','k',0}; // Dois textos com largura fixa de seis bytes.
    size_t largura = Conversor::Conversor::larguraMinima(palavras, 2, 6); // Calcula o maior texto mais terminador.
    Serial.printf("Largura mínima: %u\\n", (unsigned int)largura); // Mostra o tamanho necessário.
}
void loop() { // Sem processamento repetitivo neste exemplo.
}
```

### `Dataset::Dataset` — amostras em vetores paralelos

A classe não declara métodos públicos próprios. Os membros públicos `carga`, `distancia` e `gasto` são `std::vector<float>` alinhados por posição: todos os valores de um mesmo índice representam uma única amostra. Não existe retorno de método a documentar. Adicione uma amostra em todos os vetores antes de treinar.

```cpp
#include <Dataset/Dataset.h> // Declara o tipo de dados de treinamento.
Dataset::Dataset dados; // Cria os três vetores inicialmente vazios.
void setup() { // Adiciona uma amostra completa.
    dados.carga.push_back(5.0f); // Guarda a carga medida.
    dados.distancia.push_back(2.0f); // Guarda a distância associada, na mesma posição.
    dados.gasto.push_back(7.0f); // Guarda o alvo observado correspondente.
}
void loop() { // Sem processamento contínuo neste exemplo.
}
```

### `Epocas::Epocas` — repetição de treino por épocas

`float executar(int epochs, Dataset::Dataset &dados, Compilar::Compilar &modelo, int tamanhoLote = 32)` embaralha índices e coordena as atualizações em cada época. `epochs` é o número de passagens, `dados` contém as amostras, `modelo` contém parâmetros e `tamanhoLote` controla amostras por atualização (valores menores que um são tratados como um). Retorna MSE da última época. Use para gerir épocas sem implementar o laço.

```cpp
#include <Epocas/Epocas.h> // Inclui o executor de épocas e seus componentes.
#include <Compilar/Compilar.h> // Declara os parâmetros do modelo.
#include <Dataset/Dataset.h> // Declara as amostras.
Dataset::Dataset dados; // Reserva os vetores de treino.
Compilar::Compilar modelo; // Reserva pesos, viés e estado do otimizador.
Epocas::Epocas executorEpocas; // Cria o coordenador.
void setup() { // Executa um treino curto de demonstração.
    Serial.begin(115200); // Ativa a apresentação da MSE.
    dados.carga.push_back(1.0f); // Insere a carga de uma amostra.
    dados.distancia.push_back(2.0f); // Insere a característica pareada.
    dados.gasto.push_back(3.0f); // Insere o alvo pareado.
    modelo.neuronios.configurar(2, 1); // Ajusta a arquitetura linear às duas entradas.
    modelo.configurar(); // Inicializa a camada e Adam.
    float mseFinal = executorEpocas.executar(10, dados, modelo, 1); // Percorre dez épocas com lote unitário.
    Serial.printf("MSE final: %.5f\\n", mseFinal); // Exibe o retorno.
}
void loop() { // Sem trabalho periódico neste exemplo.
}
```

### `Gradientes::Gradientes` — derivadas de uma amostra

`void processarAmostraLote(float carga, float distancia, float gastoReal, const float *pesos, float vies, float *gradientes, float &somaErroQuadrado, Prever::Prever &previsor)` calcula previsão e erro, soma erro quadrático e acumula três gradientes. Os parâmetros de entrada representam amostra, coeficientes e intercepto; os ponteiros e referência são acumuladores mutáveis, e `previsor` executa a fórmula linear. Sem retorno. Use para compor gradientes de um lote antes de chamar Adam.

```cpp
#include <Gradientes/Gradientes.h> // Declara o acumulador de derivadas.
#include <Prever/Prever.h> // Declara a previsão linear reutilizada pelo cálculo.
Gradientes::Gradientes calculador; // Cria a operação de acumulação.
Prever::Prever previsor; // Cria o componente que calcula o valor estimado.
float pesos[2] = {0.1f, 0.2f}; // Coeficientes da carga e da distância.
float gradientes[3] = {0.0f, 0.0f, 0.0f}; // Zera acumuladores de peso 0, peso 1 e viés.
float somaErroQuadrado = 0.0f; // Inicia a soma quadrática do lote em zero.
void setup() { // Processa uma amostra de exemplo.
    calculador.processarAmostraLote(1.0f, 2.0f, 3.0f, pesos, 0.0f, gradientes, somaErroQuadrado, previsor); // Atualiza os acumuladores.
}
void loop() { // Sem processamento repetitivo neste exemplo.
}
```

### `Gravador::Gravador` — captura I2S e envio WAV

O construtor `Gravador(int tempoGravacaoSegundos = 2, int pinoBclk = 4, int pinoWs = 5, int pinoDin = 9)` escolhe duração e GPIOs BCLK, WS e DIN; a duração é limitada a 1–10 segundos. `iniciar()` prepara Serial a 921600 baud, cabeçalho WAV e I2S; não recebe parâmetros nem retorna valor. `gravar()` verifica comandos seriais e captura/envia um WAV quando recebe `G`; não recebe parâmetros nem retorna valor. Use `iniciar` em `setup` e `gravar` em `loop`. O exemplo pressupõe ESP32-S3, INMP441, driver I2S compatível e LED RGB no GPIO definido pela biblioteca.

```cpp
#include <Seai.h> // Importa o módulo Gravador junto às demais declarações públicas.
Gravador::Gravador gravador(2, 4, 5, 9); // Escolhe dois segundos e os GPIOs BCLK, WS e DIN usados no exemplo.
void setup() { // Configura o dispositivo uma vez antes do laço principal.
    gravador.iniciar(); // Inicializa comunicação serial, cabeçalho WAV e canal I2S.
}
void loop() { // Consulta comandos continuamente sem bloquear quando não há dados.
    gravador.gravar(); // Aguarda o comando 'G' e, quando recebido, captura e envia o áudio.
}
```

### `Modelo::Modelo` — rede neural genérica

`bool executar(const std::vector<std::vector<float>> &entradas, const std::vector<std::vector<float>> &saidasEsperadas, const std::vector<float> &entradaPrevisao)` valida dimensões e valores, constrói camadas, treina com Adam, calcula a previsão e apresenta a perda final. As matrizes devem conter amostras compatíveis e a entrada de previsão deve ter o mesmo número de características. Retorna `true` em execução concluída, `false` em dados/parâmetros inválidos. Use como interface principal.

Membros públicos configuráveis: `taxaAprendizado` (float), `epocas` (int), `tamanhoLote` (int), `neuroniosPorCamada` (`std::vector<int>`) e `verbose` (`Verbose::Verbose`). Configure-os antes de chamar `executar`.

```cpp
#include <Seai.h> // Importa Modelo e as classes de suporte.
#include <vector> // Declara vetores e matrizes dinâmicas.
Modelo::Modelo modelo; // Cria uma rede com hiperparâmetros padrão.
void setup() { // Configura e executa treino mais previsão.
    Serial.begin(115200); // Abre canal para erros e perda final.
    modelo.taxaAprendizado = 0.05f; // Define o passo do otimizador.
    modelo.epocas = 10; // Define dez passagens completas para este exemplo.
    modelo.tamanhoLote = 1; // Atualiza os pesos após cada amostra.
    modelo.neuroniosPorCamada = {8}; // Configura oito unidades na camada oculta.
    const std::vector<std::vector<float>> entradas = {{1.0f, 2.0f}, {2.0f, 3.0f}}; // Duas amostras com duas características.
    const std::vector<std::vector<float>> alvos = {{3.0f}, {5.0f}}; // Um valor esperado para cada amostra.
    const std::vector<float> consulta = {1.5f, 2.5f}; // Entrada para previsão após o treino.
    bool sucesso = modelo.executar(entradas, alvos, consulta); // Valida, treina e estima a saída.
    if (!sucesso) { // Trata recusas sem assumir que houve resultado.
        Serial.println("Revise as dimensões dos dados e a configuração do modelo."); // Indica próximos passos.
    }
}
void loop() { // Sem processamento periódico neste exemplo.
}
```

### `Neuronios::Neuronios` — camada totalmente conectada

- `configurar(int quantidadeEntradas, int quantidadeNeuronios = NUMERO_NEURONIOS_PADRAO)`: define dimensões e limita neurônios a 1–64; sem retorno. Use antes de inicializar.
- `float gerarNumeroAleatorio()`: devolve valor pseudoaleatório entre zero e um, proveniente do gerador ESP32. Use se precisar de uma amostra do mesmo gerador.
- `inicializar()`: inicializa pesos com escala Glorot e vieses em zero; sem retorno. Use após configurar.
- `calcular(const std::vector<float> &entradas, std::vector<float> &saidas, bool ativacaoTanh) const`: calcula uma saída por neurônio; `entradas` deve ter largura configurada, `saidas` é preenchido, e `ativacaoTanh` seleciona `tanh` (camada oculta) ou saída linear. Sem retorno; incompatibilidade limpa a saída e informa pela Serial.

Os membros públicos incluem limites, pesos/vieses legados, dimensões, matrizes de pesos da camada e vieses por neurônio.

```cpp
#include <Neuronios/Neuronios.h> // Inclui a implementação da camada.
#include <vector> // Fornece os vetores de entrada e saída.
Neuronios::Neuronios camada; // Cria a camada com valores padrão até configuração.
void setup() { // Configura, inicializa, calcula e consulta o gerador.
    Serial.begin(115200); // Permite receber diagnósticos de dimensão.
    camada.configurar(2, 8); // Define duas características de entrada e oito neurônios.
    camada.inicializar(); // Sorteia pesos na escala Glorot e zera vieses.
    float numeroInicializacao = camada.gerarNumeroAleatorio(); // Obtém uma fração pseudoaleatória para uso diagnóstico.
    const std::vector<float> entradas = {0.5f, 0.25f}; // Fornece exatamente duas características.
    std::vector<float> saidas; // Reserva o vetor que receberá oito ativações.
    camada.calcular(entradas, saidas, true); // Usa tanh porque o exemplo representa camada oculta.
    Serial.printf("Amostra aleatória: %.4f; saídas: %u\\n", numeroInicializacao, (unsigned int)saidas.size()); // Informa resultados.
}
void loop() { // Sem processamento contínuo neste exemplo.
}
```

### `Perdas::Perdas` — erro quadrático médio

`float calcularMSE(float somaErrosQuadrados, int totalAmostras)` divide a soma de erros quadráticos pela quantidade de amostras e retorna MSE; para total menor ou igual a zero retorna `0.0f`. Use para normalizar a perda acumulada do conjunto/lote.

```cpp
#include <Perdas/Perdas.h> // Declara a métrica MSE.
Perdas::Perdas metrica; // Cria o calculador sem estado adicional.
void setup() { // Calcula uma média sobre quatro amostras.
    Serial.begin(115200); // Ativa a saída numérica.
    float mse = metrica.calcularMSE(2.0f, 4); // Divide soma de erros quadráticos pelo total.
    Serial.printf("MSE: %.5f\\n", mse); // Exibe a perda resultante.
}
void loop() { // Sem processamento repetitivo neste exemplo.
}
```

### `Prever::Prever` — previsão linear

`float calcular(float carga, float distancia, const float *pesos, float vies)` aplica `pesos[0] * carga + pesos[1] * distancia + vies`. Recebe as características, dois pesos e viés; retorna `float`. Use para inferência linear com parâmetros já ajustados.

```cpp
#include <Prever/Prever.h> // Declara o avaliador linear.
Prever::Prever previsor; // Cria o componente de previsão.
float pesosModelo[2] = {0.5f, 1.25f}; // Coeficientes das características na ordem de entrada.
void setup() { // Calcula uma consulta.
    Serial.begin(115200); // Abre a serial para imprimir previsão.
    float gastoPrevisto = previsor.calcular(2.0f, 3.0f, pesosModelo, 0.1f); // Combina carga, distância, pesos e viés.
    Serial.printf("Previsão: %.2f\\n", gastoPrevisto); // Exibe o valor calculado.
}
void loop() { // Sem processamento contínuo neste exemplo.
}
```

### `Treinar::Treinar` — processamento direto por lotes

`float executarLotes(Dataset::Dataset &dados, const std::vector<int> &indices, int tamanho_lote, Compilar::Compilar &modelo, Gradientes::Gradientes &gradientesCalc, Perdas::Perdas &perdasCalc, Prever::Prever &previsor)` percorre os índices em lotes, acumula erros e gradientes e atualiza o modelo. Os índices mapeiam a ordem processada às posições do dataset; os componentes restantes fornecem modelo, derivadas, métrica e previsão. Retorna soma ponderada de perdas da época. Use para controle de baixo nível, garantindo vetores alinhados e índices válidos.

```cpp
#include <Treinar/Treinar.h> // Declara o executor por lote.
#include <Compilar/Compilar.h> // Fornece parâmetros e Adam.
#include <Dataset/Dataset.h> // Fornece as amostras.
#include <Gradientes/Gradientes.h> // Fornece acumulação de gradientes.
#include <Perdas/Perdas.h> // Fornece cálculo de MSE.
#include <Prever/Prever.h> // Fornece a previsão linear.
#include <vector> // Fornece a lista de índices.
Dataset::Dataset dados; // Cria o conjunto de amostras.
Compilar::Compilar modelo; // Cria os parâmetros treináveis.
Gradientes::Gradientes gradientes; // Cria o calculador de derivadas.
Perdas::Perdas perdas; // Cria a métrica usada para normalizar o lote.
Prever::Prever previsor; // Cria a previsão chamada pelo calculador.
Treinar::Treinar treinador; // Cria o processador dos lotes.
void setup() { // Prepara uma amostra e executa um lote.
    Serial.begin(115200); // Abre a saída para a perda.
    dados.carga.push_back(1.0f); // Insere carga na posição zero.
    dados.distancia.push_back(2.0f); // Insere distância pareada na posição zero.
    dados.gasto.push_back(3.0f); // Insere o alvo na posição zero.
    modelo.neuronios.configurar(2, 1); // Configura o modelo linear para duas entradas.
    modelo.configurar(); // Inicializa pesos, viés e Adam.
    const std::vector<int> indicesAmostras = {0}; // Referencia a única amostra válida, cujo índice é zero.
    float perdaEpoca = treinador.executarLotes(dados, indicesAmostras, 1, modelo, gradientes, perdas, previsor); // Treina lote unitário.
    Serial.printf("Perda ponderada: %.5f\\n", perdaEpoca); // Exibe o retorno da época.
}
void loop() { // Sem processamento repetitivo neste exemplo.
}
```

### `Verbose::Verbose` — logs do treinamento

Membros públicos booleanos: `mostrarConfiguracao`, `mostrarNumeroEpoca` e `mostrarPerdaPorEpoca` habilitam cada categoria independentemente (inicialmente desabilitados).

- `registrarConfiguracao(float taxaAprendizado, int epocas, int tamanhoLote, const std::vector<int> &neuroniosPorCamada, std::size_t numeroSaidas)`: imprime hiperparâmetros e dimensões somente se a opção estiver ligada. Sem retorno; use no início do treino.
- `registrarEpoca(int epoca)`: imprime número de época se habilitado. Sem retorno; o número apresentado começa em um.
- `registrarPerdaEpoca(float perda)`: imprime perda se habilitado. Sem retorno; use para diagnóstico de convergência.
- `mostrarResultados(float carga, float distancia, float perdas, const float *pesos, float vies, float gasto_previsto)` e `mostrarResultados(const std::vector<float> &entrada, float perdas, const std::vector<float> &saida)`: sobrecargas compatíveis com API linear e vetorial; imprimem apenas perda final, não os outros valores. Sem retorno. Use ao encerrar inferência/treino.

```cpp
#include <Verbose/Verbose.h> // Declara as opções e mensagens de acompanhamento.
#include <vector> // Fornece a configuração de camadas ocultas.
Verbose::Verbose logTreino; // Cria o registrador.
void setup() { // Demonstra todos os métodos públicos de log.
    Serial.begin(115200); // Habilita as mensagens pela porta serial.
    logTreino.mostrarConfiguracao = true; // Habilita resumo dos hiperparâmetros.
    logTreino.mostrarNumeroEpoca = true; // Habilita numeração das épocas.
    logTreino.mostrarPerdaPorEpoca = true; // Habilita perdas intermediárias.
    const std::vector<int> camadasOcultas = {8}; // Descreve uma camada oculta de oito neurônios.
    logTreino.registrarConfiguracao(0.05f, 10, 2, camadasOcultas, 1); // Registra configuração de exemplo.
    logTreino.registrarEpoca(1); // Registra a primeira época, numerada a partir de um.
    logTreino.registrarPerdaEpoca(0.25f); // Registra a perda da época.
    const float pesos[2] = {0.5f, 1.25f}; // Cria parâmetros para a assinatura linear legada.
    logTreino.mostrarResultados(2.0f, 3.0f, 0.25f, pesos, 0.1f, 4.85f); // Mostra somente perda final.
    const std::vector<float> entrada = {2.0f, 3.0f}; // Cria uma entrada para a assinatura vetorial.
    const std::vector<float> saida = {4.85f}; // Cria a saída associada à entrada.
    logTreino.mostrarResultados(entrada, 0.25f, saida); // Usa a sobrecarga genérica, também restrita à perda.
}
void loop() { // Sem processamento contínuo neste exemplo.
}
```

## Gravador de áudio

O fluxo abaixo corresponde ao exemplo `examples/GravadorExemplo/GravadorExemplo.ino` e ao script `ferramentas/gravadorVoz/principal.py`. Requer placa e microfone compatíveis, script e dependências instalados, além da porta serial da placa.

1. Abra o exemplo do gravador em `examples/GravadorExemplo/GravadorExemplo.ino` na Arduino IDE e verifique o código.
2. Conecte a placa ao notebook pelo cabo USB de dados.
3. Selecione a placa correta e a porta correspondente na Arduino IDE.
4. Envie o código para a placa e aguarde a conclusão do carregamento.
5. Feche o Serial Monitor; o script Python precisa acessar a porta serial sem concorrência.
6. Abra `ferramentas/gravadorVoz/principal.py`, na pasta `ferramentas/gravadorVoz`, em um editor.
7. Execute o script Python conforme as instruções em [`ferramentas/gravadorVoz/README.md`](ferramentas/gravadorVoz/README.md).
8. Digite `1` e pressione Enter; depois digite `1` e pressione Enter novamente para selecionar a primeira opção/porta apresentada.
9. Digite o rótulo da palavra a gravar, por exemplo `ligar`, e pressione Enter.
10. A placa pisca em vermelho. Quando o LED ficar verde, fale a palavra uma vez, de forma clara.
11. Depois da gravação, aparecem três opções: ouvir, salvar ou excluir.
12. Ouvir reproduz o áudio quantas vezes quiser. Salvar mantém o arquivo e volta ao menu. Excluir apaga a gravação e volta ao menu.

O script também documenta instalação no Windows, configuração do INMP441 e opções do menu. Confira seu README específico, pois detalhes de placa, porta e instalação dependem do ambiente.

## Referências

- [Site da biblioteca Small Embedded AI](https://small-embedded-ai.netlify.app/)
- [Repositório e exemplos da biblioteca](https://github.com/brunomakeram/Small-Embedded-Ai)
- [Documentação oficial da Arduino IDE](https://docs.arduino.cc/software/ide-v2/)
- [Documentação oficial Arduino — bibliotecas](https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-installing-a-library/)
- [Documentação Espressif ESP-IDF — I2S](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/i2s.html)
- [Documentação oficial Arduino-ESP32 — instalação](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
- [Documentação Python — tutorial](https://docs.python.org/3/tutorial/)
- [PySerial — documentação](https://pyserial.readthedocs.io/en/latest/)
- [Glossário do projeto](GLOSSARIO.md)
