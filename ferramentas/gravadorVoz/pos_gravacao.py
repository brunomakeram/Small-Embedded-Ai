# Apresenta as ações disponíveis para a amostra WAV já escrita no diretório de gravações...
# Ouvir reproduz o áudio sem sair do menu; Salvar mantém o arquivo e Excluir remove apenas essa amostra...
# O resultado da escolha fica em estado["mensagem"] para a próxima atualização do menu principal...

# Só existe no Windows, toca WAV sem precisar instalar nada...
import winsound

from arquivos import excluir_wav
from config import AMARELO, AZUL, BRANCO, RESET, VERDE, VERMELHO, estado
from tela import linha_caixa


# Repete as opções até o usuário confirmar que mantém ou exclui o arquivo recém-gravado...
# caminho aponta para o WAV salvo; a reprodução pode ser solicitada várias vezes antes da decisão final...
def menu_pos_gravacao(caminho):
    while True:
        print(f"{AMARELO}Gravado: {caminho.name}{RESET}")
        linha_caixa(("[O]", AZUL), (" • ", BRANCO), ("OUVIR", AMARELO))
        linha_caixa(("[S]", AZUL), (" • ", BRANCO), ("SALVAR", AMARELO))
        linha_caixa(("[E]", AZUL), (" • ", BRANCO), ("EXCLUIR", AMARELO))
        opcao = input(f"{VERDE}Escolha: {RESET}").strip().upper()
        if opcao == "O":
            # Toca o arquivo e só volta quando acabar, são 2 segundos...
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
