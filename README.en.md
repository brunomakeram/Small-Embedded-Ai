<div align="center">

# Small Embedded AI (Seai)

**On-device neural-network training and inference for ESP32 and Raspberry Pi Pico (RP2040).**

[![Version](https://img.shields.io/badge/version-3.0.0-blue.svg)](https://github.com/brunomakeram/Small-Embedded-Ai/releases)
[![Platforms](https://img.shields.io/badge/platforms-ESP32%20%7C%20RP2040-orange.svg)](https://github.com/brunomakeram/Small-Embedded-Ai)
[![Language](https://img.shields.io/badge/language-C%2B%2B-informational.svg)](https://github.com/brunomakeram/Small-Embedded-Ai)
[![Arduino](https://img.shields.io/badge/framework-Arduino-00979D.svg?logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Documentation](https://img.shields.io/badge/docs-website-brightgreen.svg)](https://small-embedded-ai.netlify.app/)

[Português](README.md) · **English**

</div>

Seai is a modular C++ library for training and running lightweight neural networks directly on supported microcontrollers. It provides an object-oriented model API, Adam optimization, batch training, inference, optional weight persistence, CSV utilities, and I2S audio capture components.

> **Scope:** This README documents version **3.0.0**. Supported architectures are ESP32 and Raspberry Pi Pico (RP2040). AVR boards, including Arduino Uno, are not supported.

## Table of contents

- [Features](#features)
- [Supported hardware and requirements](#supported-hardware-and-requirements)
- [Installation](#installation)
- [Quick start](#quick-start)
- [Model API and training behavior](#model-api-and-training-behavior)
- [Examples](#examples)
- [Optional modules](#optional-modules)
- [Repository layout](#repository-layout)
- [Serial input conventions](#serial-input-conventions)
- [Limitations and practical guidance](#limitations-and-practical-guidance)
- [Contributing](#contributing)
- [License and credits](#license-and-credits)

## Features

- Train a feed-forward neural network on-device from numeric input and target matrices.
- Configure hidden-layer widths, epochs, batch size, and learning rate through the model object.
- Optimize parameters using Adam and backpropagation.
- Run inference on a trained model using `prever(...)`.
- Calculate and expose the final mean squared error (MSE) through `perdaFinal`.
- Optionally persist compatible model weights when weight persistence is enabled in the build.
- Read and write numeric CSV datasets through the storage module on supported configurations.
- Capture mono PCM audio from an I2S microphone using the ESP32-S3 recorder module.
- Enable serial diagnostic output through the `Verbose` module.

## Supported hardware and requirements

| Item | Requirement / status |
|---|---|
| Supported architectures | ESP32 and RP2040 (Raspberry Pi Pico family) |
| Arduino environment | Arduino IDE with the Seai library installed |
| AVR boards | Not supported, including Arduino Uno |
| Voice recorder | ESP32-S3, Arduino-ESP32 core 3.x, and a compatible INMP441 microphone |
| RP2040 UF2 example | Arduino-Pico core and a board/profile compatible with the selected flash configuration |

Select the correct board package and board profile for your device before compiling. Hardware-specific modules and examples may have additional pin, core-version, flash, or serial configuration requirements; consult the relevant example and the voice-recorder guide.

## Installation

### Arduino IDE

Install Seai as an Arduino library using the repository/library folder, then restart the IDE if necessary. The library metadata is in `library.properties`:

- Library name: `Seai`
- Version: `3.0.0`
- Declared architectures: `esp32,rp2040`

Open an individual sketch under `examples/` and select the target board and port before compiling or uploading.

### Clone the repository

```bash
git clone https://github.com/brunomakeram/Small-Embedded-Ai.git
```

Install or place the library in the Arduino libraries directory according to your Arduino IDE setup. The sketches in `examples/` are standalone; open the `.ino` file in the corresponding example folder.

## Quick start

The following example illustrates the core API with two input features and one output. The project sketch `examples/Seai/Seai.ino` includes serial input handling and error checks.

```cpp
#include <Seai.h>
#include <vector>

Modelo::Modelo model;

const std::vector<std::vector<float>> inputs = {
    {0.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 0.0f},
    {1.0f, 1.0f}
};

const std::vector<std::vector<float>> targets = {
    {0.0f},
    {1.0f},
    {1.0f},
    {0.0f}
};

void setup() {
    Serial.begin(115200);

    // Train using the supplied dataset and evaluate the given input.
    if (!model.executar(inputs, targets, {0.0f, 1.0f})) {
        Serial.println("Training failed.");
        return;
    }

    // Inference can also be requested after training.
    const std::vector<float> prediction = model.prever({0.0f, 1.0f});
    if (!prediction.empty()) {
        Serial.println(prediction[0], 4);
    }
}

void loop() {}
```

`executar(...)` receives a matrix of training inputs, a matrix of expected outputs, and one input vector to evaluate after training. Each input row must have a matching target row; all rows must have consistent dimensions. Values must be finite.

## Model API and training behavior

The main public class is `Modelo::Modelo`, available through `#include <Seai.h>`.

| Member | Description |
|---|---|
| `taxaAprendizado` | Positive learning rate used to initialize Adam. Default: `0.05f`. |
| `epocas` | Number of passes over the training data. Default: `500`. |
| `tamanhoLote` | Number of samples accumulated per parameter update. Values below 1 are treated as 1 during training. Default: `32`. |
| `neuroniosPorCamada` | Number of neurons in each hidden layer. The output width is inferred from the targets. |
| `perdaFinal` | MSE calculated on the full training dataset after training. |
| `verbose` | Logging configuration exposed by the `Verbose` module. |
| `executar(inputs, targets, predictionInput)` | Validates the data, builds the network, trains it, computes the final loss, optionally saves weights, and evaluates `predictionInput`. Returns `false` on invalid data or configuration. |
| `prever(input)` | Runs inference using the current model. Returns an empty vector if the model has not been trained or the input is invalid. |

### Implementation notes

- The model constructs hidden layers from `neuroniosPorCamada`; the output-layer width comes from the target vectors.
- Hidden layers use `tanh`; the output layer is linear, which supports regression-style outputs.
- Training uses backpropagation, Adam updates, shuffled sample order, and mini-batches.
- The loss reported by `perdaFinal` is mean squared error over the provided dataset.
- Each configured hidden layer must contain between 1 and 64 neurons. The output layer is also limited to 64 neurons.
- Inputs, targets, and prediction values must be finite, and all sample dimensions must agree.
- The model uses `std::vector` for its data and working buffers; account for the available RAM when choosing dataset size and network dimensions.

Example configuration:

```cpp
model.neuroniosPorCamada = {8, 4}; // Two hidden layers
model.epocas = 300;
model.tamanhoLote = 8;
model.taxaAprendizado = 0.01f;
```

## Examples

Each directory contains a standalone Arduino sketch. Examples use compact datasets to demonstrate the library; training data and model quality are illustrative and should not be treated as a general-purpose pretrained model.

| Example | Sketch | Demonstration |
|---|---|---|
| Basic Seai model | [`examples/Seai/Seai.ino`](examples/Seai/Seai.ino) | Training and inference with two numeric features. |
| Text classification | [`examples/ClassificacaoPLN/ClassificacaoPLN.ino`](examples/ClassificacaoPLN/ClassificacaoPLN.ino) | Classification using an explicitly defined vocabulary; this is not a general-purpose language model. |
| Sentiment classification | [`examples/ClassificacaoSentimentos/ClassificacaoSentimentos.ino`](examples/ClassificacaoSentimentos/ClassificacaoSentimentos.ino) | A small sentiment-classification demonstration. |
| Battery prediction | [`examples/PrevisaoBateria/PrevisaoBateria.ino`](examples/PrevisaoBateria/PrevisaoBateria.ino) | Regression-style estimation using battery-related values. |
| Propeller anomaly | [`examples/AnomaliaHelice/AnomaliaHelice.ino`](examples/AnomaliaHelice/AnomaliaHelice.ino) | Anomaly-analysis demonstration based on motor current. |
| I2S voice recorder | [`examples/GravadorExemplo/GravadorExemplo.ino`](examples/GravadorExemplo/GravadorExemplo.ino) | Audio capture on supported ESP32-S3 hardware. |
| RP2040 persistence | [`examples/RP2040_16MB/RP2040_16MB.ino`](examples/RP2040_16MB/RP2040_16MB.ino) | Weight persistence and prediction on a compatible RP2040 setup. |

Example images are in [`imagens/`](imagens/). Open each sketch independently, configure its board and serial settings, and follow the prompts printed by that sketch.

### RP2040 persistence example

The `RP2040_16MB` sketch enables weight persistence with `SALVAR_PESO`. It demonstrates a linear relationship (`y = 2x`) and asks for a prediction value in the Serial Monitor. It uses a 4096-byte emulated EEPROM area; the board's advertised 16 MB flash capacity is **not** used as a 16 MB EEPROM allocation. Use the Arduino-Pico core and a board profile compatible with the selected flash configuration.

## Optional modules

### Weight persistence

Weight persistence is optional and is enabled in a sketch with the appropriate compile-time configuration, as demonstrated by the RP2040 example. When enabled, the model attempts to load compatible saved weights and saves the final trained weights. If weights are absent or invalid/incompatible, training proceeds with newly initialized weights. Verify the target board's storage implementation and available capacity before relying on persistence.

### CSV storage

The `Armazenamento` module provides LittleFS-based CSV operations, including mounting storage, reading numeric input/output columns, writing a dataset, and appending a record. The number of input columns is specified when reading; remaining columns are treated as outputs. CSV files must contain a header and at least one data row, with consistent numeric columns.

Storage support is conditional in the umbrella header. Confirm that the target architecture/build enables the module and provides the expected LittleFS implementation before using it.

### I2S audio recorder

The recorder module is intended for ESP32-S3 with Arduino-ESP32 core 3.x and an INMP441-compatible microphone. Its default configuration uses a 16 kHz sample rate, mono 16-bit PCM WAV output, and BCLK/WS/DIN GPIOs 4/5/9. The recording duration defaults to 2 seconds and is clamped to 1–10 seconds. Pins and duration can be provided to the recorder constructor.

The sketch listens on Serial for the character `G`; when received, it records audio and sends a WAV header followed by PCM samples. Check that the selected board exposes the configured pins and that the microphone is wired for the channel used by the example.

For wiring, setup, and the companion Windows/Python recording utility, see [`ferramentas/gravadorVoz/README.md`](ferramentas/gravadorVoz/README.md). That utility documents its specific prerequisites and procedure; do not assume it applies unchanged to other operating systems or hardware.

## Repository layout

```text
.
├── README.md                     # Portuguese documentation
├── README.en.md                  # English documentation
├── GLOSSARIO.md                  # Portuguese glossary
├── LICENSE
├── library.properties
├── src/
│   ├── Seai.h                    # Public umbrella header
│   ├── Adam/                     # Adam optimizer
│   ├── Aprendizagem/             # Learning routines
│   ├── Armazenamento/            # LittleFS CSV storage
│   ├── Camadas/                  # Network layers
│   ├── Compilar/                 # Training/model compilation support
│   ├── Conversor/                # Data conversion utilities
│   ├── Epocas/                   # Epoch handling
│   ├── Gradientes/               # Gradient/backpropagation support
│   ├── Gravador/                 # I2S audio recorder
│   ├── Modelo/                   # Model API
│   ├── Neuronios/                # Neuron implementation
│   ├── Perdas/                   # Loss functions
│   ├── Pesos/                    # Weight persistence
│   ├── Placas/                   # Platform detection/support
│   ├── Prever/                   # Inference support
│   ├── Treinar/                  # Training support
│   └── Verbose/                  # Serial diagnostics
├── examples/                     # Standalone Arduino sketches
├── ferramentas/gravadorVoz/      # Voice dataset recording utility
└── imagens/                      # Example images
```

The public include is `Seai.h`. Individual modules are organized below `src/`; hardware- or feature-specific modules may be conditionally included depending on build configuration.

## Serial input conventions

Several examples read values from the Arduino Serial Monitor. Set the baud rate indicated by the sketch and select a line ending accepted by that example, commonly **Newline** or **Both NL & CR**. Enter the format requested by the prompt; numeric examples generally expect a decimal point. The recorder sketch uses Serial at 921600 baud for audio transfer, so its serial settings differ from the basic ML sketches.

## Limitations and practical guidance

- Seai is for the supported ESP32 and RP2040 architectures; it is not compatible with AVR boards such as Arduino Uno.
- Training executes on the microcontroller. Runtime and RAM use grow with the dataset, number of features, layer widths, and epoch count.
- Keep network dimensions and training data appropriate for the device's memory and processing budget.
- Normalize or otherwise prepare input features appropriately for the problem. The library validates finite values and dimensions but does not automatically prepare a dataset for you.
- Example datasets are demonstrations, not guarantees of accuracy. Use representative data and independently evaluate a trained model for your application.
- Persistent weights are tied to the supported storage configuration; confirm storage capacity and compatibility on the actual board.
- Audio capture depends on the specified ESP32-S3/core/microphone combination and correct pin configuration.

## Contributing

Issues, corrections, and improvements are welcome through the repository's GitHub issue and pull-request workflows. When reporting a problem, include the board model, board package/core version, Arduino IDE version, relevant sketch, and complete compiler or runtime output.

## License and credits

Created by **Bruno Maker**. See [`LICENSE`](LICENSE) for the applicable license terms.

- Repository: [brunomakeram/Small-Embedded-Ai](https://github.com/brunomakeram/Small-Embedded-Ai)
- Project website: [small-embedded-ai.netlify.app](https://small-embedded-ai.netlify.app/)
