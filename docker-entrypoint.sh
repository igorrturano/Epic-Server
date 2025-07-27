#!/bin/bash

# Script de entrada para o container POL
# Configura o ambiente e inicia o servidor

set -e

echo "=== Inicializando servidor POL ==="

# Função para mostrar ajuda
show_help() {
    echo "Uso: $0 [OPÇÕES]"
    echo ""
    echo "Opções:"
    echo "  --uo-data PATH    Caminho para os dados UO (padrão: /opt/uo-data)"
    echo "  --help            Mostrar esta ajuda"
    echo ""
    echo "Exemplo:"
    echo "  $0 --uo-data /home/user/UO/Epic"
    echo ""
}

# Variáveis padrão
UO_DATA_PATH="/opt/uo-data"
POL_UO_DATA_ROOT="/opt/uo-data"

# Processar argumentos
<<<<<<< HEAD
POL_COMMAND="/opt/pol/pol"
=======
>>>>>>> 53ce792e806ec0daae7e97f25b3637f9def08d26
while [[ $# -gt 0 ]]; do
    case $1 in
        --uo-data)
            UO_DATA_PATH="$2"
            shift 2
            ;;
        --help)
            show_help
            exit 0
            ;;
<<<<<<< HEAD
        /opt/pol/pol)
            # Default command, just store it
            POL_COMMAND="$1"
            shift
            ;;
        *)
            # Pass unknown arguments to POL
            break
=======
        *)
            echo "Argumento desconhecido: $1"
            show_help
            exit 1
>>>>>>> 53ce792e806ec0daae7e97f25b3637f9def08d26
            ;;
    esac
done

echo "Caminho dos dados UO: $UO_DATA_PATH"

# Verificar se os diretórios necessários existem
echo "Verificando estrutura de diretórios..."

# Criar diretórios se não existirem
mkdir -p /opt/pol/realm
mkdir -p /opt/pol/data
mkdir -p /opt/pol/logs
mkdir -p /opt/uo-data

# Função para copiar dados UO
copy_uo_data() {
    local source_path="$1"
    local target_path="/opt/uo-data"
    
    echo "Copiando dados UO de $source_path para $target_path..."
    
    if [ -d "$source_path" ]; then
        # Verificar se já existem dados no destino
        if [ "$(ls -A $target_path 2>/dev/null)" ]; then
            echo "AVISO: Diretório de destino não está vazio. Pulando cópia."
            return 0
        fi
        
        # Copiar arquivos necessários
        echo "Copiando arquivos UO..."
        
        # Lista de arquivos/diretórios importantes para o POL
        local important_files=(
            "map0.mul"
            "map1.mul"
            "map2.mul"
            "map3.mul"
            "map4.mul"
            "map5.mul"
            "staidx0.mul"
            "staidx1.mul"
            "staidx2.mul"
            "staidx3.mul"
            "staidx4.mul"
            "staidx5.mul"
            "statics0.mul"
            "statics1.mul"
            "statics2.mul"
            "statics3.mul"
            "statics4.mul"
            "statics5.mul"
            "tiledata.mul"
            "multi.mul"
            "multi.idx"
            "anim.mul"
            "anim.idx"
            "animdata.mul"
            "animindex.mul"
            "hues.mul"
            "verdata.mul"
            "radarcol.mul"
            "mapdif0.mul"
            "mapdif1.mul"
            "mapdif2.mul"
            "mapdif3.mul"
            "mapdif4.mul"
            "mapdif5.mul"
            "stadif0.mul"
            "stadif1.mul"
            "stadif2.mul"
            "stadif3.mul"
            "stadif4.mul"
            "stadif5.mul"
        )
        
        # Copiar arquivos importantes
        for file in "${important_files[@]}"; do
            if [ -f "$source_path/$file" ]; then
                echo "Copiando $file..."
                cp "$source_path/$file" "$target_path/"
            fi
        done
        
        # Copiar diretórios importantes
        local important_dirs=(
            "gumpidx"
            "gumpart"
            "fonts"
            "speech"
            "music"
            "sound"
        )
        
        for dir in "${important_dirs[@]}"; do
            if [ -d "$source_path/$dir" ]; then
                echo "Copiando diretório $dir..."
                cp -r "$source_path/$dir" "$target_path/"
            fi
        done
        
        echo "Cópia dos dados UO concluída!"
        
    else
        echo "ERRO: Diretório de dados UO não encontrado em $source_path"
        echo "Certifique-se de que o caminho está correto e os dados UO estão disponíveis"
        return 1
    fi
}

# Verificar e copiar dados UO se necessário
if [ "$UO_DATA_PATH" != "/opt/uo-data" ]; then
    echo "Caminho personalizado de dados UO detectado: $UO_DATA_PATH"
    copy_uo_data "$UO_DATA_PATH"
