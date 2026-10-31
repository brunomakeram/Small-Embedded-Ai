armazenamento.montar("/Bateria.csv");
armazenamento.lerDataset("/Bateria.csv", bateria);
armazenamento.gravarRegistro("/Bateria.csv", 49.0f, 5.0f, 44.4f);

---

## Verbose

Conversa com o usuário pelo Serial Monitor (115200 baud).

- `float lerNumero(const char *mensagem)`
  - Mostra a mensagem, espera o usuário digitar e devolve o valor como `float`.
- `void mostrarResultados(float carga, float distancia, float perdas, const float *pesos, float vies, float gasto_previsto)`
  - Imprime o relatório final do treino.

Saída de exemplo:

Digite a carga (kg): 49
Digite a distância (km): 5

===== RESULTADOS DO TREINAMENTO =====
Carga inputada: 49.00 kg
Distância inputada: 5.00 km
Pesos Ajustados (w1, w2): [0.51984, 4.77810]
Viés Ajustado (b): -4.95865
MSE (Perda Final): 0.00023
-> Previsão de Gasto de Bateria: 44.40%
=====================================

---

## Estrutura da biblioteca

Maker/
  library.properties
  README.md
  src/          os módulos (.h)
  examples/
    Maker/
      Maker.ino exemplo de uso
`

As pastas `src` e `examples` são nomes fixos do formato de biblioteca do Arduino.
