#pragma once
// Fornece Serial, temporização, tipos Arduino e controle do LED integrado.
#include <Arduino.h>
// Permite dimensionar o buffer de áudio conforme o tempo escolhido.
#include <vector>
// Declara a API I2S usada para configurar e ler o microfone INMP441.
#include "driver/i2s_std.h"
// Fornece o tipo de pino GPIO usado na configuração do I2S.
#include "driver/gpio.h"

// Agrupa a implementação do gravador no namespace correspondente à classe.
namespace Gravador
{
    // Mantém a configuração, os buffers e as operações de gravação em uma instância.
    class Gravador
    {
    private:
        // A taxa de 16 kHz é a frequência usada pelo formato WAV e pela captura I2S.
        static const int taxaAmostragem = 16000;

        // Guarda a duração escolhida pelo usuário, limitada no construtor ao intervalo de 1 a 10 segundos.
        int tempo_gravacao_segundos;
        // Guarda os GPIOs fornecidos para os sinais de clock e dados do INMP441.
        int pino_bclk;
        int pino_ws;
        int pino_din;
        // Usa o GPIO 48, padrão do LED RGB das placas ESP32-S3 deste exemplo.
        const int pino_led = 48;
        // Reserva uma amostra PCM de 16 bits por instante da gravação configurada.
        std::vector<int16_t> buffer_audio;
        // Recebe 1000 valores I2S por leitura, limitando cada bloco temporário a 4000 bytes.
        int32_t chunk_microfone[1000];
        // Guarda o cabeçalho WAV PCM mono de 16 kHz e 16 bits; iniciar() recalcula os campos de tamanho.
        uint8_t cabecalho_wav[44] = {
            // Identificador RIFF e tamanho inicial, substituído conforme a duração configurada.
            'R', 'I', 'F', 'F',
            0x24, 0xFA, 0x00, 0x00,
            // Identificadores WAVE/fmt e tamanho de 16 bytes da descrição PCM.
            'W', 'A', 'V', 'E',
            'f', 'm', 't', ' ',
            0x10, 0x00, 0x00, 0x00,
            // Indica PCM sem compressão e um canal de áudio.
            0x01, 0x00,
            0x01, 0x00,
            // Frequência de amostragem de 16000 Hz, armazenada em little-endian.
            0x80, 0x3E, 0x00, 0x00,
            // Taxa de 32000 bytes por segundo e alinhamento de 2 bytes por amostra mono.
            0x00, 0x7D, 0x00, 0x00,
            0x02, 0x00,
            // Cada amostra PCM tem 16 bits.
            0x10, 0x00,
            // Identificador dos dados de áudio e tamanho inicial, atualizado por iniciar().
            'd', 'a', 't', 'a',
            0x00, 0xFA, 0x00, 0x00
        };
        // Recebe do driver o identificador do canal I2S criado para o microfone.
        i2s_chan_handle_t canal_microfone = nullptr;

    public:
        // Usa 2 segundos e GPIOs 4, 5 e 9 por padrão; limita a duração a 1–10 segundos.
        Gravador(int tempoGravacaoSegundos = 2, int pinoBclk = 4, int pinoWs = 5, int pinoDin = 9)
            : tempo_gravacao_segundos(tempoGravacaoSegundos > 10 ? 10 : (tempoGravacaoSegundos < 1 ? 1 : tempoGravacaoSegundos)),
              pino_bclk(pinoBclk),
              pino_ws(pinoWs),
              pino_din(pinoDin),
              buffer_audio(taxaAmostragem * tempo_gravacao_segundos)
        {
        }

        // Prepara o cabeçalho WAV, a comunicação serial e o canal I2S antes da primeira gravação.
        void iniciar()
        {
            // Calcula o tamanho PCM em bytes usando 16 mil amostras por segundo e 2 bytes por amostra mono.
            uint32_t tamanho_dados = taxaAmostragem * tempo_gravacao_segundos * sizeof(int16_t);
            // O tamanho RIFF exclui os 8 bytes iniciais e inclui o restante do cabeçalho e dos dados.
            uint32_t tamanho_arquivo = tamanho_dados + 36;
            // Escreve os tamanhos do cabeçalho em little-endian, como exige o formato WAV.
            for (int indice = 0; indice < 4; indice++)
            {
                cabecalho_wav[4 + indice] = (tamanho_arquivo >> (8 * indice)) & 0xFF;
                cabecalho_wav[40 + indice] = (tamanho_dados >> (8 * indice)) & 0xFF;
            }

            // Usa a mesma velocidade serial esperada pelo programa receptor do áudio.
            Serial.begin(921600);

            // Solicita ao driver um canal I2S de recepção e deixa a seleção da unidade a cargo do ESP32.
            i2s_chan_config_t configuracao_canal = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
            i2s_new_channel(&configuracao_canal, NULL, &canal_microfone);

            // Define a frequência, o formato dos slots e os pinos usados pelo microfone.
            i2s_std_config_t configuracao_padrao = {
                // Configura o clock I2S para a frequência de amostragem do WAV.
                .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(taxaAmostragem),
                // O INMP441 fornece amostras de 32 bits em slots estéreo; gravar é feito pelo canal esquerdo.
                .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
                .gpio_cfg = {
                    // O microfone não usa o sinal master clock.
                    .mclk = I2S_GPIO_UNUSED,
                    // Associa BCLK ao GPIO escolhido, com GPIO 4 como padrão.
                    .bclk = (gpio_num_t)pino_bclk,
                    // Associa WS ao GPIO escolhido, com GPIO 5 como padrão.
                    .ws = (gpio_num_t)pino_ws,
                    // O canal é somente de recepção, portanto não há saída de dados.
                    .dout = I2S_GPIO_UNUSED,
                    // Associa DIN ao GPIO escolhido, com GPIO 9 como padrão.
                    .din = (gpio_num_t)pino_din,
                    // Mantém os sinais sem inversão, conforme o padrão do módulo.
                    .invert_flags = {
                        .mclk_inv = false,
                        .bclk_inv = false,
                        .ws_inv = false,
                    },
                },
            };

            // Aplica o formato e os pinos selecionados ao canal recém-criado.
            i2s_channel_init_std_mode(canal_microfone, &configuracao_padrao);
            // Habilita a recepção para deixar o canal pronto para capturar áudio.
            i2s_channel_enable(canal_microfone);
        }

