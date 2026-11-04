# Small Embedded AI (Seai)

Biblioteca C++ para treinar e executar modelos de Machine Learning diretamente
em placas ESP32 e Raspberry Pi Pico (RP2040). A Seai oferece redes neurais
multicamada, conversão de dados, persistência de pesos e captura de áudio I2S.
Esta é a versão 3.0.0; somente ESP32 e Raspberry Pi Pico (RP2040) são
arquiteturas suportadas.

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
apontam para imagens que poderão ser adicionadas à pasta `imagens/` do GitHub;
o código executável fica nos links `.ino`.

Os conjuntos de treino sintéticos continuam no código para demonstrar o
treinamento. Os dados usados para a previsão/classificação de cada execução
são informados pelo Monitor Serial; configure a velocidade indicada para cada
exemplo e selecione final de linha `Nova linha` ou `NL e CR`.

### Previsão de bateria — regressão linear

![Exemplo PrevisaoBateria](imagens/PrevisaoBateria.jpg)

[Abrir o sketch PrevisaoBateria.ino](examples/PrevisaoBateria/PrevisaoBateria.ino)

**Entrada no Monitor Serial (115200 baud):** depois do treinamento, digite a
carga transportada em kg e, em seguida, a distância em km, uma por linha
(exemplo: `45.0` e `30.0`). Use ponto como separador decimal. O exemplo usa
viagens sintéticas para treino; substitua `dadosFisicos` por medições reais
quando adaptar o modelo. Cada amostra de treino contém carga, distância e
percentual de bateria gasto.

### PLN — classificação de palavras

![Exemplo ClassificacaoPLN](imagens/ClassificacaoPLN.jpg)

[Abrir o sketch ClassificacaoPLN.ino](examples/ClassificacaoPLN/ClassificacaoPLN.ino)

**Entrada no Monitor Serial (115200 baud):** após o treino, digite uma frase
por linha para classificar. Digite `sair` para terminar. Exemplo:
`socorro para acidente`. Use texto ASCII sem acentos; o vocabulário fixo
reconhece palavras relacionadas a remédio, resgate e mercadoria, e ignora
palavras que não estejam nele. Para adaptar o treinamento, edite
`frasesTreino`, seus rótulos `classesTreino` (`0` remédio, `1` resgate,
`2` mercadoria) e `vocabulario`.

### Classificação de sentimentos

![Exemplo ClassificacaoSentimentos](imagens/ClassificacaoSentimentos.jpg)

[Abrir o sketch ClassificacaoSentimentos.ino](examples/ClassificacaoSentimentos/ClassificacaoSentimentos.ino)

**Entrada no Monitor Serial (115200 baud):** após o treino, digite uma frase
por linha para classificar. Digite `sair` para terminar. Exemplo:
`gostei muito`. Use texto ASCII sem acentos; o vocabulário fixo reconhece
palavras positivas, negativas e neutras, e ignora palavras que não estejam
nele. Para adaptar o treinamento, edite `frasesTreino`, seus rótulos
`classesTreino` (`0` positivo, `1` negativo, `2` neutro) e `vocabulario`.

### Anomalia de hélice — análise da corrente do motor

![Exemplo AnomaliaHelice](imagens/AnomaliaHelice.jpg)

[Abrir o sketch AnomaliaHelice.ino](examples/AnomaliaHelice/AnomaliaHelice.ino)

**Entrada no Monitor Serial (115200 baud):** digite primeiro pelo menos duas
leituras de corrente normal, em ampères, separadas por vírgula (exemplo:
`2.00,2.04,1.98,2.03`). Depois digite a corrente atual, em ampères, numa linha
(exemplo: `2.50`). Use ponto decimal. O sketch calcula média e desvio-padrão
da referência; a regra N-sigma usa o limite definido por
`NUMERO_DESVIOS_ANOMALIA`. O modelo de demonstração usa observações
sintéticas rotuladas; para adaptá-lo, substitua `desviosNormalizados` e
`rotulosAnomalia` com dados representativos.

### Exemplo geral de modelo — XOR

