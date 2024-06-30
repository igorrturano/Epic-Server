use uo;
use datafile;

include ":datafile:datafile";
include ":containers:storageAreas";

const ALL_TAXES := 1;

function CobrarImpostos()

	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	var elems := df.keys();

	var elem;
	var pagamentos;
	foreach  key in elems

		elem := DFFindElement(df, key );
		pagamentos := Cint(elem.GetProp("pagamentos"));
		pagamentos := pagamentos + 1;
		elem.SetProp("pagamentos", pagamentos);

	endforeach


endfunction

/*
 * Retorna o tanto de imposto que um player deve pagar
 */
function GetTaxValue(serial, city)

	var taxes := GetPlayerTaxes(serial);

	var value := 0;

	var keys := taxes.keys();
	foreach key in keys
		value := value + taxes[key];
	endforeach

	var citytaxes := GetCityTaxes(city);
	keys := citytaxes.keys();
	foreach key in keys
		value := value + citytaxes[key];
	endforeach

	return value;

endfunction

/*
 * Pega a divida total do player.
 */

function getDivida(serial, city)

	var valor := GetTaxValue(serial, city);

	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	var elem := DFFindElement(df, CStr(serial), DF_CREATE);

	var pagamentos := CInt(elem.GetProp("pagamentos"));

	var divida := pagamentos * valor;
	divida := divida*(-1);

	if (divida > 0)
		return divida;
	endif

endfunction

/*
 * Retorna os impostos especificos do player.
 */
function GetPlayerTaxes(serial)

	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	var elem := DFFindElement(df, CStr(serial), DF_CREATE);
	return	elem.GetProp("taxes");

endfunction

/*
 * Adiciona um devedor na cidade
 */
function AddDevedor(serial, city)

	//procura o datafile que guarda o quanto cada cidade paga, se nao achar cria
	var df := DFOpenDataFile(":tax:tax", DF_CREATE, DF_KEYTYPE_STRING);
	
	//procura o elem com o nome da cidade, se nao achar cria
	var elem := DFFindElement(df, city, DF_CREATE);

	var devedores := elem.getProp("devedores");
	if ( (!devedores) || (devedores == error) )
		devedores := array;
	endif
	devedores.append(serial);
	elem.SetProp("devedores", devedores);

endfunction

/*
 * Retorna todos os devedores de uma cidade
 */
function getDevedores(city)

	//procura o datafile que guarda o quanto cada cidade paga, se nao achar cria
	var df := DFOpenDataFile(":tax:tax", DF_CREATE, DF_KEYTYPE_STRING);
	
	//procura o elem com o nome da cidade, se nao achar cria
	var elem := DFFindElement(df, city, DF_CREATE);

	var devedores := elem.getProp("devedores");
	return devedores;	

endfunction

/*
 * Deleta o devedor de uma cidade.
 */
function EraseDevedor(serial, city)

	//procura o datafile que guarda o quanto cada cidade paga, se nao achar cria
	var df := DFOpenDataFile(":tax:tax", DF_CREATE, DF_KEYTYPE_STRING);
	
	//procura o elem com o nome da cidade, se nao achar cria
	var elem := DFFindElement(df, city, DF_CREATE);

	var devedores := elem.getProp("devedores");
	devedores.erase(serial);
	
	elem.SetProp("devedores", devedores);

endfunction

/*
 * Verifica quantos meses passaram e faz o pagamento, se nao for possivel
 * adiciona o player na lista de devedores
 */
function payTaxes(serial, city, bankbox)

	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	var elem := DFFindElement(df, CStr(serial), DF_CREATE);

	var pagamentos := CInt(elem.GetProp("pagamentos"));

	if (!pagamentos)
		return 0;
	else
		var total := GetTaxValue(serial, city);
		while (pagamentos > 0)
			if (total < 0 )
				if (ConsumeSubstance( bankbox, 0xff21, total ))
					AddRealmCash(city, total);
					pagamentos := pagamentos - 1;
				else
					AddDevedor(serial, city);
					break;	
				endif
			else
				CreateItemInContainer(bankbox, 0xff21, total);
				var gold := GetCityGold(city);
				setCityGold(city, (gold-total));
				pagamentos := pagamentos - 1;
			endif
		endwhile

		elem.SetProp("pagamentos", pagamentos);
	endif

