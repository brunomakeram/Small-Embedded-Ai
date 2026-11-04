# Glossário — Small Embedded AI

Este glossário reúne termos técnicos que aparecem no README, nos exemplos e na documentação do gravador. As definições refletem o comportamento documentado no projeto.

## Inteligência artificial e treinamento

- **Amostra** — observação individual usada no treino ou na avaliação. Cada amostra contém valores de entrada e os valores de saída esperados correspondentes.
- **Adam** — otimizador que ajusta pesos e vieses usando médias móveis dos gradientes.
- **Ativação / `tanh`** — transformação aplicada aos valores de uma camada. A rede usa `tanh` nas camadas ocultas e uma saída linear.
- **Backpropagation (retropropagação)** — cálculo do efeito do erro nas camadas anteriores para obter os gradientes dos parâmetros.
- **Camada** — conjunto de neurônios que transforma um vetor de entrada em outro vetor. A saída de uma camada alimenta a seguinte.
- **Característica (feature)** — valor de entrada que descreve uma amostra, como carga e distância ou a presença de uma palavra.
- **Classificação** — tarefa que associa uma entrada a uma categoria, como sentimento positivo, negativo ou neutro. Nos exemplos, a classe escolhida é a de maior valor previsto.
- **Conjunto de dados (dataset)** — coleção de amostras usadas em treino ou avaliação. Nas matrizes de treino, cada linha de `entradas` corresponde à linha de mesmo índice em `saidasEsperadas`.
- **Época** — passagem completa por todas as amostras de treinamento.
- **Gradiente** — derivada que indica como uma alteração em um parâmetro afeta a perda.
- **Hiperparâmetro** — configuração do treino definida antes de executá-lo, como taxa de aprendizado, épocas, tamanho do lote e neurônios por camada.
- **Inferência / previsão** — cálculo da saída do modelo para uma entrada, usando os parâmetros atuais.
- **Lote (batch)** — grupo de amostras cujos gradientes são acumulados antes de atualizar os parâmetros.
- **MSE (Mean Squared Error, erro quadrático médio)** — média dos erros ao quadrado entre previsões e valores esperados; é a perda usada para acompanhar o treino.
- **Neurônio** — unidade que combina entradas com pesos e um viés para calcular uma saída.
- **Peso** — parâmetro aprendido que multiplica uma entrada.
- **Regressão** — tarefa de prever valores numéricos contínuos, como o gasto de bateria.
- **Taxa de aprendizado** — escala das atualizações dos parâmetros. Um valor muito alto pode tornar o treino instável.
- **Viés (bias)** — parâmetro somado à combinação ponderada das entradas, independentemente dos valores de entrada.

## Texto e dados

- **ASCII** — codificação de caracteres usada nos exemplos de texto; os exemplos de tokenização trabalham com palavras sem acentos.
- **Entrada / saída esperada (alvo)** — os valores fornecidos ao modelo e os valores que se espera que ele aprenda a prever, respectivamente.
- **Normalização** — transformação de valores para uma escala definida. O exemplo de bateria normaliza entradas e alvo; o de anomalia divide o desvio padronizado pelo limite N.
- **PLN (Processamento de Linguagem Natural)** — área que trata dados em linguagem humana. O exemplo classifica frases com um vocabulário explícito; não é um modelo de linguagem geral.
- **Tokenização** — separação de uma frase em palavras para processá-las. No exemplo, palavras fora do vocabulário são ignoradas.
- **Vocabulário** — lista explícita de palavras reconhecidas pelo classificador de texto.
- **Vetor de presença de palavras** — representação numérica em que cada posição corresponde a uma palavra do vocabulário e vale `1` quando ela aparece na frase e `0` caso contrário.
- **Vetor de classes (one-hot)** — representação do rótulo em que a posição da classe correta vale `1` e as demais valem `0`.
- **Média** — soma das medições dividida pela quantidade de medições; no exemplo de anomalia, é calculada a partir das leituras normais de corrente.
- **Desvio-padrão** — medida da dispersão das leituras em torno da média. O exemplo calcula o desvio-padrão amostral das correntes normais.
- **Escore z (desvio padronizado)** — distância entre uma leitura e a média, expressa em unidades de desvio-padrão: `(leitura - média) / desvio-padrão`.
- **Regra N-sigma** — regra que considera anômala, no exemplo, uma corrente acima da média por mais de N desvios-padrão. A classificação aprendida pela Seai é apresentada separadamente dessa regra.
- **CSV** — arquivo de texto com campos separados por vírgulas. No exemplo de armazenamento, a primeira linha nomeia as colunas e `quantidadeEntradas` define onde começam as saídas.
- **LittleFS** — sistema de arquivos usado pelo exemplo para armazenar o CSV na memória flash do microcontrolador.

## C++, conversão e armazenamento

