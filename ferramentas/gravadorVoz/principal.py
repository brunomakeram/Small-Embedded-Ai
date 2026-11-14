# Ponto de entrada da ferramenta de gravação de voz...
# Conecta as opções do menu às etapas de configuração, comunicação serial e gerenciamento do arquivo WAV...
# A sessão continua até o usuário escolher sair; a conexão serial é fechada antes do encerramento...

import os
import sys

# Biblioteca que permite abrir outro programa a partir deste...
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

# Faz o terminal do Windows aceitar as cores e os desenhos das caixas...
os.system("")
sys.stdout.reconfigure(encoding="utf-8")


# Opção 01: completa as configurações ausentes, solicita o áudio à placa e abre o menu da amostra...
# Se uma etapa for cancelada ou a captura falhar, retorna ao menu sem salvar bytes incompletos...
def acao_gravar():
    passos = [passo_palavra, passo_confirmar]
    # Sem porta escolhida, pergunta a porta primeiro...
    if estado["porta"] is None:
        passos.insert(0, passo_porta)
    if not executar_fluxo(passos):
        return
    dados = receber_wav()
    # None é erro de porta ou bytes faltando, a mensagem já foi preparada pela placa...
    if dados is None:
        return
    caminho = salvar_wav(dados)
    menu_pos_gravacao(caminho)


# Opção 02: altera o rótulo que será usado para nomear as próximas gravações...
def acao_trocar_palavra():
    executar_fluxo([passo_palavra])


# Opção 03: escolhe se os arquivos serão organizados numa pasta única ou por palavra...
def acao_modo_pasta():
    executar_fluxo([passo_modo])


# Opção 04: seleciona outra porta serial e fecha qualquer conexão vinculada à porta anterior...
def acao_trocar_porta():
    executar_fluxo([passo_porta])


# Opção 05: conta e exibe o total de amostras WAV agrupadas pelo rótulo de cada arquivo...
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


# Opção 06: encerra a conexão ativa e inicia outra instância do mesmo programa...
# O processo atual termina quando a instância substituta retorna...
def acao_executar():
    # Fecha a porta antes, senão a nova execução não consegue abrir a COM...
    if estado["conexao"]:
        estado["conexao"].close()
    # sys.executable é o caminho do Python que está rodando, __file__ é este arquivo...
    # subprocess.run abre o script de novo e espera ele terminar...
    subprocess.run([sys.executable, __file__])
    # Quando a nova execução fechar, esta cópia antiga também fecha...
    sys.exit()


# Associa cada opção numérica do menu à função que executa sua ação...
ACOES = {
    "01": acao_gravar,
    "02": acao_trocar_palavra,
    "03": acao_modo_pasta,
    "04": acao_trocar_porta,
    "05": acao_contar,
    "06": acao_executar,
}

# Desenha o menu, normaliza a opção informada e executa a ação correspondente...
# A opção 00 encerra o ciclo; entradas desconhecidas deixam uma mensagem de erro para a próxima tela...
while True:
    desenhar_tela()
    opcao = input(f"{VERDE}INFORME UMA OPCAO: {RESET}").strip().zfill(2)
    # Limpa a mensagem da ação anterior...
    estado["mensagem"] = ""
    if opcao == "00":
        break
    if opcao in ACOES:
        ACOES[opcao]()
    else:
        estado["mensagem"] = f"{VERMELHO}Opcao invalida.{RESET}"

# Fecha a porta antes de sair...
if estado["conexao"]:
    estado["conexao"].close()
