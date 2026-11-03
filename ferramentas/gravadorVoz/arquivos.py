# arquivos.py
# Tudo que mexe com os arquivos WAV na pasta Audios: contar, salvar e excluir.

from config import PASTA_BASE, estado


# Conta quantos arquivos WAV existem em toda a pasta de gravações
def contar_gravacoes():
    # Se a pasta ainda não existe, não tem nenhuma gravação
    if not PASTA_BASE.exists():
        return 0
    return len(list(PASTA_BASE.rglob("*.wav")))


# Conta as gravações de cada palavra, devolve um dicionário tipo {"ligar": 4, "desligar": 2}
def contar_por_palavra():
    contagem = {}
    if PASTA_BASE.exists():
        for arquivo in PASTA_BASE.rglob("*.wav"):
            # Tudo antes do último underline é a palavra
            palavra = arquivo.stem.rsplit("_", 1)[0]
            contagem[palavra] = contagem.get(palavra, 0) + 1
    return contagem


# Salva os bytes recebidos da placa como o próximo arquivo da palavra e devolve o caminho dele
def salvar_wav(dados):
    palavra = estado["palavra"]
    # Pasta única ou subpasta com o nome da palavra
    pasta = PASTA_BASE if estado["modo_pasta"] == 1 else PASTA_BASE / palavra
    pasta.mkdir(parents=True, exist_ok=True)
    # O número vem da contagem de arquivos, por isso só é seguro excluir a última gravação
    # Se apagar uma do meio, a próxima sobrescreve outra que já existe
    numero = len(list(pasta.glob(f"{palavra}_*.wav"))) + 1
    caminho = pasta / f"{palavra}_{numero:03d}.wav"
    # Os bytes já são um WAV completo, só grava
    caminho.write_bytes(dados)
    return caminho


# Apaga o arquivo da gravação que acabou de ser feita
def excluir_wav(caminho):
    caminho.unlink()
