# Ferramenta de linha de comando para transferir modelos Seai entre a placa e o computador...
# O subcomando receber captura o CSV emitido por modelo.exportar() e grava os bytes sem alterar o formato...
# O subcomando enviar valida os marcadores do arquivo, aguarda modelo.importar() e transmite o CSV original...
# A comunicação usa PySerial, 115200 baud e os mesmos marcadores definidos pelo firmware...

import argparse
import sys
import time
from pathlib import Path

import serial
from serial.tools import list_ports

# Delimitam o bloco CSV completo para separar o modelo dos logs enviados pela placa...
MARCADOR_INICIO = b"SEAI_INICIO"
MARCADOR_FIM = b"SEAI_FIM"
# Confirmam que a placa está pronta e informam o resultado da gravação do modelo...
MARCADOR_PRONTO = b"SEAI_PRONTO_IMPORTAR"
MARCADOR_SUCESSO = b"SEAI_IMPORTACAO_OK"
MARCADOR_ERRO = b"SEAI_IMPORTACAO_ERRO"
# Mantém a velocidade serial igual à usada pelo exemplo de transferência...
VELOCIDADE = 115200
# Define o limite de espera para receber o modelo ou a confirmação final da placa...
TEMPO_LIMITE = 120


def escolher_porta(porta):
    # Devolve a porta indicada na linha de comando ou apresenta as portas detectadas para seleção...
    # porta pode ser um nome como COM4; sem uma escolha válida, a função informa o erro ao usuário...
    if porta:
        return porta

    # Armazena as portas serial disponíveis para apresentar uma seleção ao usuário....
    portas = list(list_ports.comports())
    if not portas:
        raise RuntimeError("Nenhuma porta serial foi encontrada.")

    # Numera as portas a partir de um para que a escolha seja natural na linha de comando....
    for indice, item in enumerate(portas, start=1):
        print(f"[{indice}] {item.device} - {item.description}")

    while True:
        # Lê a opção digitada e remove espaços externos antes de convertê-la....
        escolha = input("Selecione o numero da porta: ").strip()
        try:
            # Converte a opção para o índice que será validado contra a lista de portas....
            indice = int(escolha)
        except ValueError:
            print("Informe o numero correspondente a uma porta.")
            continue
        if 1 <= indice <= len(portas):
            return portas[indice - 1].device
        print("Numero de porta invalido.")


def ler_ate(conexao, marcadores, prazo):
    # Lê linhas UTF-8 da conexão até localizar um marcador esperado ou esgotar o prazo...
    # conexao é uma porta serial aberta; marcadores contém as respostas aceitas e prazo está em segundos...
    # Linhas intermediárias são exibidas como mensagens da placa; o marcador encontrado é devolvido...
    # Calcula o instante final de leitura sem depender de alterações no relógio do sistema....
    limite = time.monotonic() + prazo
    while time.monotonic() < limite:
        # Lê uma linha serial; o timeout da conexão permite testar novamente até o prazo final....
        linha = conexao.readline()
        if linha:
            # Remove espaços e finais de linha antes de comparar os marcadores....
            texto = linha.strip()
            if texto in marcadores:
                return texto
            print(linha.decode("utf-8", errors="replace").rstrip())
    raise TimeoutError("Tempo limite aguardando resposta da placa.")


def receber(args):
    # Solicita ao sketch que execute modelo.exportar() e salva exatamente o bloco CSV recebido...
    # args.porta seleciona a conexão e args.saida define o arquivo local, padrão modelo.csv...
    # A espera inicial permite que uma placa reiniciada ao abrir a porta termine a inicialização...
    # Resolve a porta serial selecionada pela opção ou pelo menu interativo....
    porta = escolher_porta(args.porta)
    # Converte o caminho solicitado em um objeto usado para gravar o CSV recebido....
    destino = Path(args.saida)
    # Abre a conexão com velocidade e limites compatíveis com o firmware....
    with serial.Serial(porta, VELOCIDADE, timeout=1, write_timeout=5) as conexao:
        conexao.reset_input_buffer()
        time.sleep(2)
        conexao.write(b"E\n")
        conexao.flush()
        print("Aguardando o modelo enviado pela placa...")

        # Acumula o marcador inicial, as linhas do modelo e o marcador final sem reformatar os bytes....
        linhas = []
        # Define o prazo total da captura do CSV emitido pela placa....
        limite = time.monotonic() + TEMPO_LIMITE
        # Indica se o bloco do modelo já começou; logs anteriores não pertencem ao arquivo....
        iniciado = False
        # Ignora logs anteriores ao marcador inicial e acumula as linhas até o marcador final...
        while time.monotonic() < limite:
            # Recebe uma linha completa da Serial, ignorando leituras vazias até o limite....
            linha = conexao.readline()
            if not linha:
                continue
            # Normaliza apenas a linha de controle para reconhecer os marcadores do protocolo....
            marcador = linha.strip()
            if not iniciado:
                if marcador == MARCADOR_INICIO:
                    linhas.append(linha)
                    iniciado = True
                continue
            linhas.append(linha)
            if marcador == MARCADOR_FIM:
                destino.write_bytes(b"".join(linhas))
                print(f"Modelo salvo em: {destino}")
                return

    raise TimeoutError("Tempo limite aguardando o fim da exportacao do modelo.")


