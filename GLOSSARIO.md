# Glossário — Small Embedded AI

Este glossário explica os termos técnicos usados na documentação e nos exemplos da Seai. As definições refletem a versão 4.0.0 (V4) e o comportamento documentado no projeto.

## Inteligência artificial e treinamento

- **Amostra** — observação individual usada no treino ou na avaliação. Cada amostra contém valores de entrada e os valores de saída esperados correspondentes.
- **Adam** — otimizador que ajusta pesos e vieses usando médias móveis dos gradientes.
- **Ativação / `tanh`** — transformação aplicada aos valores de uma camada. A rede usa `tanh` nas camadas ocultas e uma saída linear.
- **Backpropagation (retropropagação)** — cálculo do efeito do erro nas camadas anteriores para obter os gradientes dos parâmetros.
- **Camada** — conjunto de neurônios que transforma um vetor de entrada em outro vetor. A saída de uma camada alimenta a seguinte.
- **Característica (feature)** — valor de entrada que descreve uma amostra, como carga, distância ou presença de uma palavra.
- **Classificação** — tarefa que associa uma entrada a uma categoria, como sentimento positivo, negativo ou neutro. Nos exemplos, a classe escolhida é a de maior valor previsto.
- **Conjunto de dados (dataset)** — coleção de amostras usadas no treino ou na avaliação. Nas matrizes de treino, cada linha de `entradas` corresponde à linha de mesmo índice em `saidasEsperadas`. Na V4, os exemplos declaram explicitamente seus dados; não há dataset padrão `DATASET_PADRAO`.
- **Época** — passagem completa por todas as amostras de treinamento.
- **Gradiente** — derivada que indica como uma alteração em um parâmetro afeta a perda.
- **Hiperparâmetro** — configuração definida antes de executar o treino, como taxa de aprendizado, quantidade de épocas, tamanho do lote e neurônios por camada.
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
- **Regra N-sigma** — regra que considera anômala, no exemplo, uma corrente acima da média por mais de N desvios-padrão. A classificação aprendida pela Seai é apresentada separadamente dessa regra direta.
- **CSV** — arquivo de texto com campos separados por vírgulas. No módulo de armazenamento, a primeira linha nomeia as colunas e `quantidadeEntradas` define onde começam as saídas.
- **Cabeçalho CSV** — primeira linha do arquivo, que contém os nomes das colunas. Os nomes não determinam, por si só, a divisão entre entradas e saídas.
- **Armazenamento opt-in** — comportamento da V4 em que a biblioteca não cria nem grava datasets automaticamente. O programa precisa chamar explicitamente as operações de `Seai::Armazenamento` para ler ou gravar CSV.
- **LittleFS** — sistema de arquivos usado para operações de CSV e persistência de pesos na memória flash do ESP32. Montar o sistema de arquivos não cria dataset por padrão.
- **EEPROM** — memória não volátil usada para persistir pesos do modelo em configurações compatíveis com RP2040.

## Modelo e transferência serial (V4)

- **Namespace `Seai`** — escopo C++ usado pelas APIs públicas da biblioteca na V4; por exemplo, `Seai::Modelo` e `Seai::Armazenamento`.
- **Exportar modelo (`exportar()`)** — operação explícita que envia pela Serial os dados de um modelo treinado no formato CSV `SEAI_PESOS`.
- **Importar modelo (`importar()`)** — operação explícita que recebe pela Serial um modelo, valida os dados e prepara as camadas para inferência; a operação também persiste os pesos na memória compatível da placa.
- **`SEAI_PESOS`** — formato CSV versão 1 usado para transferir os pesos do modelo entre ESP32 e RP2040.
- **`SEAI_INICIO` / `SEAI_FIM`** — marcadores que delimitam o conteúdo do modelo transferido pela Serial.
- **Persistência de pesos** — gravação dos parâmetros do modelo em memória não volátil para que possam ser recuperados posteriormente. Na V4, utiliza LittleFS no ESP32 e EEPROM no RP2040, conforme o suporte da placa.
- **`ferramentas/seai.py`** — ferramenta Python de transferência de modelos. O comando `receber` salva o CSV recebido no computador; `enviar` transmite um arquivo CSV para a placa. A transferência ocorre quando solicitada, não automaticamente.
- **PySerial** — pacote Python utilizado pela ferramenta para comunicar-se com a placa pela porta serial. Pode ser instalado com `python -m pip install pyserial`.
- **Porta serial / COM** — canal de comunicação entre o computador e a placa; no Windows, costuma ser identificada como `COM` seguida de um número. A opção `--porta` permite indicar a porta diretamente às ferramentas compatíveis.

