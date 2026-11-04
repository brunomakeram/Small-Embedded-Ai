#include <Seai.h>

// Mantém uma instância do modelo entre setup() e loop().
Modelo::Modelo modeloMachineLearning;
// Cada linha contém duas características usadas como entrada de uma amostra de treino.
const std::vector<std::vector<float>> entradasTreino = {
    {0.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 0.0f},
    {1.0f, 1.0f}
};
// Define o alvo correspondente a cada linha de entradasTreino.
const std::vector<std::vector<float>> saidasTreino = {
    {0.0f},
    {1.0f},
    {1.0f},
    {0.0f}
};
// Fornece a amostra que será prevista depois que o modelo concluir o treinamento.
const std::vector<float> entradaPrevisao = {1.0f, 0.0f};

// Inicializa a Serial e executa o treino e a previsão uma vez ao iniciar a placa.
void setup() {
    Serial.begin(115200);
    delay(2000);
    modeloMachineLearning.executar(entradasTreino, saidasTreino, entradaPrevisao);
}

// Mantém o sketch ativo e informa que a execução inicial já foi concluída.
void loop() {
    Serial.println("\n================================");
    Serial.println("Treinamento finalizado.");
    Serial.println("================================\n");
    delay(5000);
}