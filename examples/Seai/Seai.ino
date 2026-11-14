#include <Seai.h>

// Mantém uma instância do modelo entre setup() e loop()....
Seai::Modelo modeloMachineLearning;
// Cada linha contém duas características usadas como entrada de uma amostra de treino....
const std::vector<std::vector<float>> entradasTreino = {
    {0.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 0.0f},
    {1.0f, 1.0f}
};
// Define o alvo correspondente a cada linha de entradasTreino; as posições precisam permanecer alinhadas....
const std::vector<std::vector<float>> saidasTreino = {
    {0.0f},
    {1.0f},
    {1.0f},
    {0.0f}
};
// Fornece a amostra que será prevista depois que o modelo concluir o treinamento....
const std::vector<float> entradaPrevisao = {1.0f, 0.0f};

// Inicializa a Serial e treina o modelo uma vez; os comandos de transferência são atendidos depois no loop()....
void setup() {
    Serial.begin(115200);
    delay(2000);
    modeloMachineLearning.executar(entradasTreino, saidasTreino, entradaPrevisao);
}

// Atende E para exportar e I para importar; sem esses comandos, não executa transferência de modelo....
// As mensagens periódicas apenas indicam que o treino de setup() já terminou....
void loop() {
    if (Serial.available()) {
        char comando = (char)Serial.read();
        if (comando == 'E') {
            modeloMachineLearning.exportar();
        } else if (comando == 'I') {
            modeloMachineLearning.importar();
        }
    }
    Serial.println("\n================================");
    Serial.println("Treinamento finalizado.");
    Serial.println("================================\n");
    delay(5000);
}