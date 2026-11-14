# Renderiza no terminal o menu, as informações da sessão e as perguntas interativas...
# As funções usam as cores e a largura definidas em config.py para manter o desenho uniforme...

import os

from arquivos import contar_gravacoes
from config import (
    AMARELO, AZUL, BRANCO, CANCELAR, LARGURA_INTERNA, OPCOES, RESET,
    TITULO, VERDE, VOLTAR, estado,
)


# Compõe uma linha respeitando a largura interna e aplicando uma cor a cada trecho...
# Cada elemento de pedacos é um par (texto, código ANSI); o preenchimento usa o texto sem códigos de cor...
def linha_caixa(*pedacos):
    # Texto sem cor, serve pra calcular quantos espaços faltam até a borda direita...
    texto_puro = "".join(texto for texto, cor in pedacos)
    # Texto com as cores aplicadas...
    texto_colorido = "".join(cor + texto + RESET for texto, cor in pedacos)
    # Espaços pra completar a linha, o 1 é o espaço depois da borda esquerda...
    espacos = LARGURA_INTERNA - 1 - len(texto_puro)
    print(AZUL + "║" + RESET + " " + texto_colorido + " " * espacos + AZUL + "║" + RESET)


# Desenha a borda horizontal entre os caracteres laterais e repete o símbolo central...
# esquerda e direita definem os cantos; meio é o traço repetido pela largura configurada...
def linha_horizontal(esquerda, meio, direita):
    print(AZUL + esquerda + meio * LARGURA_INTERNA + direita + RESET)


# Limpa o terminal e redesenha o estado atual, incluindo gravações, opções e mensagens...
# Os valores mostrados vêm do estado compartilhado e da contagem atual de arquivos WAV...
def desenhar_tela():
    # Limpa o terminal do Windows...
    os.system("cls")
    linha_horizontal("╔", "═", "╗")
    # Título centralizado com fundo vermelho...
    print(AZUL + "║" + RESET + TITULO + "GRAVADOR DE VOZ MAKER INMP441".center(LARGURA_INTERNA) + RESET + AZUL + "║" + RESET)
    linha_horizontal("╠", "═", "╣")
    linha_caixa(("PORTA: ", AMARELO), (estado["porta"] or "nenhuma", BRANCO))
    linha_caixa(("PALAVRA: ", AMARELO), (estado["palavra"] or "nenhuma", BRANCO))
    nome_modo = "PASTA UNICA" if estado["modo_pasta"] == 1 else "PASTA POR PALAVRA"
    linha_caixa(("MODO: ", AMARELO), (nome_modo, BRANCO))
    linha_caixa(("GRAVACOES: ", AMARELO), (str(contar_gravacoes()), VERDE))
    linha_horizontal("╠", "═", "╣")
    # Uma linha pra cada opção do menu...
    for numero, nome in OPCOES:
        linha_caixa((f"[{numero}]", AZUL), (" • ", BRANCO), (nome, AMARELO))
    linha_horizontal("╚", "═", "╝")
    # Mensagem do resultado da última ação, fica fixa embaixo do menu até a próxima opção...
    if estado["mensagem"]:
        print(estado["mensagem"])


# Apresenta uma pergunta e devolve a resposta sem espaços externos...
# CANCELAR e VOLTAR são normalizados para maiúsculas; qualquer outra resposta conserva o texto digitado...
def perguntar(texto):
    resposta = input(f"{VERDE}{texto} (C cancela, V volta): {RESET}").strip()
    if resposta.upper() in (CANCELAR, VOLTAR):
        return resposta.upper()
    return resposta
