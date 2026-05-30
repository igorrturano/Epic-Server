@echo off
setlocal enabledelayedexpansion

:: URL do release
set "URL=https://github.com/polserver/polserver/releases/download/NightlyRelease/Nightly-Windows.zip"

:: Nome do arquivo temporário
set "ZIPFILE=Nightly-Windows.zip"

:: Baixar o release
echo Baixando release...
curl -L "%URL%" -o "%ZIPFILE%"

:: Verificar se o download foi bem-sucedido
if exist "%ZIPFILE%" (
    echo Download concluido: %ZIPFILE%
) else (
    echo Erro ao baixar o arquivo.
    exit /b 1
)

:: Descompactar na pasta atual
echo Descompactando...
tar -xf "%ZIPFILE%"

:: Remover o arquivo zip após descompactar
del "%ZIPFILE%"

:: Encontrar o arquivo -bin.zip
set "BINZIP="
for %%f in (*-bin.zip) do (
    set "BINZIP=%%f"
)

if defined BINZIP (
    echo Encontrado: !BINZIP!
    
    :: Criar pasta temporária com o nome do arquivo (sem -bin.zip)
    set "TEMPDIR=!BINZIP:-bin.zip=!"
    if not exist "!TEMPDIR!" mkdir "!TEMPDIR!"
    
    echo Descompactando binarios na pasta temporaria...
    tar -xf "!BINZIP!" -C "!TEMPDIR!"
    
    :: Remover a pasta config da extração temporária
    if exist "!TEMPDIR!\config" (
        echo Removendo pasta config da extracao...
        rmdir /s /q "!TEMPDIR!\config"
    )
    
    :: Copiar conteúdo para a raiz
    echo Copiando arquivos para a pasta atual...
    xcopy "!TEMPDIR!\*" "." /e /y
    
    :: Limpar pasta temporária e zips
    rmdir /s /q "!TEMPDIR!"
    del /q *-bin.zip 2>nul
    del /q *-dbg.zip 2>nul
    
    echo Pronto! Os arquivos foram extraidos na pasta atual.
) else (
    echo Erro: Arquivo -bin.zip nao encontrado.
    pause
    exit /b 1
)

endlocal
pause
