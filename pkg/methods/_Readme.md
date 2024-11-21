# Methods

Diretrizes para organizar o pacote de novos métodos.

## Pastas
O script do método deve estar inserido na pasta de seu objeto com o nome methods.src. Ex.: Caso seja um item, estará na pasta ../methods/Items. Caso o objeto ainda não tenha pasta, criar uma (no plural).

(Futuramente, ver se é melhor separar cada tipo de método para seu respectivo .src. Por enquanto temos poucos métodos criados, então estão todos no arquivo methods.src.)

### Estrutura:
```
../methods
|
|--- Armors/
|	 |--- methods.src
|
|--- Items/
|	 |--- methods.src
|
|--- Weapons/
|	 |--- methods.src
|
|--- [Novo objeto no plural]
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

	[Novo objeto] :methods:[Novo objeto no plural]/methods
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

### Function:

As funções precisam estar com **exported**, para o POL conseguir exportá-la para os demais scripts. Ex.:
```
exported function getType(item)
	var elem := getItemCfgElem(item);
	return elem.TipoAtaque ? elem.TipoAtaque : error{"errortext" := "The item have no Attack Type!"};
endfunction
```
