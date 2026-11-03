# etapas.py
# As perguntas do script (palavra, porta, modo de pasta, confirmar) e o executar_fluxo que roda elas em ordem.
# Cada etapa devolve None quando deu certo, ou "C" / "V" quando o usuário cancelou ou voltou.

# Parte da biblioteca que lista as portas COM disponíveis
import serial.tools.list_ports

from config import CANCELAR, RESET, VERMELHO, VOLTAR, estado
from tela import perguntar


# Pergunta a palavra, Enter mantém a atual
def passo_palavra():
    while True:
        atual = estado["palavra"]
        resposta = perguntar(f"Palavra [{atual or 'nenhuma'}]")
        # C ou V sobem pro fluxo decidir
        if resposta in (CANCELAR, VOLTAR):
            return resposta
        # Enter sem digitar mantém a palavra atual, se ela existir
        if resposta == "" and atual:
            return None
        # Enter sem digitar e sem palavra atual, pergunta de novo
        if resposta == "":
            print(f"{VERMELHO}Digite uma palavra.{RESET}")
            continue
        # Minúscula e sem espaços, vira o prefixo do nome do arquivo
        estado["palavra"] = resposta.lower().replace(" ", "_")
        return None


# Mostra as portas COM e pergunta qual usar
def passo_porta():
    while True:
        portas = list(serial.tools.list_ports.comports())
        # Sem porta nenhuma não tem o que escolher
        if not portas:
            print(f"{VERMELHO}Nenhuma porta COM encontrada, confere o cabo.{RESET}")
            return CANCELAR
        # Lista numerada das portas, o número é a posição na lista e não o número da COM
        for posicao, porta in enumerate(portas, start=1):
            print(f"  [{posicao}] {porta.device} - {porta.description}")
        resposta = perguntar(f"Numero da porta [{estado['porta'] or 'nenhuma'}]")
        if resposta in (CANCELAR, VOLTAR):
            return resposta
        # Enter mantém a porta atual, se ela existir
        if resposta == "" and estado["porta"]:
            return None
        # Número fora da lista, pergunta de novo
        if not resposta.isdigit() or not 1 <= int(resposta) <= len(portas):
            print(f"{VERMELHO}Numero invalido.{RESET}")
            continue
        # Fecha a conexão antiga pra abrir a nova na próxima gravação
        if estado["conexao"]:
            estado["conexao"].close()
            estado["conexao"] = None
        estado["porta"] = portas[int(resposta) - 1].device
        return None


# Pergunta o modo de pasta
def passo_modo():
    while True:
        print("  [1] PASTA UNICA (ligar_001.wav)")
        print("  [2] PASTA POR PALAVRA (ligar/ligar_001.wav)")
        resposta = perguntar("Modo")
        if resposta in (CANCELAR, VOLTAR):
            return resposta
        if resposta not in ("1", "2"):
            print(f"{VERMELHO}Digite 1 ou 2.{RESET}")
            continue
        estado["modo_pasta"] = int(resposta)
        return None


# Última pergunta antes de gravar
def passo_confirmar():
    resposta = perguntar("Enter para gravar")
    if resposta in (CANCELAR, VOLTAR):
        return resposta
    return None


# Roda as perguntas em ordem, V volta uma pergunta, C cancela tudo, devolve True se chegou ao fim
def executar_fluxo(passos):
    posicao = 0
    while posicao < len(passos):
        resultado = passos[posicao]()
        if resultado == CANCELAR:
            return False
        if resultado == VOLTAR:
            # V na primeira pergunta volta pro menu
            if posicao == 0:
                return False
            posicao -= 1
        else:
            posicao += 1
    return True
