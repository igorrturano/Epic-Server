# Projeto Servidor Ultima Online FLS 2023

Este projeto utiliza a engine POL v100 para criar um servidor de Ultima Online.

## Instruções de Instalação

1. Baixe a versão do POL correspondente ao seu sistema operacional em [POL Server](https://github.com/polserver/polserver/releases).
2. Extraia o arquivo em uma pasta separada.
3. Copie as seguintes pastas e executáveis para a pasta do projeto:
   - pol
   - poltool
   - uoconvert
   - uotool
   - scripts/ecompile
   - scripts/modules/

   **Nota:** Não copie nenhum outro arquivo, pois são arquivos de configuração.

4. Abra o arquivo pol.cfg e altere UoDataFileRoot para o caminho do seu Ultima Online com os arquivos do shard.
5. Se estiver usando Linux, instale a libmysqlclient2 com o comando `apt-get install libmysqlclient2`.
6. Carregue o mapa do servidor usando o uoconvert com os seguintes comandos:
   - `./uoconvert map     realm=britannia mapid=0 usedif=1 width=6144 height=4096`
   - `./uoconvert statics realm=britannia`
   - `./uoconvert maptile realm=britannia`

7. Crie uma cópia da pasta /data_dev/ e renomeie para /data. Altere as informações no arquivo /data/accounts.txt para criar sua conta.
8. Para compilar todos os scripts, abra o terminal na pasta do projeto e execute o comando `scripts/ecompile -a`.

Agora você está pronto para iniciar seu servidor Ultima Online!

## Preparação para Transferência de Arquivos

Este guia abrange a transferência de arquivos para o launcher e host, além da atualização de mapas no servidor.

### Transferindo arquivos para o Launcher:

Para atualizar os arquivos no launcher, siga os comandos abaixo:

```
scp * root@epic-shard.com:~/epic-launcher/Server/patch
ssh root@epic-shard.com
mv ~/epic-launcher/Server/patch/* ~/epic-launcher/Server/'Ultima Online'

depois ps ax, procura o patcher (mono UltimaIPL.Patch.Server.exe -manifest)

kill -9 iddopatcher
cd epic-launcher/Server/
./run.sh
```

Após mover os arquivos:

- Execute `ps ax` para localizar o processo do patcher (procure por `mono UltimaIPL.Patch.Server.exe -manifest`).
- Mate o processo com `kill -9 [id do patcher]` e reinicie o patcher com `./run.sh`.

### Transferindo os arquivos de mapas para o Servidor

Para atualizar os mapas no servidor, utilize os seguintes comandos:

```
scp * root@epic-shard.com:~/epic-shard/muls/newpatch
ssh root@epic-shard.com
mv epic-shard/muls/newpatch/* epic-shard/muls/
cd epic-shard/
./uoconvert.sh
```

Siga as instruções para assegurar que os arquivos sejam corretamente transferidos e atualizados no servidor.


