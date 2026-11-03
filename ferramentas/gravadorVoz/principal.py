# principal.py
# Ponto de entrada do gravador, é este arquivo que você roda.
# Liga cada opção do menu à sua função e repete o menu até escolher sair.

import os
import sys

# Biblioteca que permite abrir outro programa a partir deste
import subprocess

from arquivos import contar_por_palavra, salvar_wav
from config import AMARELO, RESET, VERDE, VERMELHO, estado
from etapas import (
    executar_fluxo,
    passo_confirmar,
    passo_modo,
    passo_palavra,
    passo_porta,
)
from placa import receber_wav
from pos_gravacao import menu_pos_gravacao
from tela import desenhar_tela

# Faz o terminal do Windows aceitar as cores e os desenhos das caixas
os.system("")
sys.stdout.reconfigure(encoding="utf-8")


# Opção 01, pergunta o que falta, grava e abre o menu de ouvir, salvar ou excluir
def acao_gravar():
    passos = [passo_palavra, passo_confirmar]
    # Sem porta escolhida, pergunta a porta primeiro
    if estado["porta"] is None:
        passos.insert(0, passo_porta)
    if not executar_fluxo(passos):
        return
    dados = receber_wav()
    # None é erro de porta ou bytes faltando, a mensagem já foi preparada pela placa
    if dados is None:
        return
    caminho = salvar_wav(dados)
    menu_pos_gravacao(caminho)


# Opção 02
def acao_trocar_palavra():
    executar_fluxo([passo_palavra])


# Opção 03
def acao_modo_pasta():
    executar_fluxo([passo_modo])


# Opção 04
def acao_trocar_porta():
    executar_fluxo([passo_porta])


# Opção 05, mostra quantas gravações tem de cada palavra
def acao_contar():
    contagem = contar_por_palavra()
    if not contagem:
        print(f"{AMARELO}Nenhuma gravacao ainda.{RESET}")
        input(f"{AMARELO}Pressione Enter para voltar.{RESET}")
        return
    print(
        f"{AMARELO}  ".join(
            f"{palavra}: {total}" for palavra, total in sorted(contagem.items())
        )
        + f"{RESET}"
    )
    input(f"{AMARELO}Pressione Enter para voltar.{RESET}")


# Opção 06, fecha o script e abre ele de novo, sem digitar o nome no terminal
def acao_executar():
    # Fecha a porta antes, senão a nova execução não consegue abrir a COM
    if estado["conexao"]:
        estado["conexao"].close()
    # sys.executable é o caminho do Python que está rodando, __file__ é este arquivo
    # subprocess.run abre o script de novo e espera ele terminar
    subprocess.run([sys.executable, __file__])
    # Quando a nova execução fechar, esta cópia antiga também fecha
    sys.exit()


# Liga cada número do menu à sua função
ACOES = {
    "01": acao_gravar,
    "02": acao_trocar_palavra,
    "03": acao_modo_pasta,
    "04": acao_trocar_porta,
    "05": acao_contar,
    "06": acao_executar,
}

# Repete o menu até escolher sair
while True:
    desenhar_tela()
    opcao = input(f"{VERDE}INFORME UMA OPCAO: {RESET}").strip().zfill(2)
    # Limpa a mensagem da ação anterior
    estado["mensagem"] = ""
    if opcao == "00":
        break
    if opcao in ACOES:
        ACOES[opcao]()
    else:
        estado["mensagem"] = f"{VERMELHO}Opcao invalida.{RESET}"

# Fecha a porta antes de sair
if estado["conexao"]:
    estado["conexao"].close()