[Abrir o sketch Seai.ino](examples/Seai/Seai.ino)

**Entrada no Monitor Serial (115200 baud):** após iniciar, digite dois valores
entre `0` e `1`, um por linha (exemplo: `1` e `0`). O modelo treina com a tabela
XOR incluída no exemplo e imprime a previsão para o par informado.

### Teste de persistência no Pico

[Abrir o sketch RP2040_16MB.ino](examples/RP2040_16MB/RP2040_16MB.ino)

**Entrada no Monitor Serial (115200 baud):** digite um número para previsão
(exemplo: `3.0`). O treino demonstra a relação `y = 2x`. O perfil da placa deve
ser compatível com a flash de 16 MB; o exemplo usa 4096 bytes para EEPROM
emulada.

### Gravador de áudio

[Abrir o sketch GravadorExemplo.ino](examples/GravadorExemplo/GravadorExemplo.ino)

O exemplo é destinado às placas ESP32 compatíveis com a captura I2S e requer
microfone INMP441. Abra o Monitor Serial em **921600 baud** e envie a letra
`G` para iniciar a gravação. Para ligar o microfone, veja as instruções em
[ferramentas/gravadorVoz/README.md](ferramentas/gravadorVoz/README.md).

## Uso da API de modelo

As linhas de `entradas` e `saidasEsperadas` formam amostras correspondentes.
`executar` treina o modelo e calcula uma previsão inicial; depois, `prever`
avalia outras entradas com os mesmos parâmetros.

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
const std::vector<std::vector<float>> saidasEsperadas = {
    {0.0f},
    {3.0f},
    {2.0f},
    {5.0f}
};

void setup()
{
    Serial.begin(115200);
#if !SEAI_ESP32
    randomSeed(analogRead(A0));
#endif
    std::vector<float> primeiraEntrada = entradas[0];
    if (modelo.executar(entradas, saidasEsperadas, primeiraEntrada)) {
        std::vector<float> previsao = modelo.prever({1.0f, 0.0f});
        if (!previsao.empty()) {
            Serial.printf("Saida prevista: %.4f\n", previsao[0]);
        }
    }
}

