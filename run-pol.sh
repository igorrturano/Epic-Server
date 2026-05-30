#!/bin/bash

# Script para executar o servidor POL com dados UO personalizados
# Epic Server - POL (Penultima Online)

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Função para mostrar ajuda
show_help() {
    echo "Uso: $0 [OPÇÕES]"
    echo ""
    echo "Opções:"
    echo "  --uo-data PATH    Caminho para os dados UO"
    echo "  --build           Reconstruir a imagem Docker"
    echo "  --logs            Mostrar logs em tempo real"
    echo "  --console         Acessar console do servidor"
    echo "  --stop            Parar o servidor"
    echo "  --clean           Forçar limpeza de containers existentes"
    echo "  --help            Mostrar esta ajuda"
    echo ""
    echo "Exemplos:"
    echo "  $0 --uo-data /home/user/UO/Epic"
    echo "  $0 --uo-data /mnt/external/UO --build"
    echo "  $0 --logs"
    echo ""
}

# Variáveis
UO_DATA_PATH=""
BUILD_IMAGE=false
SHOW_LOGS=false
ACCESS_CONSOLE=false
STOP_SERVER=false
FORCE_CLEAN=false
CONTAINER_NAME="epic-pol-server"

# Processar argumentos
while [[ $# -gt 0 ]]; do
    case $1 in
        --uo-data)
            UO_DATA_PATH="$2"
            shift 2
            ;;
        --build)
            BUILD_IMAGE=true
            shift
            ;;
        --logs)
            SHOW_LOGS=true
            shift
            ;;
        --console)
            ACCESS_CONSOLE=true
            shift
            ;;
        --stop)
            STOP_SERVER=true
            shift
            ;;
        --clean)
            FORCE_CLEAN=true
            shift
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            echo "Argumento desconhecido: $1"
            show_help
            exit 1
            ;;
    esac
done

# Verificar se Docker está instalado
if ! command -v docker &> /dev/null; then
    print_error "Docker não está instalado!"
    exit 1
fi

# Função para parar servidor
stop_server() {
    print_info "Parando servidor POL..."
    if docker ps -a -q -f name="$CONTAINER_NAME" | grep -q .; then
        docker stop "$CONTAINER_NAME" 2>/dev/null || true
        docker rm "$CONTAINER_NAME" 2>/dev/null || true
        print_success "Servidor parado e container removido"
    else
        print_warning "Container não existe"
    fi
}

# Função para limpeza forçada
force_clean() {
    print_info "Limpando containers existentes..."
    if docker ps -a -q -f name="$CONTAINER_NAME" | grep -q .; then
        docker stop "$CONTAINER_NAME" 2>/dev/null || true
        docker rm "$CONTAINER_NAME" 2>/dev/null || true
        print_success "Container removido"
    else
        print_info "Nenhum container para remover"
    fi
}

# Função para mostrar logs
show_logs() {
    print_info "Mostrando logs do servidor..."
    if docker ps -q -f name="$CONTAINER_NAME" | grep -q .; then
        docker logs -f "$CONTAINER_NAME"
    else
        print_error "Container não está rodando"
        exit 1
    fi
}

# Função para acessar console
access_console() {
    print_info "Acessando console do servidor..."
    if docker ps -q -f name="$CONTAINER_NAME" | grep -q .; then
        docker exec -it "$CONTAINER_NAME" /bin/bash
    else
        print_error "Container não está rodando"
        exit 1
    fi
}

# Função para construir imagem
build_image() {
    print_info "Construindo imagem Docker..."
    
    # Limpar containers órfãos com o mesmo nome
    if docker ps -a -q -f name="$CONTAINER_NAME" | grep -q .; then
        print_info "Limpando container existente antes da construção..."
        docker stop "$CONTAINER_NAME" 2>/dev/null || true
        docker rm "$CONTAINER_NAME" 2>/dev/null || true
    fi
    
    if docker build -t epic-pol-server .; then
        print_success "Imagem construída com sucesso"
    else
        print_error "Erro ao construir imagem"
        exit 1
    fi
}

