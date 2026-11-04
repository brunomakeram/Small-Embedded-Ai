# Small Embedded AI (Seai)

Biblioteca C++ para treinar e executar modelos em placas compatíveis com
Arduino. A Seai oferece redes neurais multicamada, conversão de dados,
persistência CSV em LittleFS e captura de áudio I2S.

**Site da biblioteca:** https://small-embedded-ai.netlify.app/

## Requisitos

- Arduino IDE e a biblioteca Seai instalada.
- Para os exemplos abaixo: ESP32-S3 e Arduino-ESP32 core 3.x.
- Os 5 exemplos de IA desta página foram validados na ESP32-S3.
- O gravador de áudio também requer microfone INMP441 compatível.

## O que a Seai pode fazer

Os exemplos demonstram classificação de palavras/textos, classificação de
sentimentos, regressão linear para estimar consumo de bateria e análise de
anomalias pela corrente do motor. São exemplos compactos para execução
embarcada; a qualidade depende dos dados coletados e da representatividade das
amostras. Os classificadores de texto usam um vocabulário explícito, não um
modelo de linguagem geral.

## Exemplos testados na ESP32-S3

Abra cada pasta como sketch independente na Arduino IDE. As referências abaixo
apontam para imagens que poderão ser adicionadas à pasta `imagens/` do GitHub;
o código executável fica nos links `.ino`.

### Previsão de bateria — regressão linear

![Exemplo PrevisaoBateria](imagens/PrevisaoBateria.jpg)

[Abrir o sketch PrevisaoBateria.ino](examples/PrevisaoBateria/PrevisaoBateria.ino)

**Como montar os dados:** registre várias viagens. Cada amostra contém carga
transportada em kg, distância percorrida em km e percentual de bateria gasto
naquela viagem. Substitua as linhas sintéticas de `dadosFisicos` por medições
reais. Mantenha o método de medição do percentual consistente e inclua viagens
com cargas e distâncias variadas. O exemplo normaliza as duas entradas e o alvo
antes do treino.

### PLN — classificação de palavras

![Exemplo ClassificacaoPLN](imagens/ClassificacaoPLN.jpg)

[Abrir o sketch ClassificacaoPLN.ino](examples/ClassificacaoPLN/ClassificacaoPLN.ino)

**Como montar os dados:** adicione frases representativas a `frasesTreino` e
um rótulo correspondente em `classesTreino`: `0` para remédio, `1` para resgate
e `2` para mercadoria. Inclua quantidade e variedade semelhantes por classe.
Adicione ao `vocabulario` palavras importantes para o seu domínio. O sketch
transforma cada frase num vetor de presença de palavras usando `Conversor`.
Palavras fora do vocabulário são ignoradas; o exemplo usa texto ASCII sem
acentos.

### Classificação de sentimentos

![Exemplo ClassificacaoSentimentos](imagens/ClassificacaoSentimentos.jpg)

[Abrir o sketch ClassificacaoSentimentos.ino](examples/ClassificacaoSentimentos/ClassificacaoSentimentos.ino)

**Como montar os dados:** cada frase em `frasesTreino` precisa do rótulo na
mesma posição em `classesTreino`: `0` para positivo, `1` para negativo e `2`
para neutro. Use frases reais do seu contexto, incluindo variações de escrita,
frases neutras e exemplos equilibrados entre classes. Amplie o `vocabulario`
com palavras relevantes. A tokenização de demonstração usa ASCII sem acentos e
ignora palavras que não estejam no vocabulário.

### Anomalia de hélice — análise da corrente do motor

![Exemplo AnomaliaHelice](imagens/AnomaliaHelice.jpg)

[Abrir o sketch AnomaliaHelice.ino](examples/AnomaliaHelice/AnomaliaHelice.ino)

**Como montar os dados:** primeiro colete uma série de leituras de corrente
durante operação normal e preencha `correnteNormal`; o sketch calcula a média
e o desvio-padrão dessa referência. Depois ajuste `correnteAtual` com a leitura
que será avaliada e `NUMERO_DESVIOS_ANOMALIA` com o limite N desejado. Para
treinar com outros motores, substitua `desviosNormalizados` e
`rotulosAnomalia` por observações rotuladas: cada entrada é o desvio da corrente
em relação à média, dividido pelo desvio-padrão e por N; use saída `0` para
normal e `1` para anomalia. A regra direta N-sigma determina se a leitura está
acima do limite; a classificação da Seai é uma estimativa aprendida adicional.

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
    {1.0f},
    {1.0f},
    {0.0f}
};

void setup()
{
    Serial.begin(115200);
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

## Dados de treino em CSV (LittleFS)

`Armazenamento::Armazenamento` lê e grava matrizes
`std::vector<std::vector<float>>`. No CSV, a primeira linha contém os nomes das
colunas. O argumento `quantidadeEntradas` de `lerDados` informa quantas
colunas iniciais são entradas; as colunas restantes são saídas. Os nomes são
livres, mas cada linha deve ter o mesmo número de valores do cabeçalho.

## Adam e conversão

`Adam::Adam::atualizarParametros` recebe vetores de parâmetros e gradientes,
permitindo qualquer quantidade de valores. A atualização calcula a média dos
gradientes sem modificar o vetor de entrada.

`Conversor::Conversor` converte valores entre `FLOAT32`, inteiros e `STRING`.
Para campos de texto de largura fixa, informe `larguraEntrada` e
`larguraSaida` a `converterDado`. `larguraMinima` calcula a largura necessária
para armazenar a maior palavra de uma lista.

## Gravador de áudio

Além de treinar e executar modelos embarcados, a Seai pode se comunicar pela
porta serial com um script Python no notebook para gravar áudio na placa e
transferir as gravações para o computador. Pelo menu do script, é possível
ouvir as gravações, salvá-las no notebook ou excluí-las.

O fluxo corresponde ao sketch
`examples/GravadorExemplo/GravadorExemplo.ino` e ao script
`ferramentas/gravadorVoz/principal.py`. Requer placa e microfone compatíveis,
script e dependências instalados, além da porta serial da placa.

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