- **`bool`** — tipo lógico usado para indicar, por exemplo, sucesso (`true`) ou falha (`false`).
- **`float`** — tipo numérico de ponto flutuante usado em entradas, parâmetros, perdas e previsões.
- **`std::size_t` / `size_t`** — tipo inteiro sem sinal usado para quantidades, dimensões e índices de contêineres.
- **`std::vector`** — contêiner C++ de tamanho dinâmico usado para armazenar vetores e matrizes de dados.
- **Ponteiro (`*`)** — valor que representa um endereço de memória; na conversão, aponta para os buffers de entrada ou saída.
- **Referência (`&`)** — acesso ao objeto original sem cópia; uma função pode, por referência, preencher ou alterar o objeto recebido.
- **Índice** — posição de um elemento em um vetor. Em C++, o primeiro elemento tem índice zero.
- **Conversão de dados** — transformação entre tipos numéricos e texto. A conversão para inteiros arredonda valores; `INT8`, `INT16` e `UINT8` também limitam o resultado à faixa representável.
- **`FLOAT32`, `INT32`, `INT16`, `INT8`, `UINT8`, `STRING`** — tipos aceitos por `Conversor`. Os inteiros indicam largura e sinal; `STRING` representa texto armazenado em uma largura de bytes informada.
- **Largura de texto** — quantidade de bytes reservados para cada texto de tamanho fixo. `larguraMinima` retorna o tamanho da maior palavra mais um byte para o terminador nulo.
- **Little-endian** — ordem de bytes que armazena primeiro o byte menos significativo. É usada nos campos numéricos do cabeçalho WAV pelo gravador.
- **Cabeçalho CSV** — primeira linha do arquivo, que contém os nomes das colunas. Os nomes não determinam, por si só, a divisão entre entradas e saídas.

## Microcontrolador, áudio e comunicação

- **Arduino IDE** — ambiente usado para abrir, compilar e enviar sketches Arduino à placa.
- **ESP32-S3** — microcontrolador da Espressif usado nos exemplos e no gravador.
- **GPIO** — pino de entrada/saída de uso geral do microcontrolador. O gravador usa GPIOs configuráveis para sinais do microfone e o LED integrado.
- **I2S** — interface digital serial usada para receber áudio do microfone.
- **INMP441** — módulo de microfone digital I2S usado como referência no gravador.
- **BCLK / SCK** — sinal de clock de bits do I2S, que sincroniza a transmissão dos bits.
- **WS (Word Select)** — sinal I2S que indica a seleção do canal de áudio.
- **DIN / SD** — entrada de dados I2S do ESP32-S3, ligada à saída de dados do microfone.
- **L/R** — pino do INMP441 que seleciona o canal do microfone; a ligação documentada no projeto é ao GND.
- **PCM** — representação de áudio por amostras digitais. O gravador envia áudio mono de 16 kHz, com amostras de 16 bits.
- **Taxa de amostragem** — quantidade de amostras de áudio por segundo; no gravador é de 16.000 Hz.
- **WAV** — formato de arquivo de áudio com cabeçalho que descreve, entre outros dados, formato, frequência, canais e tamanho do áudio. O gravador envia o cabeçalho antes dos dados PCM.
- **Porta serial / COM** — canal de comunicação entre o computador e a placa; no Windows, costuma ser identificada como `COM` seguida de um número.
- **Baud rate** — velocidade configurada para a comunicação serial; o gravador inicializa a Serial a 921600 baud.
- **PySerial** — pacote Python usado pelo script do gravador para comunicar-se com a placa pela porta serial.
- **Serial Monitor (Monitor Serial)** — ferramenta da Arduino IDE para visualizar e enviar dados seriais. Deve ser fechado para que o script Python possa usar a mesma porta.
- **Comando `G`** — caractere enviado pela Serial que solicita ao gravador o início de uma captura.
- **USB CDC On Boot** — opção da placa ESP32 na Arduino IDE necessária, conforme o procedimento do gravador, para comunicação USB serial.
- **Cabo USB de dados** — cabo que transporta comunicação além de energia; um cabo somente de carga não permite a comunicação necessária com a placa.
- **Enter** — tecla usada pelo menu do gravador para confirmar uma opção ou enviar um texto solicitado.
- **Rótulo** — palavra informada no gravador para nomear as amostras, por exemplo, `ligar`.
- **LED vermelho/verde** — indicação visual do gravador: vermelho pisca três vezes antes da captura; verde indica que a captura começou. Após o envio, a cor também informa se a duração ficou dentro da margem esperada.
- **Salvar / excluir / ouvir** — ações disponíveis após uma gravação: salvar o WAV no computador, excluí-lo ou reproduzi-lo.
- **Script Python** — programa executado pelo Python. `ferramentas/gravadorVoz/principal.py` conduz o menu de gravação no Windows.
- **Windows** — sistema operacional exigido pela ferramenta de gravação Python documentada neste projeto.

## Referências

- [Site da biblioteca Small Embedded AI](https://small-embedded-ai.netlify.app/)
- [Repositório e exemplos da biblioteca](https://github.com/brunomakeram/Small-Embedded-Ai)
- [README principal](README.md)
- [Documentação do gravador](ferramentas/gravadorVoz/README.md)
- [Documentação oficial da Arduino IDE](https://docs.arduino.cc/software/ide-v2/)
- [Documentação oficial Arduino — bibliotecas](https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-installing-a-library/)
- [Documentação oficial Arduino-ESP32 — instalação](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
- [Documentação Espressif ESP-IDF — I2S](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/i2s.html)
- [Documentação Python — tutorial](https://docs.python.org/3/tutorial/)
- [PySerial — documentação](https://pyserial.readthedocs.io/en/latest/)
