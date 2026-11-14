#include <Seai.h>

// Usa dois segundos de captura e os GPIOs padrão do INMP441 definidos na classe Gravador....
Seai::Gravador gravador;

// Inicializa a Serial na velocidade esperada pela ferramenta e prepara o canal de áudio I2S....
void setup()
{
    Serial.begin(921600);
    
    gravador.iniciar();
}

// Verifica continuamente o comando de captura e envia o WAV recebido pela Serial....
void loop()
{
    gravador.gravar();
}