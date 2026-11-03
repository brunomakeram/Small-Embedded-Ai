# pos_gravacao.py
# Menu que aparece logo depois de gravar: ouvir, salvar ou excluir a gravação que acabou de ser feita.
# O arquivo já foi escrito na pasta antes deste menu, então Salvar só mantém e Excluir apaga.

# Só existe no Windows, toca WAV sem precisar instalar nada
import winsound

from arquivos import excluir_wav
from config import AMARELO, AZUL, BRANCO, RESET, VERDE, VERMELHO, estado
from tela import linha_caixa


# Mostra as 3 opções e repete até escolher Salvar ou Excluir, Ouvir pode ser usado quantas vezes quiser
def menu_pos_gravacao(caminho):
    while True:
        print(f"{AMARELO}Gravado: {caminho.name}{RESET}")
        linha_caixa(("[O]", AZUL), (" • ", BRANCO), ("OUVIR", AMARELO))
        linha_caixa(("[S]", AZUL), (" • ", BRANCO), ("SALVAR", AMARELO))
        linha_caixa(("[E]", AZUL), (" • ", BRANCO), ("EXCLUIR", AMARELO))
        opcao = input(f"{VERDE}Escolha: {RESET}").strip().upper()
        if opcao == "O":
            # Toca o arquivo e só volta quando acabar, são 2 segundos
            winsound.PlaySound(str(caminho), winsound.SND_FILENAME)
        elif opcao == "S":
            estado["mensagem"] = f"{VERDE}Salvo: {caminho}{RESET}"
            return
        elif opcao == "E":
            excluir_wav(caminho)
            estado["mensagem"] = f"{VERMELHO}Excluido: {caminho.name}{RESET}"
            return
        else:
            print(f"{VERMELHO}Opcao invalida.{RESET}")
