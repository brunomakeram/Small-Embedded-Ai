#pragma once
// Includes: stdint (uint8_t, int32_t), stddef (size_t), string (memcpy, memset, strlen),
// stdlib (atof), stdio (snprintf), math (roundf). No Arduino, o Arduino.h já traz quase tudo.
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

namespace Conversor
{
    // Classe que representa o tipo de dado usado pela biblioteca (float, int, string).
    class Conversor
    {

        // public: tudo abaixo pode ser acessado de fora da classe, tipo TipoDado::INT8.
    public:
        // static: a constante pertence à classe, existe uma cópia só (não uma por objeto).
        // constexpr: o valor é resolvido na compilação e não ocupa RAM.
        // uint8_t: inteiro sem sinal de 1 byte (0 a 255).

        // Decimal de 32 bits (4 bytes), o padrão. Valor 0.
        static constexpr uint8_t FLOAT32 = 0;
        // Inteiro de 32 bits (4 bytes). Valor 1.
        static constexpr uint8_t INT32 = 1;
        // Inteiro de 16 bits (2 bytes). Valor 2.
        static constexpr uint8_t INT16 = 2;
        // Inteiro de 8 bits (1 byte), o mais econômico. Valor 3.
        static constexpr uint8_t INT8 = 3;
        // Inteiro sem sinal de 8 bits (1 byte), de 0 a 255. Valor 4.
        static constexpr uint8_t UINT8 = 4;
        // Texto de tamanho variável, a largura em bytes é informada por quem chama. Valor 5.
        static constexpr uint8_t STRING = 5;

        // TETO_BYTES_TEXTO: máximo de bytes que o texto pode ocupar (quantidade x largura do texto).
        // Protege a RAM: acima disso a converterDado recusa em vez de estourar a memória.
        // Ex.: 100000 bytes comportam 5000 palavras de largura 20. Pode ser trocado por quem usa a biblioteca.
        static constexpr size_t TETO_BYTES_TEXTO = 100000;

        // Construtor: recebe um dos valores acima (0 a 5) e valida antes de guardar.
        // Sem os dois pontos: a cópia agora é feita aqui dentro, depois da checagem.
        Conversor(uint8_t tipoDadoEscolhido)
        {
            // Se for maior que STRING (5), não existe tipo com esse número.
            if (tipoDadoEscolhido > STRING)
            {
                // Usa FLOAT32, o tipo padrão da biblioteca.
                tipoDadoConvertido = FLOAT32;
            }
            else
            {
                // Valor válido: guarda o que a pessoa escolheu.
                tipoDadoConvertido = tipoDadoEscolhido;
            }
        }

        // tamanhoBytes: devolve quantos bytes cada valor desse tipo ocupa na memória.
        // uint8_t: o retorno é um número pequeno (0, 1, 2 ou 4), cabe em 1 byte.
        // const: a função só lê o tipo, não altera nada no objeto.
        uint8_t tamanhoBytes() const
        {
            // switch: compara tipoDadoConvertido com cada constante.
            switch (tipoDadoConvertido)
            {
            // 4 bytes: FLOAT32 e INT32 (empilhados, dividem o mesmo return).
            case FLOAT32:
            case INT32:
                return 4;
            // 2 bytes: INT16.
            case INT16:
                return 2;
            // 1 byte: INT8 e UINT8.
            case INT8:
            case UINT8:
                return 1;
            // default: STRING devolve 0, porque o tamanho do texto não é fixo.
            // Quem chama converterDado informa as larguras (larguraEntrada e larguraSaida). Valor desconhecido também devolve 0.
            default:
                return 0;
            }
        }

