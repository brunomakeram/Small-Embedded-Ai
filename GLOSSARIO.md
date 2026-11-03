# Glossário — Small Embedded AI

Este glossário explica os termos técnicos usados no README, nos exemplos C++ e no procedimento do gravador. As descrições acompanham as APIs atualmente documentadas; consulte também as fontes oficiais ao final.

## Biblioteca, rede e treinamento

- **Amostra** — observação individual de um conjunto de dados, com características de entrada e o alvo associado.
- **Adam** — otimizador adaptativo que atualiza parâmetros usando médias móveis dos gradientes e de seus quadrados. Nesta biblioteca é usado para pesos e vieses.
- **Ativação / `tanh`** — transformação aplicada à soma ponderada de entradas. A rede usa `tanh` em camadas ocultas; a camada final é linear para regressão.
- **Backpropagation (retropropagação)** — cálculo das derivadas do erro através das camadas para determinar como ajustar parâmetros.
- **Camada** — conjunto de neurônios que recebe um vetor de valores e calcula suas ativações. Camadas sucessivas encadeiam a saída anterior como entrada seguinte.
- **Característica** — variável de entrada do modelo; nos exemplos tradicionais, carga e distância.
- **Dataset** — conjunto de amostras. Em `Dataset::Dataset`, `carga`, `distancia` e `gasto` são vetores paralelos: a posição correspondente em cada vetor pertence à mesma amostra.
- **Época** — passagem completa do algoritmo por todas as amostras de treinamento.
- **Épsilon (`eps`, `epsilon`)** — pequena constante de estabilidade numérica adicionada ao denominador do Adam para evitar divisão instável por valor próximo de zero.
- **Gradiente** — derivada que indica como uma alteração de parâmetro afeta a perda.
- **Hiperparâmetro** — configuração do processo de treino, como taxa de aprendizado, épocas, tamanho de lote ou quantidade de neurônios.
- **Inferência / previsão** — cálculo da saída do modelo para uma entrada, após ou sem treinamento conforme o componente utilizado.
- **Lote (batch)** — subconjunto de amostras cujos gradientes são acumulados antes de atualizar os parâmetros.
- **Matriz de entradas / saídas esperadas** — vetor de vetores em que cada linha corresponde a uma amostra e cada coluna a uma característica ou saída.
- **MSE (Mean Squared Error, erro quadrático médio)** — média dos erros ao quadrado; métrica de perda usada para acompanhar a diferença entre saída prevista e alvo.
- **Neurônio** — unidade que combina entradas com pesos e viés e calcula uma saída.
- **Peso** — coeficiente aprendido que multiplica uma característica de entrada.
- **Taxa de aprendizado** — escala do ajuste aplicado aos parâmetros em cada atualização. Taxa excessiva pode tornar o treino instável.
- **Viés (bias/intercepto)** — parâmetro adicionado à soma ponderada, deslocando o resultado independentemente das entradas.

## Código e tipos de dados

- **`bool`** — tipo de retorno que indica verdadeiro/sucesso ou falso/falha.
- **`float`** — tipo numérico de ponto flutuante usado em entradas, pesos, perdas e previsões.
- **`std::size_t`** — tipo inteiro sem sinal adequado a dimensões e posições de contêineres.
- **`std::vector`** — contêiner de tamanho dinâmico da biblioteca padrão C++, usado para dados e camadas.
- **Ponteiro (`*`)** — endereço de um elemento ou buffer. Nas conversões e gradientes, indica onde ler ou gravar os valores.
- **Referência (`&`)** — acesso ao objeto original sem cópia; parâmetros de saída por referência podem ser modificados pela função.
- **Índice** — posição de uma amostra ou camada dentro de um vetor. A primeira posição C++ é zero; no treinamento, o índice zero identifica a primeira amostra.
- **Conversão numérica** — tradução entre representações de dados como `FLOAT32`, inteiros com/sem sinal e texto. Inteiros de destino podem ser arredondados e limitados à faixa representável.
- **`FLOAT32`, `INT32`, `INT16`, `INT8`, `UINT8`, `STRING`** — identificadores de tipos aceitos por `Conversor::Conversor`. `UINT8` é inteiro sem sinal de oito bits; `STRING` é texto de largura informada.
- **Largura de texto** — quantidade de bytes reservados para cada elemento textual de tamanho fixo. `larguraMinima` retorna a maior palavra mais um byte para terminador nulo.
- **Little-endian** — ordem de bytes na qual o byte menos significativo é armazenado primeiro; usado em cabeçalhos WAV e valores binários em plataformas compatíveis.

