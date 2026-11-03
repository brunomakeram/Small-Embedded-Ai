# placa.py
# Conversa com o ESP32 pela porta serial: abre a conexão, manda o G e recebe o WAV de 2 segundos.
# Não salva nada em disco, só devolve os bytes recebidos.

import time

# Biblioteca que conversa com a porta COM
import serial

from config import AMARELO, RESET, TAMANHO_ESPERADO, TEMPO_LIMITE, VELOCIDADE, VERMELHO, estado


# Abre a conexão com a placa se ainda não estiver aberta
def abrir_conexao():
    if estado["conexao"] is None:
        conexao = serial.Serial(estado["porta"], VELOCIDADE, timeout=TEMPO_LIMITE)
        # Abrir a porta pode reiniciar a placa, espera ela voltar
        time.sleep(2)
        estado["conexao"] = conexao
    return estado["conexao"]


# Manda o G e devolve os bytes do WAV, ou None se deu erro (a mensagem de erro já fica em estado)
def receber_wav():
    try:
        conexao = abrir_conexao()
    except serial.SerialException:
        estado["mensagem"] = f"{VERMELHO}Nao abriu a porta, tem CoolTerm ou Monitor Serial aberto?{RESET}"
        return None
    # Descarta qualquer lixo que tenha chegado antes
    conexao.reset_input_buffer()
    # Dispara a gravação no ESP32
    conexao.write(b"G")
    print(f"{AMARELO}LED piscando, fale quando ficar verde...{RESET}")
    # Espera os 64044 bytes, ou o tempo limite
    dados = conexao.read(TAMANHO_ESPERADO)
    # Se faltou byte ou não começa com RIFF, a gravação falhou
    if len(dados) != TAMANHO_ESPERADO or not dados.startswith(b"RIFF"):
        estado["mensagem"] = f"{VERMELHO}Falhou: chegaram {len(dados)} bytes de {TAMANHO_ESPERADO}.{RESET}"
        return None
    return dados
