#include <Seai.h>

// Usa a duração e os GPIOs padrão do INMP441 definidos no construtor.
Gravador::Gravador gravador;

// Inicializa a Serial e configura o canal I2S antes de receber comandos.
void setup()
{
    gravador.iniciar();
    Serial.println("Gravador pronto. Digite G no Monitor Serial para gravar.");
}

// Verifica continuamente se há um comando serial que solicite uma gravação.
void loop()
{
    gravador.gravar();
}