def enviar(args):
    # Envia à placa um arquivo recebido anteriormente, sem reinterpretar nem reformatar os números CSV...
    # args.arquivo identifica a origem; args.porta pode selecionar a conexão sem menu interativo...
    # O arquivo precisa começar em SEAI_INICIO e terminar em SEAI_FIM antes de abrir a serial...
    # Converte o caminho de entrada para leitura binária sem alterar a codificação do CSV....
    origem = Path(args.arquivo)
    # Carrega todos os bytes que serão enviados e preserva exatamente o formato exportado....
    dados = origem.read_bytes()
    # Divide uma cópia lógica por linhas para validar apenas os marcadores extremos....
    linhas = dados.splitlines()
    if not linhas or linhas[0].strip() != MARCADOR_INICIO or linhas[-1].strip() != MARCADOR_FIM:
        raise ValueError("O arquivo nao contem os marcadores SEAI_INICIO e SEAI_FIM.")

    # Resolve a porta serial escolhida pelo usuário ou informada na linha de comando....
    porta = escolher_porta(args.porta)
    # Abre a conexão e aguarda a placa anunciar que está pronta antes de transmitir....
    with serial.Serial(porta, VELOCIDADE, timeout=1, write_timeout=5) as conexao:
        conexao.reset_input_buffer()
        time.sleep(2)
        conexao.write(b"I\n")
        conexao.flush()
        print("Aguardando a placa ficar pronta para importar...")
        # Só transmite os bytes depois que modelo.importar() sinaliza que iniciou a leitura...
        ler_ate(conexao, {MARCADOR_PRONTO}, TEMPO_LIMITE)

        conexao.write(dados)
        conexao.flush()
        print("CSV enviado; aguardando validacao e gravacao...")
        # Uma recusa do firmware vira erro explícito; apenas a confirmação autoriza sucesso...
        # Aguarda uma confirmação de sucesso ou uma recusa explícita do firmware....
        resultado = ler_ate(
            conexao,
            {MARCADOR_SUCESSO, MARCADOR_ERRO},
            TEMPO_LIMITE,
        )
        if resultado == MARCADOR_ERRO:
            raise RuntimeError("A placa recusou o modelo; consulte as mensagens acima.")
        print("Modelo importado e gravado na placa.")


def criar_argumentos():
    # Constrói a interface CLI: receber grava um CSV local e enviar transmite um CSV existente...
    # As opções --porta e --saida evitam perguntas interativas quando o fluxo é automatizado...
    # Cria o parser principal e exige a escolha de um dos comandos disponíveis....
    parser = argparse.ArgumentParser(description="Transfere modelos Seai pela Serial.")
    # Registra os subcomandos receber e enviar como entradas obrigatórias da CLI....
    subcomandos = parser.add_subparsers(dest="comando", required=True)

    # Configura as opções do comando que captura um modelo enviado pela placa....
    comando_receber = subcomandos.add_parser("receber", help="Recebe um modelo da placa.")
    comando_receber.add_argument("--porta", help="Porta serial da placa.")
    comando_receber.add_argument(
        "--saida",
        default="modelo.csv",
        help="Arquivo CSV de destino (padrao: modelo.csv).",
    )
    comando_receber.set_defaults(executar=receber)

    # Configura as opções do comando que transmite um CSV com marcadores válidos para a placa....
    comando_enviar = subcomandos.add_parser("enviar", help="Envia um arquivo CSV para a placa.")
    comando_enviar.add_argument("arquivo", help="Arquivo CSV exportado anteriormente.")
    comando_enviar.add_argument("--porta", help="Porta serial da placa.")
    comando_enviar.set_defaults(executar=enviar)
    return parser


def principal():
    # Analisa os argumentos, executa o subcomando escolhido e converte falhas operacionais em código de saída...
    # Erros de arquivo, serial, tempo limite e formato são exibidos em stderr e retornam status 1...
    # Analisa a linha de comando e prepara os argumentos do subcomando selecionado....
    argumentos = criar_argumentos().parse_args()
    try:
        argumentos.executar(argumentos)
    except (OSError, serial.SerialException, TimeoutError, RuntimeError, ValueError) as erro:
        print(f"Erro: {erro}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(principal())