## C++, conversão e armazenamento

- **`bool`** — tipo lógico usado para indicar, por exemplo, sucesso (`true`) ou falha (`false`).
- **`float`** — tipo numérico de ponto flutuante usado em entradas, parâmetros, perdas e previsões.
- **`std::size_t` / `size_t`** — tipo inteiro sem sinal usado para quantidades, dimensões e índices de contêineres.
- **`std::vector`** — contêiner C++ de tamanho dinâmico usado para armazenar vetores e matrizes de dados.
- **Ponteiro (`*`)** — valor que representa um endereço de memória; na conversão, aponta para os buffers de entrada ou saída.
- **Referência (`&`)** — acesso ao objeto original sem cópia; uma função pode, por referência, preencher ou alterar o objeto recebido.
- **Índice** — posição de um elemento em um vetor. Em C++, o primeiro elemento tem índice zero.
- **Conversão de dados** — transformação entre tipos numéricos e texto. A conversão para inteiros arredonda valores; `INT8`, `INT16` e `UINT8` limitam o resultado à faixa representável pelo tipo.
- **`FLOAT32`, `INT32`, `INT16`, `INT8`, `UINT8`, `STRING`** — tipos aceitos por `Seai::Conversor`. Os inteiros indicam largura e sinal; `STRING` representa texto armazenado em uma largura fixa de bytes.
- **Largura de texto** — quantidade de bytes reservados para cada texto de tamanho fixo. `larguraMinima` retorna o tamanho da maior palavra mais um byte para o terminador nulo.
- **Little-endian** — ordem de bytes que armazena primeiro o byte menos significativo. É usada nos campos numéricos do cabeçalho WAV pelo gravador.

## Microcontrolador, áudio e comunicação

- **Arduino IDE** — ambiente usado para abrir, compilar e enviar sketches Arduino à placa.
- **ESP32-S3** — microcontrolador da Espressif usado nos exemplos e no gravador.
- **RP2040 / Raspberry Pi Pico** — família de microcontroladores e placa compatível com a biblioteca; a V4 oferece persistência de pesos em EEPROM em configurações compatíveis.
- **AVR** — arquitetura de microcontroladores que não é suportada pela Seai, incluindo Arduino Uno.
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
- **Baud rate** — velocidade configurada para a comunica��ão serial; o gravador inicializa a Serial a 921600 baud.
- **Serial Monitor (Monitor Serial)** — ferramenta da Arduino IDE para visualizar e enviar dados seriais. Deve ser fechado para que um script Python possa usar a mesma porta.
- **Comando `G`** — caractere enviado pela Serial que solicita ao gravador o início de uma captura.
- **USB CDC On Boot** — opção da placa ESP32 na Arduino IDE necessária, conforme o procedimento do gravador, para comunicação USB serial.
- **Cabo USB de dados** — cabo que transporta comunicação além de energia; um cabo somente de carga não permite a comunicação necessária com a placa.
- **Rótulo** — palavra informada no gravador para nomear as amostras, por exemplo, `ligar`.
- **LED vermelho/verde** — indicação visual do gravador: vermelho pisca três vezes antes da captura; verde indica que a captura começou.
- **Salvar / excluir / ouvir** — ações disponíveis após uma gravação: salvar o WAV no computador, excluí-lo ou reproduzi-lo.
- **Script Python** — programa executado pelo Python. `ferramentas/gravadorVoz/principal.py` conduz o menu de gravação documentado para Windows.
- **Windows** — sistema operacional exigido pela ferramenta de gravação Python documentada neste projeto.

## Referências

- [Site da biblioteca Small Embedded AI](https://small-embedded-ai.netlify.app/)
- [Repositório e exemplos da biblioteca](https://github.com/brunomakeram/Small-Embedded-Ai)
- [README principal](README.md)
- [README em inglês](README.en.md)
- [Documentação do gravador](ferramentas/gravadorVoz/README.md)
- [Documentação oficial da Arduino IDE](https://docs.arduino.cc/software/ide-v2/)
- [Documentação oficial Arduino — bibliotecas](https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-installing-a-library/)
- [Documentação oficial Arduino-ESP32 — instalação](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
- [Documentação Espressif ESP-IDF — I2S](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/i2s.html)
- [Documentação Python — tutorial](https://docs.python.org/3/tutorial/)
- [PySerial — documentação](https://pyserial.readthedocs.io/en/latest/)