void loop()
{
}
```

Configure `taxaAprendizado`, `epocas`, `tamanhoLote` e
`neuroniosPorCamada` antes de chamar `executar`. Configure
`verbose.mostrarConfiguracao`, `verbose.mostrarNumeroEpoca` ou
`verbose.mostrarPerdaPorEpoca` para habilitar as respectivas mensagens.

### API de alto nível

`Modelo::Modelo` é a interface recomendada para a maioria dos projetos. Suas
propriedades públicas permitem configurar o treino:

| Propriedade | Tipo | Padrão | Uso |
|---|---:|---:|---|
| `taxaAprendizado` | `float` | `0.05` | Tamanho das atualizações do Adam. |
| `epocas` | `int` | `500` | Quantas vezes percorrer o conjunto de treino. |
| `tamanhoLote` | `int` | `32` | Quantas amostras acumulam gradientes antes de atualizar os parâmetros. |
| `neuroniosPorCamada` | `std::vector<int>` | `{8}` | Número de neurônios em cada camada oculta; a saída é dimensionada pelos alvos. |
| `perdaFinal` | `float` | `0.0` | Perda MSE do treino concluído; consulte depois de `executar`. |
| `verbose` | `Verbose::Verbose` | opções desligadas | Controla os registros do treino. |

`executar(entradas, saidasEsperadas, entradaPrevisao)` treina e retorna `true`
quando os dados são válidos. `prever(entrada)` faz inferência após o treino e
retorna um vetor vazio se o modelo ou a dimensão da entrada forem inválidos.
Todas as amostras precisam ter dimensões consistentes e conter valores finitos.
As camadas ocultas usam `tanh`; a camada de saída é linear, portanto aceita
saídas contínuas para regressão.

O número da época é registrado uma vez ao final de cada época. Para habilitar a
persistência dos pesos, defina a opção antes de incluir a biblioteca:

```cpp
#define SALVAR_PESO SALVAR_PESO_LIGADO
#include <Seai.h>
```

Por padrão, `SALVAR_PESO` é `SALVAR_PESO_DESLIGADO`. Quando ligada, a Seai tenta
carregar os pesos salvos compatíveis antes do treino e grava os pesos finais
uma vez após o treino. No ESP32, o arquivo CSV fica no LittleFS; no RP2040, os pesos são gravados em
binário na EEPROM emulada do core. A EEPROM do RP2040 usa 4096 bytes; modelos
que não couberem no espaço disponível não são gravados. Se as dimensões da rede
mudarem, os pesos antigos são ignorados.

No RP2040, inicialize o gerador aleatório no `setup()` antes do treino usando
um pino analógico livre, por exemplo `randomSeed(analogRead(A0));`. O ESP32
continua usando o gerador `esp_random()` nativo.

### Instalar UF2 no Raspberry Pi Pico

O projeto inclui um UF2 pronto em
`examples/RP2040_16MB/RP2040_16MB.ino.uf2`. O procedimento de arrastar o UF2
instala o firmware na placa; abra o sketch correspondente na Arduino IDE para
usar o Monitor Serial.

#### Caso 1 — usar o código pronto

O arquivo UF2 pronto acompanha o projeto. Com a placa desconectada, mantenha
pressionado o botão **BOOT**, conecte o cabo USB e solte o botão quando a
unidade aparecer no computador. Arraste
`examples/RP2040_16MB/RP2040_16MB.ino.uf2` para essa unidade. Não é necessário
exportar o binário.

#### Caso 2 — modificar o código

1. Compile o sketch na Arduino IDE.
2. Abra **Rascunho (Sketch) > Exportar binário compilado**.
3. Abra a pasta `build` criada ao lado do arquivo `.ino`.
4. Entre na pasta com o nome do RP2040 e copie o arquivo terminado em `.UF2`.
5. Desconecte o USB da placa.
6. Segure o botão **BOOT** e, mantendo-o pressionado, reconecte o USB.
7. Cole (`Ctrl+V`) o arquivo UF2 na unidade/pasta `RPI` que aparecer.
8. Grave novamente pela Arduino IDE. A porta da placa aparecerá; abra o
   Monitor Serial em 115200 baud e use a entrada indicada no exemplo.

No exemplo de persistência, selecione um perfil de placa compatível com flash
de 16 MB. A placa deve usar o core Arduino-Pico e a EEPROM emulada reserva
4096 bytes; os 16 MB de flash não são usados como espaço de EEPROM.

### API de baixo nível

Os módulos também podem ser usados separadamente quando o projeto precisa
controlar uma camada linear e cada etapa do treino:

| Classe | Métodos principais | Função |
|---|---|---|
| `Neuronios::Neuronios` | `configurar`, `inicializar`, `calcular` | Mantém pesos e vieses de uma camada totalmente conectada; aceita de 1 a 64 neurônios. |
| `Camadas::Camadas` | `adicionar`, `limpar`, `tamanho`, `operator[]` | Organiza uma sequência de camadas. |
| `Compilar::Compilar` | `configurar` | Configura e inicializa uma camada e o seu otimizador Adam. |
| `Aprendizagem::Aprendizagem` | `treinarModelo` | Treina as matrizes de amostras por várias épocas e devolve a perda MSE final. |
| `Epocas::Epocas` | `executar` | Valida os dados, embaralha as amostras e coordena as épocas. |
| `Treinar::Treinar` | `executarLotes` | Processa lotes, acumula gradientes e atualiza os parâmetros. |
| `Gradientes::Gradientes` | `processarAmostraLote` | Calcula previsões e acumula gradientes de pesos e vieses para uma amostra. |
| `Prever::Prever` | `calcular` | Calcula a saída linear de uma camada configurada. |
| `Perdas::Perdas` | `calcularMSE` | Divide a soma dos erros quadráticos pelo total informado. |

Exemplo de regressão linear usando essa interface:

```cpp
#include <Seai.h>
#include <vector>