endfunction

function isDevedor(serial, city)

	var devedores := getDevedores(city);

	foreach devedor in devedores
		if (devedor == serial)
			return 1;
		endif
	endforeach

	return 0;

endfunction

function hasAccount(serial, city)

	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	var elem := DFFindElement(df, CStr(serial), DF_CREATE);

	if (elem.GetProp("cidade") == city)
		return 1;
	else
		return 0;
	endif

endfunction

function getCitzenCity(serial)

	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	var elem := DFFindElement(df, CStr(serial), DF_CREATE);

	if (elem.GetProp("cidade") == error)
		return "Sem cidade";
	else
		return elem.GetProp("cidade");
	endif

endfunction

function getCitzenPayments(serial)

	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	var elem := DFFindElement(df, CStr(serial), DF_CREATE);

	if (elem.GetProp("cidade") == error)
		return 0;
	else
		return elem.GetProp("pagamentos");
	endif

endfunction

function SetupPlayerAccount(serial, city)

	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	var elem := DFFindElement(df, CStr(serial), DF_CREATE);

	elem.SetProp("cidade", city);
	elem.SetProp("pagamentos", 0);
	elem.EraseProp("taxes");

endfunction

function GetCitzens(city)

	var citzens := array;

	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	var elements := df.Keys();

	var citzen;
	foreach elem in elements
		citzen := DFFindElement(df, elem, DF_NO_CREATE);
		if (citzen.GetProp("cidade") == city)
			citzens.append(elem);
		endif
	endforeach

	return citzens;

endfunction

/*
 * Retorna todos os impostos que existem naquela cidade
 */

function GetCityTaxes(city)

	//procura o datafile que guarda o quanto cada cidade paga, se nao achar cria
	var df := DFOpenDataFile(":tax:tax", DF_CREATE, DF_KEYTYPE_STRING);
	
	//procura o elem com o nome da cidade, se nao achar cria
	var elem := DFFindElement(df, city, DF_CREATE);

	//pega o dictionary chamado "taxes"
	var taxes := elem.GetProp("taxes");
	if (taxes == error)
		taxes := dictionary;
		elem.setProp("taxes", taxes);
	endif

	return taxes;

endfunction

/* 
 * Adiciona um imposto na cidade
 */
function AddCityTax(city, tax, value)

	//procura o datafile que guarda o quanto cada cidade paga, se nao achar cria
	var df := DFOpenDataFile(":tax:tax", DF_CREATE, DF_KEYTYPE_STRING);
	
	//procura o elem com o nome da cidade, se nao achar cria
	var elem := DFFindElement(df, city, DF_CREATE);

	//pega o dictionary chamado "taxes"
	var taxes := elem.GetProp("taxes");
	if (taxes == error)
		taxes := dictionary;
		elem.setProp("taxes", taxes);
	endif

	taxes[tax] := value;

	elem.SetProp("taxes", taxes);

endfunction

/*
 * Deleta um imposto da cidade
 */
function DeleteCityTax(city, tax)
	//procura o datafile que guarda o quanto cada cidade paga, se nao achar cria
	var df := DFOpenDataFile(":tax:tax", DF_CREATE, DF_KEYTYPE_STRING);
	
	//procura o elem com o nome da cidade, se nao achar cria
	var elem := DFFindElement(df, city, DF_CREATE);

	//pega o dictionary chamado "taxes"
	var taxes := elem.GetProp( "taxes");

	if (taxes != error)
		taxes.erase(tax);
		elem.SetProp("taxes", taxes);
	endif

endfunction

