# tela.py
# Parte visual do terminal: a caixa do menu e as perguntas.

import os

from arquivos import contar_gravacoes
from config import (
    AMARELO, AZUL, BRANCO, CANCELAR, LARGURA_INTERNA, OPCOES, RESET,
    TITULO, VERDE, VOLTAR, estado,
)


# Desenha uma linha da caixa, cada pedaço é (texto, cor)
def linha_caixa(*pedacos):
    # Texto sem cor, serve pra calcular quantos espaços faltam até a borda direita
    texto_puro = "".join(texto for texto, cor in pedacos)
    # Texto com as cores aplicadas
    texto_colorido = "".join(cor + texto + RESET for texto, cor in pedacos)
    # Espaços pra completar a linha, o 1 é o espaço depois da borda esquerda
    espacos = LARGURA_INTERNA - 1 - len(texto_puro)
    print(AZUL + "║" + RESET + " " + texto_colorido + " " * espacos + AZUL + "║" + RESET)


# Desenha uma linha horizontal da caixa, recebe os três caracteres: esquerda, meio e direita
def linha_horizontal(esquerda, meio, direita):
    print(AZUL + esquerda + meio * LARGURA_INTERNA + direita + RESET)


# Limpa a tela e desenha o menu
def desenhar_tela():
    # Limpa o terminal do Windows
    os.system("cls")
    linha_horizontal("╔", "═", "╗")
    # Título centralizado com fundo vermelho
    print(AZUL + "║" + RESET + TITULO + "GRAVADOR DE VOZ MAKER INMP441".center(LARGURA_INTERNA) + RESET + AZUL + "║" + RESET)
    linha_horizontal("╠", "═", "╣")
    linha_caixa(("PORTA: ", AMARELO), (estado["porta"] or "nenhuma", BRANCO))
    linha_caixa(("PALAVRA: ", AMARELO), (estado["palavra"] or "nenhuma", BRANCO))
    nome_modo = "PASTA UNICA" if estado["modo_pasta"] == 1 else "PASTA POR PALAVRA"
    linha_caixa(("MODO: ", AMARELO), (nome_modo, BRANCO))
    linha_caixa(("GRAVACOES: ", AMARELO), (str(contar_gravacoes()), VERDE))
    linha_horizontal("╠", "═", "╣")
    # Uma linha pra cada opção do menu
    for numero, nome in OPCOES:
        linha_caixa((f"[{numero}]", AZUL), (" • ", BRANCO), (nome, AMARELO))
    linha_horizontal("╚", "═", "╝")
    # Mensagem do resultado da última ação, fica fixa embaixo do menu até a próxima opção
    if estado["mensagem"]:
        print(estado["mensagem"])


# Faz uma pergunta e devolve a resposta, se for C ou V devolve a letra maiúscula
def perguntar(texto):
    resposta = input(f"{VERDE}{texto} (C cancela, V volta): {RESET}").strip()
    if resposta.upper() in (CANCELAR, VOLTAR):
        return resposta.upper()
    return resposta