        // Aguarda o comando de gravação, captura o áudio e envia o WAV pela Serial.
        void gravar()
        {
            // Retorna sem bloquear enquanto nenhum comando foi recebido.
            if (Serial.available() == 0)
            {
                return;
            }

            // Consome um byte serial; somente a letra G solicita uma gravação.
            char comando = Serial.read();
            if (comando != 'G')
            {
                return;
            }

            // Pisca o LED vermelho três vezes para avisar que a captura vai começar.
            for (int piscada = 0; piscada < 3; piscada++)
            {
                rgbLedWrite(pino_led, 30, 0, 0);
                delay(400);
                rgbLedWrite(pino_led, 0, 0, 0);
                delay(400);
            }

            // Reinicia o canal para descartar amostras antigas que ficaram no buffer I2S.
            i2s_channel_disable(canal_microfone);
            i2s_channel_enable(canal_microfone);
            // Indica visualmente que a captura válida começou.
            rgbLedWrite(pino_led, 0, 30, 0);

            // Marca o início para comparar a duração real com a duração configurada.
            unsigned long inicio = millis();
            // Conta quantas amostras mono já foram copiadas para o buffer WAV.
            size_t amostras_gravadas = 0;
            // Converte a duração e a frequência de amostragem em total de amostras necessárias.
            size_t total_amostras = taxaAmostragem * tempo_gravacao_segundos;

            // Lê blocos I2S até preencher o buffer correspondente à duração solicitada.
            while (amostras_gravadas < total_amostras)
            {
                // Recebe do driver a quantidade de bytes efetivamente preenchida no bloco temporário.
                size_t bytes_lidos = 0;
                i2s_channel_read(canal_microfone, chunk_microfone, sizeof(chunk_microfone), &bytes_lidos, portMAX_DELAY);

                // Cada valor I2S tem 4 bytes; o modo estéreo intercala amostras esquerda e direita.
                int amostras_lidas = bytes_lidos / 4;
                // Avança de dois em dois para selecionar somente o canal esquerdo, como no código original.
                for (int i = 0; i < amostras_lidas; i += 2)
                {
                    // Evita ultrapassar o total de amostras reservado para a gravação.
                    if (amostras_gravadas < total_amostras)
                    {
                        // Desloca os bits úteis do INMP441 para armazená-los como uma amostra PCM de 16 bits.
                        buffer_audio[amostras_gravadas] = chunk_microfone[i] >> 16;
                        // Avança a posição de escrita após copiar a amostra.
                        amostras_gravadas++;
                    }
                }
            }

            // Calcula o tempo da captura, sem incluir os avisos do LED nem a transmissão serial.
            unsigned long duracao = millis() - inicio;
            // Apaga o LED enquanto o áudio gravado é enviado.
            rgbLedWrite(pino_led, 0, 0, 0);
            // Envia primeiro o cabeçalho, necessário para o receptor interpretar o conteúdo como WAV.
            Serial.write(cabecalho_wav, sizeof(cabecalho_wav));
            // Envia apenas as amostras usadas, em bytes PCM de 16 bits.
            Serial.write((uint8_t *)buffer_audio.data(), total_amostras * sizeof(int16_t));

            // Converte o tempo configurado para milissegundos para verificar a duração da captura.
            unsigned long duracao_esperada = tempo_gravacao_segundos * 1000;
            // Considera correta uma captura dentro de uma margem de 100 ms em torno do valor esperado.
            if (duracao > duracao_esperada - 100 && duracao < duracao_esperada + 100)
            {
                // Verde indica que a duração da captura ficou dentro da margem esperada.
                rgbLedWrite(pino_led, 0, 30, 0);
            }
            else
            {
                // Vermelho indica que a captura ficou fora da margem de duração esperada.
                rgbLedWrite(pino_led, 30, 0, 0);
            }

            // Mantém o diagnóstico visível por dois segundos e depois apaga o LED.
            delay(2000);
            rgbLedWrite(pino_led, 0, 0, 0);
        }
    };
}