## Microcontrolador, gravação e arquivos

- **Arduino IDE** — ambiente oficial para abrir, compilar e enviar sketches Arduino.
- **Cabo USB de dados** — cabo que transporta comunicação, além de energia; cabos somente de carga não permitem programar ou comunicar com a placa.
- **COM / porta serial** — interface de comunicação que o computador disponibiliza para conversar com a placa. O número/identificador pode variar por conexão.
- **Enter** — tecla que confirma uma opção ou envia texto digitado no terminal do gravador.
- **ESP32-S3** — microcontrolador da Espressif usado no exemplo de gravação; a placa escolhida no IDE precisa corresponder ao hardware conectado.
- **GPIO** — pino de entrada/saída de uso geral do microcontrolador, configurado no exemplo para sinais do microfone e LED.
- **I2S** — barramento digital serial para áudio. A classe `Gravador` recebe amostras do microfone através do driver I2S.
- **INMP441** — módulo de microfone digital I2S usado como hardware de referência do gravador.
- **BCLK** — bit clock do I2S, sinal que sincroniza bits transmitidos/recebidos.
- **WS (Word Select)** — sinal de seleção de canal/quadro I2S, também chamado de clock de palavra.
- **DIN** — entrada de dados do canal I2S de recepção no ESP32, ligada à saída de dados do microfone.
- **Serial Monitor** — ferramenta da Arduino IDE que exibe e envia dados pela porta serial. Deve ser fechado antes de executar o script que usa a mesma porta.
- **Script Python** — programa executável interpretado pelo Python; neste repositório, `ferramentas/gravadorVoz/principal.py` conduz o menu de gravação.
- **PySerial** — pacote Python para comunicação com portas seriais.
- **Rótulo** — palavra informada pelo usuário para nomear/classificar uma gravação, por exemplo `ligar`.
- **WAV** — formato de arquivo de áudio que começa com cabeçalho descrevendo formato, frequência, canais e tamanho dos dados.
- **PCM** — representação digital de áudio por amostras. O gravador transmite áudio PCM mono em amostras de 16 bits dentro de um WAV.
- **LittleFS** — sistema de arquivos leve usado em memória flash de microcontroladores ESP para armazenar, por exemplo, um CSV.
- **CSV** — arquivo de texto com campos separados por vírgula; o arquivo de dados do exemplo contém `Carga`, `Distância` e `Gasto`.
- **LED vermelho/verde** — sinal visual do gravador: vermelho indica aviso/pisca antes de capturar; verde indica o estado de gravação descrito no fluxo.
- **Salvar / excluir / ouvir** — ações do menu após captura. Salvar mantém a amostra e retorna ao menu; excluir remove a gravação; ouvir permite reprodução conforme a ferramenta e o ambiente configurados.

## Referências oficiais e do projeto

- [Site Small Embedded AI](https://small-embedded-ai.netlify.app/)
- [Repositório Small-Embedded-Ai](https://github.com/brunomakeram/Small-Embedded-Ai)
- [Arduino IDE 2 — documentação oficial](https://docs.arduino.cc/software/ide-v2/)
- [Instalar bibliotecas na Arduino IDE — documentação oficial](https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-installing-a-library/)
- [Arduino-ESP32 — instalação oficial](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
- [ESP-IDF — referência oficial de I2S](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/i2s.html)
- [Python — tutorial oficial](https://docs.python.org/3/tutorial/)
- [PySerial — documentação](https://pyserial.readthedocs.io/en/latest/)
- [README principal e procedimento completo do gravador](README.md#gravador-de-áudio)