const std::vector<std::vector<float>> entradas = {
    {0.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 0.0f},
    {1.0f, 1.0f}
};
const std::vector<std::vector<float>> saidasEsperadas = {
    {0.0f},
    {3.0f},
    {2.0f},
    {5.0f}
};

void setup()
{
    Serial.begin(115200);

    Compilar::Compilar camada;
    if (!camada.configurar(2, 1, 0.03f)) {
        Serial.println("Falha ao configurar a camada.");
        return;
    }

    Aprendizagem::Aprendizagem aprendizagem;
    float perda = aprendizagem.treinarModelo(
        entradas, saidasEsperadas, camada, 1500, 4);
    Serial.printf("Perda MSE: %.6f\n", perda);

    Prever::Prever previsor;
    std::vector<float> previsao;
    if (previsor.calcular({1.0f, 0.0f}, camada.neuronios, previsao)) {
        Serial.printf("Saida prevista: %.4f\n", previsao[0]);
    }
}

void loop()
{
}
```

Essa composição de baixo nível configura uma única camada linear. Para redes
com várias camadas ocultas e para usar diretamente `modelo.prever`, utilize
`Modelo::Modelo`.

## Dados de treino em CSV (LittleFS)

`Armazenamento::Armazenamento` trabalha com matrizes
`std::vector<std::vector<float>>`; não depende de uma classe Dataset. O CSV tem
um cabeçalho com nomes livres e uma linha numérica por amostra. Em
`lerDados(caminho, quantidadeEntradas, entradas, saidas)`, o argumento
`quantidadeEntradas` define quantas colunas iniciais são entradas; as demais
colunas são saídas. O cabeçalho deve conter pelo menos uma coluna de cada tipo,
e todas as linhas devem ter o mesmo número de campos.

| Método | O que faz |
|---|---|
| `montar(caminho)` | Monta o LittleFS e cria o arquivo com `Armazenamento::DATASET_PADRAO` se ele ainda não existir. |
| `lerDados(caminho, quantidadeEntradas, entradas, saidas)` | Lê o CSV e separa as matrizes de entrada e saída. |
| `gravarDados(caminho, nomesEntradas, nomesSaidas, entradas, saidas)` | Grava um CSV novo com os nomes de coluna e as matrizes fornecidas. |
| `gravarRegistro(caminho, entradas, saidas)` | Acrescenta uma amostra respeitando a ordem das colunas já existente no arquivo. |

Exemplo de gravação e leitura. O cabeçalho nomeia as colunas; a posição de
separação continua sendo informada por `quantidadeEntradas`:

```cpp
#include <Seai.h>
#include <vector>

void exemploCsv()
{
    Armazenamento::Armazenamento armazenamento;
    const char *caminho = "/treino.csv";
    const std::vector<String> nomesEntradas = {"carga_kg", "distancia_km"};
    const std::vector<String> nomesSaidas = {"bateria_gasta"};
    const std::vector<std::vector<float>> entradas = {
        {10.0f, 5.0f},
        {30.0f, 20.0f}
    };
    const std::vector<std::vector<float>> saidas = {
        {3.75f},
        {7.50f}
    };

    if (!armazenamento.montar(caminho) ||
        !armazenamento.gravarDados(
            caminho, nomesEntradas, nomesSaidas, entradas, saidas)) {
        Serial.println("Falha ao preparar o CSV.");
        return;
    }

    std::vector<std::vector<float>> entradasLidas;
    std::vector<std::vector<float>> saidasLidas;
    if (armazenamento.lerDados(
            caminho, nomesEntradas.size(), entradasLidas, saidasLidas)) {
        Serial.printf("Amostras lidas: %u\n", (unsigned)entradasLidas.size());
    }
}
```

## Adam e conversão

`Adam::Adam` mantém os momentos do otimizador em vetores, um valor para cada
parâmetro; não há limite fixo de três parâmetros. O construtor e
`inicializar` aceitam taxa de aprendizado, decaimento do primeiro momento,
decaimento do segundo momento e margem numérica. `atualizarParametros`
recebe o vetor de parâmetros, os gradientes acumulados e o número de amostras
do lote. Calcula a média local dos gradientes sem alterar o vetor recebido.

Exemplo de atualização de cinco parâmetros:

```cpp
Adam::Adam otimizador(0.01f);
std::vector<float> parametros = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
const std::vector<float> gradientes = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
otimizador.atualizarParametros(parametros, gradientes, 4);
```

`Conversor::Conversor` converte valores entre `FLOAT32`, inteiros e `STRING`.
Os tipos disponíveis são `FLOAT32`, `INT32`, `INT16`, `INT8`, `UINT8` e
`STRING`. `tamanhoBytes()` informa a largura dos tipos numéricos.
`converterDado(tipoDestino, entrada, saida, quantidade, larguraEntrada,
larguraSaida)` converte um vetor de valores; as larguras são necessárias para
origem ou destino `STRING`. `larguraMinima` calcula o espaço necessário para
guardar a maior palavra de um conjunto de strings de largura fixa.

```cpp
#include <Seai.h>

