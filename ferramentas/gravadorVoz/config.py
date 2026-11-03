# config.py
# Constantes, cores, opções do menu e o estado do gravador.
# O estado fica aqui pra todos os arquivos enxergarem o mesmo dicionário, sem um importar o outro.

from pathlib import Path

# Mesma velocidade do Serial.begin do ESP32
VELOCIDADE = 921600
# Segundos que a leitura espera os bytes chegarem
TEMPO_LIMITE = 10
# 44 bytes do cabeçalho WAV mais 64000 bytes de áudio (2 segundos a 16000 Hz, 16 bits)
TAMANHO_ESPERADO = 44 + 64000
# Pasta Audios ao lado deste arquivo, assim o caminho funciona em qualquer computador sem ajustar nada
PASTA_BASE = Path(__file__).resolve().parent / "Audios"

# Letras que valem em qualquer pergunta
CANCELAR = "C"
VOLTAR = "V"

# Cores do terminal
AZUL = "\033[94m"
VERDE = "\033[92m"
AMARELO = "\033[93m"
VERMELHO = "\033[91m"
BRANCO = "\033[97m"
TITULO = "\033[41m\033[97m"
RESET = "\033[0m"

# Largura entre as bordas da caixa
LARGURA_INTERNA = 60

# Opções do menu principal, número e nome
OPCOES = [
    ("01", "GRAVAR AMOSTRA"),
    ("02", "TROCAR PALAVRA"),
    ("03", "MODO DE PASTA"),
    ("04", "TROCAR PORTA COM"),
    ("05", "CONTAR GRAVACOES"),
    ("06", "EXECUTAR"),
    ("00", "SAIR"),
]

# Tudo que o script lembra enquanto está aberto
estado = {
    # Nome da porta, tipo COM5
    "porta": None,
    # Conexão aberta com a placa
    "conexao": None,
    # Palavra que está sendo gravada
    "palavra": "",
    # 1 é pasta única, 2 é pasta por palavra
    "modo_pasta": 1,
    # Texto que aparece embaixo do menu depois de cada ação
    "mensagem": "",
}