        // converterDado: converte valores do tipo deste objeto (origem) para o tipo destino.
        // bool: devolve verdadeiro se converteu, falso se não conseguiu.
        // destino: o tipo pra onde converter (TipoDado::INT8, etc.).
        // entrada: onde estão os valores de origem. saida: onde vão os convertidos.
        // quantidade: quantos valores converter.
        // larguraEntrada: quantos bytes cada texto ocupa na entrada, quando a origem (este objeto) for STRING.
        // Ex.: 10 lê a entrada de 10 em 10 bytes. Nos outros tipos é ignorado.
        // larguraSaida: quantos bytes cada texto ocupa na saída, quando o destino for STRING.
        // Pra palavra que vai caber inteira, use TipoDado::larguraMinima. Nos outros tipos é ignorado.
        // Ex.: larguraSaida = 8 reserva 8 bytes por valor, e "23.6" ocupa 4 e completa com 4 zeros.
        // As duas larguras são separadas: a entrada pode ter 10 bytes por palavra e a saída 9.
        // O = 0 deixa os parâmetros opcionais: quem converte só números chama sem passar nada.
        bool converterDado(uint8_t destino, const uint8_t *entrada, uint8_t *saida, size_t quantidade, size_t larguraEntrada = 0, size_t larguraSaida = 0) const
        {
            // Destino maior que STRING (5) não existe, então recusa.
            if (destino > STRING)
            {
                return false;
            }

            // Teto de memória da entrada: só vale quando a origem é STRING.
            // A divisão evita o estouro que a multiplicação quantidade x larguraEntrada poderia causar.
            // Ex.: larguraEntrada 20 aceita até 100000 / 20 = 5000 valores. Com 5001, recusa.
            if (tipoDadoConvertido == STRING && larguraEntrada > 0)
            {
                if (quantidade > TETO_BYTES_TEXTO / larguraEntrada)
                {
                    return false;
                }
            }

            // Teto de memória da saída: só vale quando o destino é STRING. Mesma conta, com larguraSaida.
            if (destino == STRING && larguraSaida > 0)
            {
                if (quantidade > TETO_BYTES_TEXTO / larguraSaida)
                {
                    return false;
                }
            }

            // tamanhoOrigem: quantos bytes cada valor ocupa na entrada.
            // Se o objeto é STRING, o tamanho é a larguraEntrada (o tamanhoBytes devolveria 0).
            // Nos outros tipos vem do tamanhoBytes. Ex.: UINT8 devolve 1, STRING com larguraEntrada 10 devolve 10.
            size_t tamanhoOrigem = (tipoDadoConvertido == STRING) ? larguraEntrada : tamanhoBytes();

            // tamanhoDestino: quantos bytes cada valor ocupa na saída.
            // Se o destino é STRING, o tamanho é a larguraSaida. Nos outros vem do tipoDestino.
            // Ex.: destino INT16 devolve 2, destino STRING com larguraSaida 9 devolve 9.
            size_t tamanhoDestino = (destino == STRING) ? larguraSaida : tipoDestino(destino);

            // for: repete uma vez por valor, de 0 até quantidade - 1.
            // posicaoValor: posição do valor que está sendo convertido na volta atual (0 é o primeiro).
            // size_t: o tipo próprio pra tamanhos e posições.
            for (size_t posicaoValor = 0; posicaoValor < quantidade; posicaoValor++)
            {
                // valorAtual: endereço do valor número posicaoValor dentro da entrada.
                // posicaoValor * tamanhoOrigem: pula esses valores, cada um com tamanhoOrigem bytes.
                const uint8_t *valorAtual = entrada + posicaoValor * tamanhoOrigem;

                // valorSaida: endereço onde o valor convertido vai ser escrito na saída.
                // Sem const, porque aqui a gente escreve, e não só lê.
                uint8_t *valorSaida = saida + posicaoValor * tamanhoDestino;

                // STRING para STRING: o texto passa inteiro, sem virar número no meio do caminho.
                // Se passasse pelo float, a palavra "ligar" viraria 0 no atof e o texto se perderia.
                // Ex.: larguraEntrada 10 e larguraSaida 9 copia 9 bytes de cada palavra.
                // continue: pula pro próximo valor do for, sem passar pelos dois switches abaixo.
                if (tipoDadoConvertido == STRING && destino == STRING)
                {
                    // copiarBytes: quantos bytes da palavra cabem na saída.
                    // Se a saída for menor que a entrada, o que passar é cortado.
                    size_t copiarBytes = (larguraEntrada < larguraSaida) ? larguraEntrada : larguraSaida;

                    // memset: zera os larguraSaida bytes deste valor, pra o que sobrar ficar com zeros.
                    memset(valorSaida, 0, larguraSaida);

                    // memcpy: copia copiarBytes bytes da palavra na entrada pra saída.
                    memcpy(valorSaida, valorAtual, copiarBytes);
                    continue;
                }

                // valorLido: o valor da entrada como número decimal (float), pra poder converter pra qualquer tipo.
                float valorLido = 0.0f;

                // switch: lê os bytes de valorAtual de acordo com o tipo de origem do objeto.
                // memcpy: copia bytes de um lugar pro outro, sem problema de alinhamento de memória.
                switch (tipoDadoConvertido)
                {
                // FLOAT32: copia os 4 bytes direto pro float.
                case FLOAT32:
                    memcpy(&valorLido, valorAtual, 4);
                    break;
                // INT32: copia os 4 bytes pra um inteiro com sinal e converte pra float.
                // As chaves { } existem porque o case declara uma variável.
                case INT32:
                {
                    int32_t inteiro32;
                    memcpy(&inteiro32, valorAtual, 4);
                    valorLido = (float)inteiro32;
                    break;
                }
                // INT16: igual, com 2 bytes.
                case INT16:
                {
                    int16_t inteiro16;
                    memcpy(&inteiro16, valorAtual, 2);
                    valorLido = (float)inteiro16;
                    break;
                }
                // INT8: o byte é lido como inteiro com sinal (-128 a 127).
                case INT8:
                    valorLido = (float)(int8_t)valorAtual[0];
                    break;
                // UINT8: o byte é lido sem sinal (0 a 255).
                case UINT8:
                    valorLido = (float)valorAtual[0];
                    break;
                // STRING: o valor na entrada é um texto, e ele vira número (ex.: "23.6" vira 23.6).
                case STRING:
                {
                    // textoLido: cópia do texto em um espaço seu, com 1 byte a mais pro fim.
                    // Precisa da cópia porque o texto na entrada pode ocupar a largura inteira,
                    // sem sobrar um zero no final, e o atof só sabe onde parar quando acha o zero.
                    // Tamanho fixo de 32 bytes: um número escrito não precisa de mais que isso.
                    char textoLido[32];

                    // copiarBytes: quantos bytes do texto entram na cópia.
                    // Se a larguraEntrada for maior que 31, corta em 31 pra deixar lugar pro zero do fim.
                    // Ex.: larguraEntrada 8 copia 8 bytes. larguraEntrada 500 copia só 31.
                    size_t copiarBytes = (larguraEntrada < 31) ? larguraEntrada : 31;

                    // memcpy: copia os bytes do texto da entrada pra textoLido.
                    memcpy(textoLido, valorAtual, copiarBytes);

                    // Zero no fim da cópia: marca onde o texto acaba.
                    textoLido[copiarBytes] = '\0';

                    // atof: converte o texto pra número decimal.
                    // Ex.: "23.6" vira 23.6. Texto que não é número ("abc") vira 0.
                    valorLido = (float)atof(textoLido);
                    break;
                }
                // Qualquer outro tipo não é lido aqui.
                default:
                    break;
                }

                // switch: grava valorLido em valorSaida no formato do tipo de destino.
                switch (destino)
                {
                // FLOAT32: os 4 bytes do float vão direto.
                case FLOAT32:
                    memcpy(valorSaida, &valorLido, 4);
                    break;
                // INT32: arredonda e copia os 4 bytes do inteiro.
                case INT32:
                {
                    // inteiro32: valorLido arredondado, já como inteiro de 32 bits.
                    // roundf: arredonda pro mais próximo (23.6 vira 24).
                    int32_t inteiro32 = (int32_t)roundf(valorLido);
                    memcpy(valorSaida, &inteiro32, 4);
                    break;
                }
                // INT16: arredonda, trava em -32768 a 32767 e copia os 2 bytes.
                case INT16:
                {
                    // arredondado16: valorLido arredondado, ainda float, antes de travar nos limites.
                    float arredondado16 = roundf(valorLido);
                    // Abaixo do menor valor do INT16: prende no menor.
                    if (arredondado16 < -32768.0f)
                        arredondado16 = -32768.0f;
                    // Acima do maior valor do INT16: prende no maior.
                    if (arredondado16 > 32767.0f)
                        arredondado16 = 32767.0f;
                    // inteiro16: o valor já dentro da faixa, convertido pra inteiro de 16 bits.
                    int16_t inteiro16 = (int16_t)arredondado16;
                    memcpy(valorSaida, &inteiro16, 2);
                    break;
                }
                // INT8: arredonda e trava em -128 a 127. O byte guarda o inteiro com sinal.
                case INT8:
                {
                    // arredondado8: valorLido arredondado, ainda float, antes de travar nos limites.
                    float arredondado8 = roundf(valorLido);
                    // Abaixo do menor valor do INT8: prende no menor.
                    if (arredondado8 < -128.0f)
                        arredondado8 = -128.0f;
                    // Acima do maior valor do INT8: prende no maior.
                    if (arredondado8 > 127.0f)
                        arredondado8 = 127.0f;
                    // (int8_t) aplica o sinal, (uint8_t) devolve como byte pra gravar na saída.
                    valorSaida[0] = (uint8_t)(int8_t)arredondado8;
                    break;
                }
                // UINT8: arredonda e trava em 0 a 255. Ex.: 300.2 vira 255, -5 vira 0.
                case UINT8:
                {
                    // arredondadoU8: valorLido arredondado, ainda float, antes de travar nos limites.
                    float arredondadoU8 = roundf(valorLido);
                    // Sem sinal, o menor valor é 0: negativos viram 0.
                    if (arredondadoU8 < 0.0f)
                        arredondadoU8 = 0.0f;
                    // Acima de 255 não cabe em 1 byte: prende em 255.
                    if (arredondadoU8 > 255.0f)
                        arredondadoU8 = 255.0f;
                    valorSaida[0] = (uint8_t)arredondadoU8;
                    break;
                }
                // STRING: o número vira texto (ex.: 23.6 vira "23.6") e é gravado com a largura larguraSaida.
                case STRING:
                {
                    // textoGravado: espaço onde o texto do número é montado antes de ir pra saída.
                    // 32 bytes sobram: um float escrito por extenso tem no máximo uns 15 caracteres.
                    char textoGravado[32];

                    // snprintf: escreve o número como texto dentro de textoGravado, sem passar dos 32 bytes.
                    // %g: escreve só o necessário. 23.6 vira "23.6" e 24.0 vira "24".
                    snprintf(textoGravado, sizeof(textoGravado), "%g", valorLido);

                    // tamanhoTexto: quantos caracteres o texto tem, sem contar o zero do fim.
                    // Ex.: "23.6" dá 4.
                    size_t tamanhoTexto = strlen(textoGravado);

                    // copiarBytes: quantos caracteres cabem na saída.
                    // Se o texto for maior que larguraSaida, o que passar é cortado.
                    // Ex.: larguraSaida 3 e "23.6" copia só 3 ("23.").
                    size_t copiarBytes = (tamanhoTexto < larguraSaida) ? tamanhoTexto : larguraSaida;

                    // memset: zera os larguraSaida bytes deste valor, pra o que sobrar ficar com zeros.
                    memset(valorSaida, 0, larguraSaida);

                    // memcpy: copia os caracteres pra saída.
                    // Ex.: larguraSaida 8 e "23.6" fica '2' '3' '.' '6' 0 0 0 0.
                    memcpy(valorSaida, textoGravado, copiarBytes);
                    break;
                }
                // Não chega aqui: destino maior que STRING já foi recusado no começo da função.
                default:
                    break;
                }
            }

            // Terminou o laço sem problema: avisa que converteu.
            return true;
        }