Conversor::Conversor conversor(Conversor::Conversor::STRING);
uint8_t palavras[2][16] = {"ligar", "desligar"};
uint8_t copia[2][16] = {};

void setup()
{
    bool convertido = conversor.converterDado(
        Conversor::Conversor::STRING,
        &palavras[0][0],
        &copia[0][0],
        2,
        16,
        16);
    Serial.println(convertido ? "Conversao concluida." : "Falha na conversao.");
}

void loop()
{
}
```

`Verbose::Verbose` permite habilitar separadamente
`mostrarConfiguracao`, `mostrarNumeroEpoca` e `mostrarPerdaPorEpoca`. No modelo,
configure essas opções por `modelo.verbose` antes de chamar `executar`.

## Gravador de áudio

Além de treinar e executar modelos embarcados, a Seai pode se comunicar pela
porta serial com um script Python no notebook para gravar áudio na placa e
transferir as gravações para o computador. Pelo menu do script, é possível
ouvir as gravações, salvá-las no notebook ou excluí-las.

O fluxo corresponde ao sketch
`examples/GravadorExemplo/GravadorExemplo.ino` e ao script
`ferramentas/gravadorVoz/principal.py`. Requer placa e microfone compatíveis,
script e dependências instalados, além da porta serial da placa.

No sketch, `Gravador::Gravador(tempoSegundos, pinoBclk, pinoWs, pinoDin)`
configura a duração (limitada de 1 a 10 segundos) e os pinos I2S; os valores
padrão são 2 segundos e GPIO 4, 5 e 9. `iniciar()` prepara a captura e abre a
Serial em 921600 baud. Chame `gravar()` repetidamente em `loop()`: ao receber
`G` pela Serial, o gravador captura áudio mono PCM de 16 kHz/16 bits e envia o
cabeçalho WAV e os dados binários pela mesma porta.

```cpp
#include <Seai.h>

Gravador::Gravador gravador(2, 4, 5, 9);

void setup()
{
    gravador.iniciar();
}

void loop()
{
    gravador.gravar();
}
```

1. Abra o sketch do gravador na Arduino IDE.
2. Conecte a placa pelo cabo USB de dados e selecione a placa e porta corretas.
3. Envie o sketch e aguarde o carregamento.
4. Feche o Serial Monitor para liberar a porta serial.
5. Siga as instruções de instalação e execução em
   [`ferramentas/gravadorVoz/README.md`](ferramentas/gravadorVoz/README.md).
6. No menu do script, selecione a porta, informe o rótulo da palavra e fale
   quando o LED ficar verde.
7. Ouça, salve ou exclua a gravação pelas opções apresentadas.

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

## Notas da versão 3.0.0

- Atualizada a identificação da biblioteca para a versão 3.0.0; a arquitetura
  agora está limitada a ESP32 e Raspberry Pi Pico (RP2040).
- Os exemplos de inferência e gravação passaram a receber entradas pelo
  Monitor Serial e informam o formato esperado.
- Adicionadas instruções para instalar UF2 pronto no Pico ou exportar e gravar
  um UF2 compilado após modificar o sketch.
