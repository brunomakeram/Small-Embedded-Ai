# Gravador de Voz Maker INMP441

**Versão 1.0.0** · Desenvolvido por Bruno Maker

Grava amostras de voz de 2 segundos com um microfone INMP441 ligado a uma ESP32-S3 e salva cada uma como arquivo WAV no computador. Serve para montar o conjunto de áudios usado no treino de reconhecimento de palavras.

> Funciona somente no **Windows**.

## Requisitos

- Windows 10 ou 11
- Placa ESP32-S3
- Microfone INMP441
- Arduino IDE com o pacote de placas **esp32 (Espressif Systems) versão 3.0 ou superior**
- Cabo USB que transmita dados (alguns cabos só carregam)

## Instalação no Windows

### 1. Instalar o Python

Pelo terminal (PowerShell):

```
winget install Python.Python.3.12
```

Ou baixe em https://www.python.org/downloads/ e marque a opção **Add python.exe to PATH** durante a instalação.

Feche e abra o terminal de novo e confira:

```
python --version ou py --version
```

### 2. Instalar o PySerial

O PySerial é a biblioteca que conversa com a porta COM da placa.

```
pip install pyserial
```

Se o terminal disser que o `pip` não foi encontrado, use:

```
py -m pip install pyserial

```
## Ver a versão:

py -m pip show pyserial

## Ligação do microfone

Siga o nome impresso em cada pino. A ordem pode mudar de um módulo INMP441 para outro. Abaixo está na ordem do módulo usado neste projeto:

| INMP441 | ESP32-S3 |
|---------|----------|
| SCK     | GPIO 4   |
| WS      | GPIO 5   |
| SD      | GPIO 9   |
| L/R     | GND      |
| GND     | GND      |
| VDD     | 3V3      |

O pino **L/R precisa estar no GND**. Se estiver no 3V3, a gravação sai em silêncio.

Se a sua placa não tiver o GPIO 9 exposto, escolha outro pino livre e configure o pino de dados ao criar a instância da classe `Gravador`.

## Uso da biblioteca C++

O exemplo `examples/Gravador/Gravador.ino` usa os pinos padrão e grava 2 segundos:

```cpp
#include <Arduino.h>
#include <Gravador/Gravador.h>

Gravador::Gravador gravador;

void setup() {
    gravador.iniciar();
}

void loop() {
    gravador.gravar();
}
```

O construtor aceita o tempo de gravação em segundos e os pinos BCLK, WS e DIN,
nesta ordem. Os padrões são 2 segundos e GPIO 4, 5 e 9:

```cpp
Gravador::Gravador gravadorPersonalizado(5, 14, 15, 16);
```

O tempo é limitado a 10 segundos; valores menores que 1 segundo são ajustados
para 1 segundo. O envio continua sendo um cabeçalho WAV seguido pelos dados PCM.
O gravador Python incluído espera o padrão de 2 segundos; para usar outra
duração, o programa que recebe a Serial deve ler o tamanho dos dados no cabeçalho
WAV.

## Passo a passo: código da placa (Arduino IDE)

1. Abra o exemplo `examples/Gravador/Gravador.ino` na Arduino IDE.
2. Em **Ferramentas > Placa**, escolha **ESP32S3 Dev Module**.
3. Em **Ferramentas > USB CDC On Boot**, escolha **Enabled**. Sem isso, a placa não envia o áudio pelo cabo USB e o gravador não funciona.
4. Em **Ferramentas > Porta**, escolha a porta COM da placa (exemplo: COM7).
5. Clique em **Carregar** e espere terminar.
6. **Feche o Monitor Serial** da Arduino IDE. Se ele ficar aberto, o script não consegue usar a porta.

Quando a placa recebe a letra `G` pela Serial, o LED pisca vermelho 3 vezes, fica **verde** e grava 2 segundos. Depois envia o WAV pelo cabo.

## Passo a passo: gravador (Python)

1. Abra o terminal na pasta `Gravador` e rode:

   ```
   python principal.py
   ```

2. Digite `01` (Gravar amostra).
3. Escolha a porta. Digite o **número da posição na lista**, e não o número da COM. Exemplo: se aparece `[1] COM7`, digite `1`.
4. Digite a palavra que será gravada (exemplo: `ligar`).
5. Aperte **Enter** para gravar.
6. Espere o LED da placa piscar 3 vezes e ficar **verde**. Só então fale a palavra.
7. Terminada a gravação, aparecem 3 opções:

   | Tecla | Ação |
   |-------|------|
   | O | Ouvir a gravação (quantas vezes quiser) |
   | S | Salvar e voltar ao menu |
   | E | Excluir e voltar ao menu |

Os arquivos ficam na pasta `Audios`, numerados por palavra: `ligar_001.wav`, `ligar_002.wav`...

### Menu principal

| Opção | Função |
|-------|--------|
| 01 | Gravar amostra |
| 02 | Trocar palavra |
| 03 | Modo de pasta: única (`ligar_001.wav`) ou por palavra (`ligar/ligar_001.wav`) |
| 04 | Trocar porta COM |
| 05 | Contar gravações de cada palavra |
| 06 | Executar: fecha e abre o script de novo |
| 00 | Sair |

Em qualquer pergunta, **C** cancela e **V** volta uma pergunta.

## Estrutura do projeto

```
Gravador/
  Audios/            gravações em WAV
  INMP441/
    INMP441.ino      código da placa
  principal.py       arquivo que você roda
  config.py          constantes e estado
  tela.py            menu e perguntas na tela
  etapas.py          perguntas passo a passo
  placa.py           conversa com a ESP32
  arquivos.py        salvar, contar e excluir WAV
  pos_gravacao.py    menu de ouvir, salvar ou excluir
```

## Créditos

Desenvolvido por **Bruno Maker**.

- GitHub: https://github.com/brunomakeram
- Perfil no XERJ Corpus Hub: https://hub.xerj.org/contributors/brunomakeram
- Biblioteca Maker (treino de IA direto na ESP32, em C++): https://biblioteca-treino-ia-embarcados.netlify.app/

© 2026 Bruno Maker. Todos os direitos reservados.