function DeleteDevedorByName(name, city)

	var devedores := getDevedores(city);
	if (devedores == error)
		return 0;
	endif

	var serials := devedores.keys();
	var char;
	foreach devedor in serials

		char := SystemFindObjectBySerial(devedor);
		if (char != error)
			if (lower(char.name) == lower(name))
				EraseDevedor(devedor, city);
				return 1;
			endif
		endif
		sleepms(5);
	endforeach

	return 0;

endfunction


function getCityGold(city)

	//procura o datafile que guarda o quanto cada cidade paga, se nao achar cria
	var df := DFOpenDataFile(":tax:tax", DF_CREATE, DF_KEYTYPE_STRING);
	
	//procura o elem com o nome da cidade, se nao achar cria
	var elem := DFFindElement(df, city, DF_CREATE);

	//pega o dictionary chamado "taxes"
	var gold := Cint(DFGetProp(elem, "gold", DF_CREATE));	
	
	if ( (!gold ) || (gold == error) )
		return 0;
	else
		return gold;
	endif


endfunction

function getCities()

	var df := DFOpenDataFile(":tax:tax", DF_CREATE, DF_KEYTYPE_STRING);
	var cidades := df.Keys();
	return cidades;

endfunction

function setCityGold(city, value)

	//procura o datafile que guarda o quanto cada cidade paga, se nao achar cria
	var df := DFOpenDataFile(":tax:tax", DF_CREATE, DF_KEYTYPE_STRING);
	
	//procura o elem com o nome da cidade, se nao achar cria
	var elem := DFFindElement(df, city, DF_CREATE);

	//pega o dictionary chamado "taxes"
	var gold := elem.setprop("gold", value);

endfunction

function addRealmCash(city, total)

	//procura o datafile que guarda o quanto cada cidade paga, se nao achar cria
	var df := DFOpenDataFile(":tax:tax", DF_CREATE, DF_KEYTYPE_STRING);
	
	//procura o elem com o nome da cidade, se nao achar cria
	var elem := DFFindElement(df, city, DF_CREATE);

	//pega o dictionary chamado "taxes"
	var taxes := Cint(DFGetProp(elem, "gold", DF_CREATE));	
	taxes := taxes + total;
	DFGetProp(elem, "gold", DF_NO_CREATE, taxes);

endfunction

function GetPlayerCity(serial)

	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	var elem := DFFindElement(df, CStr(serial), DF_CREATE);

	return elem.GetProp("cidade");

endfunction

function RemovePlayer(serial, city)

	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	var elem := DFFindElement(df, CStr(serial), DF_CREATE);

	elem.EraseProp("cidade");
	elem.SetProp("pagamentos", 0);
	elem.EraseProp("taxes");

	var containerID := "Owner="+Hex(serial);
	CP_RemoveStorageAreaContainer(containerID, city);

endfunction

function AddTaxToPlayer(playerSerial, tax)

	//abre o datafile se nao tiver cria
	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	
	//pega o elem do char, se nao tiver cria.
	var elem := DFFindElement(df, CStr(playerSerial), DF_CREATE);

	//pega o dictionary de taxes que o player paga
	var taxes := elem.getProp("taxes");
	if ( (taxes == 0) || (taxes == error) )
		elem.EraseProp("taxes");
		taxes := dictionary;
		elem.SetProp("taxes", taxes );
	endif
	
	var listtax := tax.keys();
	foreach key in listtax
		taxes[key] := tax[key];
	endforeach
	
	elem.SetProp("taxes", taxes);

endfunction

function DeletePlayerTax(playerSerial, all := 0, taxname := "")

	//abre o datafile se nao tiver cria
	var df := DFOpenDataFile(":tax:playerTax", DF_CREATE, DF_KEYTYPE_STRING);
	
	//pega o elem do char, se nao tiver cria.
	var elem := DFFindElement(df, CStr(playerSerial), DF_CREATE);
	
	if (all)
		elem.EraseProp("taxes");
		return;
	else
		var taxes := DFGetProp(elem, "taxes", DF_CREATE);
		taxes.erase(taxname);
		DFGetProp(elem, "taxes", DF_NO_CREATE, taxes);
		return;
	endif

endfunction