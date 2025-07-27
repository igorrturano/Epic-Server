#!/bin/bash

# Epic POL Server - Docker Management Script

set -e

echo "🚀 Epic POL Server Management"
echo "=============================="

# Function to show help
show_help() {
    echo "Usage: $0 [COMMAND]"
    echo ""
    echo "Container Management:"
    echo "  start         - Start Epic POL container"
    echo "  stop          - Stop Epic POL container"
    echo "  restart       - Restart Epic POL container"
    echo "  shell         - Access container shell"
    echo "  build         - Rebuild Docker image"
    echo "  status        - Show container status"
    echo ""
    echo "POL Server Management:"
    echo "  start-pol     - Start POL server (container must be running)"
    echo "  stop-pol      - Stop POL server (keep container running)"
    echo "  pol-logs      - Show POL server logs"
    echo ""
    echo "Development Tools:" 
    echo "  uoconvert     - Run uoconvert to generate realm data"
    echo "  compile       - Compile scripts with ecompile -u"
    echo "  logs          - Show container logs"
    echo ""
    echo "  help          - Show this help"
    echo ""
}

# Function to check if Docker is running
check_docker() {
    if ! docker info >/dev/null 2>&1; then
        echo "❌ Docker is not running. Please start Docker first."
        exit 1
    fi
}

# Function to build the image
build_image() {
    echo "🔨 Building Epic POL Server image..."
    docker build --platform linux/amd64 -t epic-pol-server .
    echo "✅ Image built successfully!"
}

# Function to start the container (not the POL server itself)
start_container() {
    echo "🚀 Starting Epic POL Container (Development Mode)..."
    
    # Use docker-compose for development volumes
    docker-compose down 2>/dev/null || true
    docker-compose up -d
    
    echo "✅ Container started with development volumes!"
    echo "📋 Container Status:"
    docker ps | grep epic-pol-server || echo "❌ Container not running"
    echo ""
    echo "🔧 Next steps:"
    echo "  - Run uoconvert: $0 uoconvert"
    echo "  - Start POL server: $0 start-pol" 
    echo "  - Access shell: $0 shell"
    echo ""
    echo "💻 Development Mode Active:"
    echo "  - Scripts sync in real-time: ./scripts ↔ container"
    echo "  - Packages sync in real-time: ./pkg ↔ container"
    echo "  - Configs sync in real-time: ./config ↔ container"
}

# Function to start POL server inside running container
start_pol_server() {
    echo "🎮 Starting POL Server..."
    
    if ! docker ps | grep -q epic-pol-server; then
        echo "❌ Container not running. Start container first with: $0 start"
        exit 1
    fi
    
    echo "Starting POL server in background..."
    docker exec -d epic-pol-server /opt/pol/docker-entrypoint.sh start-pol
    
    echo "✅ POL server starting!"
    echo "📊 View POL logs: $0 pol-logs"
}

# Function to run uoconvert
run_uoconvert() {
    echo "🗺️ Running UOConvert..."
    
    if ! docker ps | grep -q epic-pol-server; then
        echo "❌ Container not running. Start container first with: $0 start"
        exit 1
    fi
    
    docker exec -it epic-pol-server bash -c 'cd /opt/pol && ./uoconvert.sh'
    echo "✅ UOConvert completed!"
}

# Function to compile scripts
compile_scripts() {
    echo "⚙️ Compiling scripts..."
    
    if ! docker ps | grep -q epic-pol-server; then
        echo "❌ Container not running. Start container first with: $0 start"
        exit 1
    fi
    
    docker exec -it epic-pol-server bash -c 'cd /opt/pol/scripts && ./ecompile -u'
    echo "✅ Scripts compiled!"
}

# Function to stop the server
stop_server() {
    echo "🛑 Stopping Epic POL Server..."
    docker stop epic-pol-server 2>/dev/null || echo "⚠️  Container not running"
    docker rm epic-pol-server 2>/dev/null || echo "⚠️  Container not found"
    echo "✅ Server stopped!"
}

# Function to show logs
show_logs() {
    echo "📋 Epic POL Server Logs:"
    echo "========================"
    docker logs -f epic-pol-server
}

# Function to access shell
access_shell() {
    echo "🐚 Accessing Epic POL Server shell..."
    docker exec -it epic-pol-server /bin/bash
}

# Function to show POL server logs
show_pol_logs() {
    echo "📋 POL Server Logs:"
    echo "=================="
    
    if ! docker ps | grep -q epic-pol-server; then
        echo "❌ Container not running. Start container first with: $0 start"
        exit 1
    fi
    
    # Show recent POL logs by looking for POL process output
    docker exec -it epic-pol-server bash -c 'tail -f /opt/pol/log/pol.log 2>/dev/null || echo "POL log file not found. Server may not be running."'
}

# Function to stop POL server (but keep container running)
stop_pol_server() {
    echo "🛑 Stopping POL Server..."
    
    if ! docker ps | grep -q epic-pol-server; then
        echo "❌ Container not running"
        exit 1
    fi
    
    docker exec -it epic-pol-server bash -c 'pkill -f "/opt/pol/pol" || echo "POL server not running"'
    echo "✅ POL server stopped! Container still running."
}

# Function to show status
show_status() {
    echo "📊 Epic POL Server Status:"
    echo "=========================="
    
    if docker ps | grep -q epic-pol-server; then
        echo "✅ Container is running"
        echo ""
        echo "📋 Container Info:"
        docker ps | grep epic-pol-server
        echo ""
        
        # Check if POL server is running inside container
        pol_running=$(docker exec epic-pol-server bash -c 'pgrep -f "/opt/pol/pol" || echo ""')
        if [ ! -z "$pol_running" ]; then
            echo "🎮 POL Server: ✅ Running (PID: $pol_running)"
        else
            echo "🎮 POL Server: ❌ Not running"
        fi
        
        echo ""
        echo "🌐 Ports:"
        echo "  - 5003: UO Client 7.0.60"
        echo "  - 5085: UO Client 7.0.85" 
        echo "  - 3008: Web Server"
        echo "  - 5001: Debug"
        echo "  - 5002: Debug DAP"
    else
        echo "❌ Container is not running"
    fi
}

# Check command line arguments
case "${1:-help}" in
    start)
        check_docker
        build_image
        start_container
        ;;
    stop)
        check_docker
        stop_server
        ;;
    restart)
        check_docker
        stop_server
        sleep 2
        build_image
        start_container
        ;;
    start-pol)
        check_docker
        start_pol_server
        ;;
    stop-pol)
        check_docker
        stop_pol_server
        ;;
    pol-logs)
        check_docker
        show_pol_logs
        ;;
    uoconvert)
        check_docker
        run_uoconvert
        ;;
    compile)
        check_docker
        compile_scripts
        ;;
    logs)
        check_docker
        show_logs
        ;;
    shell)
        check_docker
        access_shell
        ;;
    build)
        check_docker
        build_image
        ;;
    status)
        check_docker
        show_status
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        echo "❌ Unknown command: $1"
        echo ""
        show_help
        exit 1
        ;;
esac