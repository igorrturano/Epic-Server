#!/bin/bash

# URL do release
URL="https://github.com/polserver/polserver/releases/download/NightlyRelease/Nightly-Windows.zip"

# Nome do arquivo temporário
ZIPFILE="Nightly-Windows.zip"

# Baixar o release
echo "Baixando release..."
curl -L "$URL" -o "$ZIPFILE"

# Verificar se o download foi bem-sucedido
if [ -f "$ZIPFILE" ]; then
    echo "Download concluído: $ZIPFILE"
else
    echo "Erro ao baixar o arquivo."
    exit 1
fi

# Descompactar na pasta atual
echo "Descompactando..."
unzip -o "$ZIPFILE" -d .

# Remover o arquivo zip após descompactar
rm "$ZIPFILE"

# Encontrar e descompactar apenas o arquivo -bin.zip
BINZIP=$(ls *-bin.zip 2>/dev/null)

if [ -n "$BINZIP" ]; then
    echo "Encontrado: $BINZIP"
    
    # Criar pasta temporária com o nome do arquivo (sem -bin.zip)
    TEMPDIR="${BINZIP%-bin.zip}"
    mkdir -p "$TEMPDIR"
    
    echo "Descompactando binários na pasta temporária..."
    unzip -o "$BINZIP" -d "$TEMPDIR"
    
    # Remover a pasta config da extração temporária
    if [ -d "$TEMPDIR/config" ]; then
        echo "Removendo pasta config da extração..."
        rm -rf "$TEMPDIR/config"
    fi
    
    # Copiar conteúdo para a raiz
    echo "Copiando arquivos para a pasta atual..."
    cp -r "$TEMPDIR"/* .
    
    # Limpar pasta temporária e zips
    rm -rf "$TEMPDIR"
    rm -f *-bin.zip *-dbg.zip
    
    echo "Pronto! Os arquivos foram extraídos na pasta atual."
else
    echo "Erro: Arquivo -bin.zip não encontrado."
    exit 1
fi