# Função para iniciar servidor
start_server() {
    print_info "Iniciando servidor POL..."
    
    # Se foi fornecido um caminho personalizado para dados UO
    if [ -n "$UO_DATA_PATH" ]; then
        print_info "Usando dados UO de: $UO_DATA_PATH"
        
        # Verificar se o caminho existe
        if [ ! -d "$UO_DATA_PATH" ]; then
            print_error "Diretório de dados UO não encontrado: $UO_DATA_PATH"
            exit 1
        fi
    else
        print_warning "Usando caminho padrão para dados UO: /home/filipe/UO/Epic"
        print_warning "Use --uo-data para especificar um caminho personalizado"
        UO_DATA_PATH="/home/filipe/UO/Epic"
    fi
    
    # Criar diretórios necessários
    mkdir -p realm data logs
    
    # Parar e remover container se já existir (rodando ou não)
    if docker ps -a -q -f name="$CONTAINER_NAME" | grep -q .; then
        print_info "Removendo container existente..."
        docker stop "$CONTAINER_NAME" 2>/dev/null || true
        docker rm "$CONTAINER_NAME" 2>/dev/null || true
        print_success "Container removido"
    fi
    
    # Construir imagem se necessário
    if [ "$BUILD_IMAGE" = true ] || ! docker images epic-pol-server | grep -q epic-pol-server; then
        build_image
    fi
    
    # Executar container
    print_info "Executando container..."
    
    docker run -d \
        --name "$CONTAINER_NAME" \
        --restart unless-stopped \
        -p 5003:5003 \
        -p 5085:5085 \
        -p 3008:3008 \
        -p 5001:5001 \
        -p 5002:5002 \
        -v "$(pwd)/realm:/opt/pol/realm" \
        -v "$(pwd)/data:/opt/pol/data" \
        -v "$(pwd)/logs:/opt/pol/logs" \
        -v "$UO_DATA_PATH:/tmp/uo-data:ro" \
        -v "$(pwd)/config:/opt/pol/config:ro" \
        -e POL_UO_DATA_ROOT=/opt/uo-data \
        -e POL_REALM_DATA_PATH=/opt/pol/realm \
        -e POL_WORLD_DATA_PATH=/opt/pol/data \
        epic-pol-server \
        --uo-data /tmp/uo-data
    
    if [ $? -eq 0 ]; then
        print_success "Servidor iniciado com sucesso"
        
        echo ""
        print_info "Informações do servidor:"
        echo "  - Porta 5003: Cliente UO (7.0.60)"
        echo "  - Porta 5085: Cliente UO (7.0.85)"
        echo "  - Porta 3008: Servidor Web"
        echo "  - Porta 5001: Debug"
        echo "  - Porta 5002: Debug DAP"
        echo ""
        print_info "Comandos úteis:"
        echo "  - Ver logs: $0 --logs"
        echo "  - Acessar console: $0 --console"
        echo "  - Parar servidor: $0 --stop"
        echo ""
        
    else
        print_error "Erro ao iniciar servidor"
        exit 1
    fi
}

# Executar ações baseadas nos argumentos
if [ "$FORCE_CLEAN" = true ]; then
    force_clean
elif [ "$STOP_SERVER" = true ]; then
    stop_server
elif [ "$SHOW_LOGS" = true ]; then
    show_logs
elif [ "$ACCESS_CONSOLE" = true ]; then
    access_console
elif [ "$BUILD_IMAGE" = true ]; then
    build_image
    if [ -n "$UO_DATA_PATH" ]; then
        start_server
    fi
else
    # Modo padrão: iniciar servidor
    if [ "$BUILD_IMAGE" = true ]; then
        build_image
    fi
    start_server
fi 