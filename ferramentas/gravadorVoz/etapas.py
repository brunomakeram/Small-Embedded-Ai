# Reúne as etapas interativas usadas para configurar e confirmar uma gravação de voz...
# Cada passo devolve None ao concluir, CANCELAR para interromper ou VOLTAR para retornar ao passo anterior...
# executar_fluxo interpreta esses resultados sem acoplar as perguntas à lógica do menu principal...

# Parte da biblioteca que lista as portas COM disponíveis...
import serial.tools.list_ports

from config import CANCELAR, RESET, VERMELHO, VOLTAR, estado
from tela import perguntar


# Solicita o rótulo da amostra e grava sua forma minúscula, com espaços substituídos por sublinhados...
# Enter preserva um rótulo existente; sem rótulo, a pergunta se repete até receber texto ou controle...
def passo_palavra():
    while True:
        atual = estado["palavra"]
        resposta = perguntar(f"Palavra [{atual or 'nenhuma'}]")
        # C ou V sobem pro fluxo decidir...
        if resposta in (CANCELAR, VOLTAR):
            return resposta
        # Enter sem digitar mantém a palavra atual, se ela existir...
        if resposta == "" and atual:
            return None
        # Enter sem digitar e sem palavra atual, pergunta de novo...
        if resposta == "":
            print(f"{VERMELHO}Digite uma palavra.{RESET}")
            continue
        # Minúscula e sem espaços, vira o prefixo do nome do arquivo...
        estado["palavra"] = resposta.lower().replace(" ", "_")
        return None


# Lista as portas disponíveis e guarda a porta escolhida no estado compartilhado...
# Se a seleção mudar, fecha a conexão antiga para que a próxima operação abra a nova porta...
def passo_porta():
    while True:
        portas = list(serial.tools.list_ports.comports())
        # Sem porta nenhuma não tem o que escolher...
        if not portas:
            print(f"{VERMELHO}Nenhuma porta COM encontrada, confere o cabo.{RESET}")
            return CANCELAR
        # Lista numerada das portas, o número é a posição na lista e não o número da COM...
        for posicao, porta in enumerate(portas, start=1):
            print(f"  [{posicao}] {porta.device} - {porta.description}")
        resposta = perguntar(f"Numero da porta [{estado['porta'] or 'nenhuma'}]")
        if resposta in (CANCELAR, VOLTAR):
            return resposta
        # Enter mantém a porta atual, se ela existir...
        if resposta == "" and estado["porta"]:
            return None
        # Número fora da lista, pergunta de novo...
        if not resposta.isdigit() or not 1 <= int(resposta) <= len(portas):
            print(f"{VERMELHO}Numero invalido.{RESET}")
            continue
        # Fecha a conexão antiga pra abrir a nova na próxima gravação...
        if estado["conexao"]:
            estado["conexao"].close()
            estado["conexao"] = None
        estado["porta"] = portas[int(resposta) - 1].device
        return None


# Permite escolher entre uma pasta única e uma subpasta separada para cada palavra...
# Atualiza estado["modo_pasta"] apenas após aceitar as opções 1 ou 2...
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


# Aguarda a confirmação explícita antes de iniciar a gravação no microfone...
# O cancelamento e o retorno são encaminhados ao fluxo sem iniciar a captura...
def passo_confirmar():
    resposta = perguntar("Enter para gravar")
    if resposta in (CANCELAR, VOLTAR):
        return resposta
    return None


# Executa os passos na ordem fornecida e interpreta os controles C e V...
# Devolve True quando todos os passos terminam ou False quando o usuário cancela ou volta do primeiro...
def executar_fluxo(passos):
    posicao = 0
    while posicao < len(passos):
        resultado = passos[posicao]()
        if resultado == CANCELAR:
            return False
        if resultado == VOLTAR:
            # V na primeira pergunta volta pro menu...
            if posicao == 0:
                return False
            posicao -= 1
        else:
            posicao += 1
    return True
