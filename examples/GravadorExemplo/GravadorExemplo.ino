#include <Seai.h>

// Usa a duração e os GPIOs padrão do INMP441 definidos no construtor.
Gravador::Gravador gravador;

// Inicializa a Serial e configura o canal I2S antes de receber comandos.
void setup()
{
    Serial.begin(921600);
    
    gravador.iniciar();
}

// Verifica continuamente se há um comando serial que solicite uma gravação.
void loop()
{
    gravador.gravar();
}