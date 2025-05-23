# Methods

Diretrizes para organizar o pacote de novos métodos.

## Pastas
O script do método deve estar inserido na pasta de seu objeto com o nome methods.src. Ex.: Caso seja um item, estará na pasta ../methods/Items. Caso o objeto ainda não tenha pasta, criar uma com o nome do objeto (o mesmo que está no Documentation do POL).

Os objetos que forem herança de outro, deve seguir a mesma representação de herança dos objetos em suas pastas. Ex.: Weapon herda de Equipment que herda de Item, portanto a pasta de Weapon será: ../Item/Equipment/Weapon/methods.src

### Estrutura:
```
../methods
|
|--- Item/
|	 |--- Armor/
|	 |    |--- methods.src
|	 |--- Weapon/
|	 |	  |--- methods.src
|	 |--- methods.src
|
|--- [OBJETO]
	 |--- methods.src
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

Devido a um bug grave no POL, na chamada de vários objetos e syshook.cfg, a estrutura de métodos estão concentrados neste pacote. Para adicionar novos métodos, crie a pasta methods\ dentro do pacote que você está trabalhando, adicione o arquivo methods.inc (não é .src) e dê include no methods.src (deste pacote) chamando o pacote que você está trabalhando.

Exemplo do objeto Character:

```
program Install()
	print("INSTALLING: Methods for Characters... OK!");
	return 1;
endprogram

include ":[PACOTE]:methods/methods";
include ":[PACOTE]:methods/methods";
```

### Function:

As funções precisam estar com **exported**, para o POL conseguir exportá-la para os demais scripts. Ex.:
```
exported function getType(item)
	var elem := getItemCfgElem(item);
	return elem.TipoAtaque ? elem.TipoAtaque : error{"errortext" := "The item have no Attack Type!"};
endfunction
```