        // tipoDestino: devolve quantos bytes um tipo qualquer ocupa, usado para o tipo de destino.
        // static: não depende de objeto, então é chamada assim: TipoDado::tipoDestino(TipoDado::INT8).
        // uint8_t tipo: o tipo que a gente quer medir (um valor de 0 a 5).
        static uint8_t tipoDestino(uint8_t tipo)
        {
            // switch: compara o tipo recebido com cada constante.
            switch (tipo)
            {
            // 4 bytes: FLOAT32 e INT32.
            case FLOAT32:
            case INT32:
                return 4;
            // 2 bytes: INT16.
            case INT16:
                return 2;
            // 1 byte: INT8 e UINT8.
            case INT8:
            case UINT8:
                return 1;
            // default: STRING devolve 0, porque o tamanho do texto não é fixo.
            // Quem chama converterDado informa as larguras (larguraEntrada e larguraSaida). Valor desconhecido também devolve 0.
            default:
                return 0;
            }
        }

        // larguraMinima: descobre quantos bytes cada palavra precisa pra caber inteira, sem cortar.
        // static: não depende de objeto, é chamada assim: TipoDado::larguraMinima(entrada, 10000, 64).
        // entrada: onde estão as palavras, uma atrás da outra.
        // quantidade: quantas palavras tem na entrada.
        // larguraEntrada: quantos bytes cada palavra ocupa na entrada. Ex.: 64.
        // Devolve o tamanho da maior palavra mais 1 (o zero do fim).
        // Ex.: maior palavra "desligar" (8 letras) devolve 9.
        static size_t larguraMinima(const uint8_t *entrada, size_t quantidade, size_t larguraEntrada)
        {
            // maiorTamanho: tamanho da maior palavra vista até agora. Começa em 0.
            size_t maiorTamanho = 0;

            // for: passa por cada palavra da entrada.
            for (size_t posicaoPalavra = 0; posicaoPalavra < quantidade; posicaoPalavra++)
            {
                // palavraAtual: endereço da palavra número posicaoPalavra.
                // posicaoPalavra * larguraEntrada: pula as palavras anteriores, cada uma com larguraEntrada bytes.
                const uint8_t *palavraAtual = entrada + posicaoPalavra * larguraEntrada;

                // tamanhoPalavra: quantos bytes a palavra tem até o primeiro zero.
                // Se não achar zero, a palavra ocupa a largura inteira.
                size_t tamanhoPalavra = 0;
                while (tamanhoPalavra < larguraEntrada && palavraAtual[tamanhoPalavra] != 0)
                {
                    tamanhoPalavra++;
                }

                // Se esta palavra é maior que a maior até agora, ela vira a nova maior.
                if (tamanhoPalavra > maiorTamanho)
                {
                    maiorTamanho = tamanhoPalavra;
                }
            }

            // Mais 1 pro zero do fim da palavra.
            return maiorTamanho + 1;
        }

        // private: só a própria classe enxerga o que vem abaixo.
    private:
        // Guarda qual tipo de dado o objeto é (um dos valores de 0 a 5).
        uint8_t tipoDadoConvertido;
    };

}
