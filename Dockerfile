# Dockerfile para servidor POL (Penultima Online)
FROM ubuntu:latest

# Evitar prompts interativos durante a instalação
ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependências necessárias
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libmysqlclient-dev \
    libsqlite3-dev \
    libxml2-dev \
    libboost-all-dev \
    libpcre3-dev \
    libz-dev \
    libbz2-dev \
    liblzma-dev \
    libcurl4-openssl-dev \
    libgmp-dev \
    libreadline-dev \
    libncurses5-dev \
    libtinfo-dev \
    python3 \
    python3-pip \
    wget \
    unzip \
    && rm -rf /var/lib/apt/lists/*

# Criar usuário pol para executar o servidor
RUN useradd -m -s /bin/bash pol

# Definir diretório de trabalho
WORKDIR /opt/pol

# Copiar arquivos do projeto
COPY . /opt/pol/

# Dar permissões ao usuário pol
RUN chown -R pol:pol /opt/pol

# Compilar o POL
RUN cd /opt/pol/scripts && ./ecompile -Fi && ./ecompile -u

# Criar diretório para dados UO (será montado como volume)
RUN mkdir -p /opt/uo-data && chown pol:pol /opt/uo-data

# Expor portas necessárias
EXPOSE 5003 5085 3008 5001 5002

# Mudar para usuário pol
USER pol

# Usar o script de entrada personalizado
ENTRYPOINT ["/opt/pol/docker-entrypoint.sh"]

# Comando padrão para executar o servidor
CMD ["/opt/pol/pol"] 