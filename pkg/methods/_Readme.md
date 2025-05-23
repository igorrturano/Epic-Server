# Methods

Diretrizes para organizar o pacote de novos métodos.

A explicação aqui é apenas para entender como funciona, pois já deixei todos os pacotes criados seguindo a herança correta.

## Pastas
O script do método deve estar inserido na pasta de seu objeto com o nome methods.src. Ex.: Caso seja um item, estará na pasta ../methods/Items. Caso o objeto ainda não tenha pasta, criar uma com o nome do objeto (o mesmo que está no Documentation do POL).

Os objetos que forem herança de outro, deve seguir a mesma representação de herança dos objetos em suas pastas. Ex.: Weapon herda de Equipment que herda de Item, portanto a pasta de Weapon será: ../Item/Equipment/Weapon/methods.src

### Estrutura:
```
../methods
|-- UObject/
|	|-- Item/
|	|	|-- Armor/
|	|	|   |-- methods.src
|	|	|-- Weapon/
|	|	|	|-- methods.src
|	|	|-- methods.src
|	|
|	|-- [OBJETO]
|	|	|-- methods.src
```

## Funcionamento
Para o correto funcionamento do método, é necessário seguir alguns passos: 

### SystemMethod:

Adicionar o script (sem .src) no arquivo syshook.cfg desta pasta seguindo o padrão. Ex.:

```
SystemMethod
{
	Item :methods:items/methods

	Weapon :methods:weapons/methods

	Armor :methods:armors/methods

	[OBJETO] :methods:[CAMINHO]/methods
}
```

### Install:
Também é ideal colocar no program Install() do script (.src) um print para verificar, no console, se a inicialização do method aconteceu. Ex.:
```
program Install()
	print("INSTALLING: Methods for Items...");
	return 1;
endprogram
```

Devido a um bug grave no POL, na chamada de vários objetos e syshook.cfg, a estrutura de métodos estão concentrados neste pacote.

Para adicionar novos métodos, crie a pasta methods\ dentro do pacote que você está trabalhando, crie um arquivo .inc (não é .src) com o nome do Objeto que os methods serão chamados e adicione um include no methods.src (deste pacote) chamando o pacote que você está trabalhando.

Exemplo para o objeto Character:
```
program Install()
	print("INSTALLING: Methods for Characters... OK!");
	return 1;
endprogram

include ":[PACOTE]:methods/Characters";
include ":[PACOTE2]:methods/Characters";
include ":[PACOTE3]:methods/Characters";
```
<br>

Exemplo para o objeto Item:
```
program Install()
	print("INSTALLING: Methods for Item... OK!");
	return 1;
endprogram

include ":[PACOTE]:methods/Item";
include ":[PACOTE2]:methods/Item";
include ":[PACOTE3]:methods/Item";
```

### Function:

As funções precisam estar com **exported**, para o POL conseguir exportá-la para os demais scripts. Ex.:
```
exported function getType(item)
	var elem := getItemCfgElem(item);
	return elem.TipoAtaque ? elem.TipoAtaque : error{"errortext" := "The item have no Attack Type!"};
endfunction
```