else
    # Verificar se os dados UO estão disponíveis no local padrão
    if [ ! -d "/opt/uo-data" ] || [ -z "$(ls -A /opt/uo-data 2>/dev/null)" ]; then
        echo "AVISO: Diretório de dados UO não encontrado ou vazio em /opt/uo-data"
        echo "Certifique-se de montar os dados UO como volume ou especificar --uo-data"
    else
        echo "Dados UO encontrados em /opt/uo-data"
    fi
fi

# Verificar se o executável POL existe
if [ ! -f "/opt/pol/pol" ]; then
    echo "ERRO: Executável POL não encontrado!"
    exit 1
else
    echo "Executável POL encontrado!"
    # Compilar scripts com flag -u (update)
    echo "Compilando scripts com ecompile -u..."
    cd /opt/pol/scripts
    ./ecompile -u || echo "AVISO: Falha na compilação de scripts, continuando com os existentes"
    cd /opt/pol
fi

# Verificar e ajustar configuração do UO data path
echo "Configurando caminho dos dados UO..."
if [ -f "/opt/pol/pol.cfg" ]; then
    # Criar symlink para que pol.cfg possa encontrar uo-data como caminho relativo
    if [ ! -L "/opt/pol/uo-data" ]; then
        ln -sf /opt/uo-data /opt/pol/uo-data
        echo "Symlink criado: /opt/pol/uo-data -> /opt/uo-data"
    fi
    echo "pol.cfg encontrado, configuração UO data configurada corretamente"
else
    echo "ERRO: pol.cfg não encontrado!"
    exit 1
fi

# Configurar permissões apenas para diretórios que não são volumes somente leitura
echo "Configurando permissões..."
chown -R pol:pol /opt/pol/realm /opt/pol/data /opt/pol/logs /opt/uo-data 2>/dev/null || true

# Mostrar informações do sistema
echo ""
echo "=== Informações do Sistema ==="
echo "Portas expostas:"
echo "  - 5003: Cliente UO (7.0.60)"
echo "  - 5085: Cliente UO (7.0.85)"
echo "  - 3008: Servidor Web"
echo "  - 5001: Debug"
echo "  - 5002: Debug DAP"
echo ""
echo "Dados UO: $UO_DATA_PATH -> /opt/uo-data"
echo ""
echo "=== Comandos Úteis ==="
echo "Para acessar o console do servidor:"
echo "  docker exec -it epic-pol-server /bin/bash"
echo ""
<<<<<<< HEAD
echo "Para recompilar scripts:"
echo "  docker exec -it epic-pol-server bash -c 'cd /opt/pol/scripts && ./ecompile -u'"
echo ""
echo "Para executar uoconvert (após atualizar uo-data):"
echo "  docker exec -it epic-pol-server bash -c 'cd /opt/pol && ./uoconvert.sh'"
=======
echo "Para executar comandos no console:"
echo "  docker exec -it epic-pol-server /opt/pol/pol"
>>>>>>> 53ce792e806ec0daae7e97f25b3637f9def08d26
echo ""
echo "Para ver logs em tempo real:"
echo "  docker logs -f epic-pol-server"
echo ""

# Função para tratamento de sinais
cleanup() {
    echo ""
    echo "Recebido sinal de parada. Salvando mundo..."
    # Aqui você pode adicionar comandos para salvar o mundo
    echo "Servidor POL finalizado."
    exit 0
}

# Configurar trap para sinais
trap cleanup SIGTERM SIGINT

<<<<<<< HEAD
# Check if we should start POL server automatically
if [ "$1" = "start-pol" ]; then
    echo "Iniciando servidor POL..."
    echo "Pressione Ctrl+C para parar o servidor"
    echo ""
    shift
    exec "$POL_COMMAND" "$@"
else
    echo "=== Container Epic POL pronto! ==="
    echo ""
    echo "Para iniciar o servidor POL:"
    echo "  docker exec -it epic-pol-server /opt/pol/pol"
    echo ""
    echo "Para executar uoconvert:"
    echo "  docker exec -it epic-pol-server bash -c 'cd /opt/pol && ./uoconvert.sh'"
    echo ""
    echo "Para recompilar scripts:"
    echo "  docker exec -it epic-pol-server bash -c 'cd /opt/pol/scripts && ./ecompile -u'"
    echo ""
    echo "Container ficará rodando em background..."
    
    # Keep container running
    tail -f /dev/null
fi 
=======
# Iniciar o servidor POL
echo "Iniciando servidor POL..."
echo "Pressione Ctrl+C para parar o servidor"
echo ""

# Executar o servidor POL
exec /opt/pol/pol "$@" 
>>>>>>> 53ce792e806ec0daae7e97f25b3637f9def08d26
