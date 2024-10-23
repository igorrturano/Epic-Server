use uo;
use cfgfile;

include ":attributes:attributes";
include "include/client";
include "include/say";
include ":chestspawn:treasurechest";
include ":timedScripts:timedScripts";

const MAX_RESOURCE_AMOUNT := 10;

function Garimpeiro(who)
	var rand := RandomFloat(200);
	if(rand < 2)
		EncontraGema(who);
	elseif(TemHabilidade(who, "Minerador Talentoso") && rand < 10)
		EncontraGema(who);
		SendSysMessageEx(who, "So um minerador talentoso acharia essa pedra preciosa!", SSM_INFO);
	endif
	return 1;
endfunction

function EncontraGema(who)
	var cfg := ReadConfigFile(":gems:itemdesc");
	var keys := GetConfigStringKeys( cfg );
	var gema := keys[RandomInt(keys.size())];
	while (!gema)
		gema := keys[RandomInt(keys.size())];
	endwhile
	SendSysMessageEx(who, "Voce achou uma pedra preciosa!", SSM_INFO);
	CreateItemInBackPack(who, gema, 1);
	PrintTextAbove(who, "*encontra algo reluzente*");
	SkillCheck(who, GATHERING, -1);
	return;
endfunction

function GetMapsInBackpack(who)
	var items := EnumerateItemsInContainer(who.backpack);
	var maps := array;
    	foreach item in items
			//sendsysmessage(who, " " + isTreasureMap(item) + " " + item.objtype);
			if ( isTreasureMap(item) )
				if ( !isSolved(item) )
					maps.append(item);
				endif
			endif
			
		endforeach

	return maps;

endfunction

function isTreasureMap(item)

	if ( GetObjProperty(item, "chestx") != error )
		return 1;
	endif

	return 0;

endfunction

function isSolved(map)
    if(GetObjProperty(map, "Solved") == error)

	return 0;
    else
	return 1;

    endif
endfunction

function DigForMap(who, targ, map)

	if (!reserveItem(map))
		return;
	endif

	var chestx := Cint(GetObjProperty( map, "chestx" ));
	var chesty := Cint(GetObjProperty( map, "chesty" ));
var chestz := who.z;

	if((!chestx) || (!chesty))
		SendSysMessage( who, "Este nao parece ser um mapa de tesouro.", SSM_FAIL );
		return;
	endif

	if(!CheckLosAt( who, targ.x, targ.y, targ.z ) );
		SendSysMessageEx( who, "Voce nao ve o alvo.", SSM_FAIL );
		return;
	endif

	PerformAction(who, 0x0b);
	PlaySoundEffect(who, 0x126);

	var range := 5; //GetRange( who );
	var distn := coordinatedistance( targ.x, targ.y, chestx, chesty );
	var maxrange := 20;
	var mobiles := ListMobilesNearLocation(targ.x, targ.y, targ.z, 0, who.realm);

	if(distn > maxrange)
		SendSysMessageEx(who,"Voce parece muito distante.", SSM_INFO);
		return;

	elseif((distn > range) && (distn <= maxrange))
		SendSysMessage(who,"Voce cavou e cavou mas nao encontrou nenhum tesouro.", SSM_INFO);
		return;

	elseif((distn < range) && (len(mobiles) > 0))
		SendSysMessageEx(who,"voce parou de cavar porque algo esta sobre o tesouro.");
		return;
	else
		who.frozen := 1;
		PrintTextAbove(who, "*Comeca a cavar o tesouro*");
		SendSysMessageEx(who, "NAO DESLOGUE, A ANIM DEMORA UM POUCO, aguarde o boneco terminar de cavar!", SSM_INFO);
		var i := 1;
		var mex := who.x;
		var mey := who.y;
		var parts := {};
		var maplevel := CInt(GetObjProperty(map, "level") );
		var loot := "treasuremap" + maplevel;

		var chest;
		chestz := who.z;
		sendsysmessage(who, " " + who.z);
		var chests := CreateTreasureChests(maplevel, 1, 0, chestx, chesty, who.z, who.realm, loot);
		chest := chests[1];
/*		if (GetObjProperty(map, "tesouroescondido") == 1)
			Loot_Generate(chest, loot);
		endif  	*/

		var ret := MoveObjectToLocation(chest, chest.x, chest.y, chest.z, who.realm,  MOVEOBJECT_FORCELOCATION);
//		sendsysmessage(who, " " + ret);

		chest.movable := 0;

		while((i <= 9) && (who.x == mex) && (who.y == mey))
			if((who.x != mex) && (who.y != mey))
				foreach thing in parts
					DestroyItem(thing);
				endforeach
				break;
			endif

			PrintTextAbove(who, "*Cavando*");
			PerformAction(who, 0x0b);
			PlaySoundEffect(who, 0x126);
			sleep(1);

			if(i == 2)
          			var patch := CreateItemAtLocation(chestx, chesty, chestz, 2321, 1, who.realm);
          			patch.movable := 0;
		          	parts.append(patch);
        		elseif(i == 3)
          			var patch := CreateItemAtLocation(chestx, chesty, chestz, 2322, 1, who.realm);
          			patch.movable := 0;
          			parts.append(patch);
        		elseif(i == 4)
          			var patch := CreateItemAtLocation(chestx, chesty, chestz, 2323, 1, who.realm);
          			patch.movable := 0;
          			parts.append(patch);
        		elseif(i == 5)
          			var patch := CreateItemAtLocation(chestx, chesty, chestz, 2324, 1, who.realm);
          			patch.movable := 0;
          			parts.append(patch);
          			chest.movable := 1;
          			MoveObjectToLocation(chest, chest.x, chest.y, (chest.z+2), who.realm,  MOVEOBJECT_FORCELOCATION);
          			chest.movable := 0;
        		elseif(i == 6)
          			var patch := CreateItemAtLocation(chestx, chesty, chestz, 2324, 1, who.realm);
          			patch.movable := 0;
          			parts.append(patch);
          			chest.movable := 1;
          			MoveObjectToLocation(chest, chest.x, chest.y, (chest.z+2), who.realm,  MOVEOBJECT_FORCELOCATION);
          			chest.movable := 0;
        		elseif(i == 7)
          			var patch := CreateItemAtLocation(chestx, chesty, chestz, 2324, 1, who.realm);
          			patch.movable := 0;
          			parts.append(patch);
          			chest.movable := 1;
          			MoveObjectToLocation(chest, chest.x, chest.y,( chest.z+2), who.realm,  MOVEOBJECT_FORCELOCATION);
          			chest.movable := 0;
        		elseif(i == 8)
          			chest.movable := 1;
          			MoveObjectToLocation(chest, chest.x, chest.y, (chest.z+2), who.realm,  MOVEOBJECT_FORCELOCATION);
          			chest.movable := 0;
          			foreach thing in parts
            				DestroyItem(thing);
          			endforeach
        		endif
			if((who.x != mex) || (who.y != mey))
				SendSysMessageEx(who,"Voce deve ficar parado enquanto cava.", SSM_FAIL);
				DestroyItem(chest);
		        	return;
		      	endif

        		i := i + 1;

		endwhile

		who.frozen := 0;
		PrintTextAbove(who, "*Termina de cavar*");
		sleep(5);

		// Para implementar basta compilar o mining.src depois colocar mais mobs para maplevels diferentes há um bug no z, o chest sai voando mas da pra pegar

		if (maplevel == 1)
            PrintTextAbove(who, "*Gritos nos arredores* Atacar rapazes! A pior parte já fizeram!");
			sleep(2);
            createnpcfromtemplate(":brainai:bandoleiro", who.x, who.y, who.z +5, 0 , who.realm);
            createnpcfromtemplate(":brainai:roguearcher", who.x, who.y, who.z +5, 0 , who.realm);
			createnpcfromtemplate(":brainai:trapaceiro", who.x, who.y, who.z +5, 0 , who.realm);
            createnpcfromtemplate(":brainai:saqueadortumulo", who.x, who.y, who.z +5, 0 , who.realm);
            createnpcfromtemplate(":brainai:saqueadortumulo", who.x, who.y, who.z +5, 0 , who.realm);
			sleep(15);
			PrintTextAbove(who, "*O ataque continua e o lider se aproxima*");
			createnpcfromtemplate(":brainai:mercenario", who.x, who.y, who.z +5, 0 , who.realm);
      	elseif (maplevel == 2)
		 	PrintTextAbove(who, "*Gritos nos arredores* Atacar rapazes! A pior parte já fizeram!");
			sleep(2);
            createnpcfromtemplate(":brainai:mercenario", who.x, who.y, who.z +5, 0 , who.realm);
            createnpcfromtemplate(":brainai:roguearcher", who.x, who.y, who.z +5, 0 , who.realm);
			createnpcfromtemplate(":brainai:trapaceiro", who.x, who.y, who.z +5, 0 , who.realm);
            createnpcfromtemplate(":brainai:roguearcher", who.x, who.y, who.z +5, 0 , who.realm);
            createnpcfromtemplate(":brainai:mercenario", who.x, who.y, who.z +5, 0 , who.realm);
			createnpcfromtemplate(":brainai:mercenario", who.x, who.y, who.z +5, 0 , who.realm);
        elseif (maplevel == 3)
			PrintTextAbove(who, "*Gritos e lamurias de almas atormendas* Juntem-se a nos...");
			sleep(5);
			CreateNpcFromTemplate(":brainai:Ghost", who.x, who.y, who.z, 0, who.realm);
			CreateNpcFromTemplate(":brainai:Ghost", who.x, who.y, who.z, 0, who.realm);
			CreateNpcFromTemplate(":brainai:Ghost", who.x, who.y, who.z, 0, who.realm);
			CreateNpcFromTemplate(":brainai:Spectre", who.x, who.y, who.z, 0, who.realm);
			CreateNpcFromTemplate(":brainai:Spectre", who.x, who.y, who.z, 0, who.realm);
			CreateNpcFromTemplate(":brainai:Spectre", who.x, who.y, who.z, 0, who.realm);
			sleep(15);
			PrintTextAbove(who, "*Vozes Fantasmagoricas* Deliciosa ganancia...");
			CreateNpcFromTemplate(":brainai:espectroanciao", who.x, who.y, who.z, 0, who.realm);
			CreateNpcFromTemplate(":brainai:espectroanciao", who.x, who.y, who.z, 0, who.realm);
			CreateNpcFromTemplate(":brainai:espectroanciao", who.x, who.y, who.z, 0, who.realm);
		elseif (maplevel == 4)
			PrintTextAbove(who, "*Criaturas gigantescas guardam o local*");
			sleep(5);
			CreateNpcFromTemplate(":brainai:Wyvern", who.x, who.y, who.z, 0, who.realm);
			CreateNpcFromTemplate(":brainai:Wyvern", who.x, who.y, who.z, 0, who.realm);
			CreateNpcFromTemplate(":brainai:Wyvern", who.x, who.y, who.z, 0, who.realm);
			sleep(15);
			CreateNpcFromTemplate(":brainai:RedDragon", who.x, who.y, who.z, 0, who.realm);
		elseif (maplevel == 5)
			CreateNpcFromTemplate(":brainai:Dracolich", who.x, who.y, who.z, 0, who.realm);
        endif
		chest.movable := 0;
			//SetObjProperty(item, "#DestroyAt", ReadGameClock() + 60); 
      		EraseObjProperty(map, "level");
      		EraseObjProperty(map, "chestx");
      		EraseObjProperty(map, "chesty");
      		SetObjProperty(map, "Solved", 1);
      		map.name := "Mapa de tesouro [Completado por " + who.name + "]";
			//TS_StartTimer(chest, "treasure_chest_destroy", 600);
			detach();
			ChestDestroy (chest);
    endif

endfunction

function ChestDestroy( chest )
sleep(600);
DestroyItem(chest);
print("o chest foi destruido");
endfunction

function GetRange( who );

  var skill := Cint((AP_GetSkill( who, GATHERING )));

  if( skill >= 100 )
    return 5;
  elseif( skill >= 80 )
    return 4;
  elseif( skill >= 60 )
    return 3;
  elseif( skill >= 40 )
    return 2;;
  elseif( skill >= 20 )
    return 1;
  else
    return 0;
  endif

endfunction

function GetDataFile()
	var data_file := DFOpenDataFile(":gathering:OreSettings", DF_CREATE);
	return data_file;
endfunction


function GetDataElem(elem_name)
	var data_file := GetDataFile();
	var data_elem := DFFindElement(data_file, elem_name, DF_CREATE);

	return data_elem;
endfunction

function SetOreAmount(realm, x_pos, y_pos, amount)
	var pos_string := CStr(realm+" "+x_pos+"x"+y_pos);

	var data_elem := GetDataElem(pos_string);

	data_elem.SetProp("Amount", CInt(amount));
endfunction

function GetOreAmount(realm, x_pos, y_pos)

	var pos_string := CStr(realm+" "+x_pos+"x"+y_pos);

	var data_elem := GetDataElem(pos_string);

	var amount := data_elem.GetProp("Amount");
	if ( amount == error )
		SetOreAmount(realm, x_pos, y_pos, MAX_RESOURCE_AMOUNT);
		amount := MAX_RESOURCE_AMOUNT;
	endif

	return CInt(amount);
endfunction

function GetResourceId(oreToSearch)

	case (oreToSearch)

		"areia": return 0x6075;

		"cobre": return 0x6450;

		"bronze": return 0x699c;

		"argila": return 0x6074;

		"ferro": return 0x6300; //TN USED

		"litan": return 0x6301;

		"Fulmetium ": return 0x6302;

		"Venetium": return 0x6303;

		"Ignium": return 0x6304;

		"ouro": return 0x6305;

		//"durang": return 0x6306;

		"galvorn": return 0x6307; //TN USED

		"mithril": return 0x6308; //TN USED

		"frigutium": return 0x6451; //TN USED

        "illyra": return 0x6314;

		//"gromril": return 0x6452; //TN USED

		"prata": return 0x6992; //TN USED

		"estanho": return 0x6990; //TN USED

		//"ario": return 0x6312;

		"pedras": return 0x6666;

		"marmore": return 0x8850;

		"arenito": return 0x8852;

	endcase

endfunction

function DetermineOreType(who, where)

	var cfg := ReadConfigFile(":gathering:config/miningspots");
	var ores := cfg[where].Ores;
	var percent := cfg[where].Rarity;
  if (ores == error)
    var df := opendatafile(":gathering:miningspots");
    var elem := df.FindElement(where);
    ores := elem.GetProp("Ores");
    percent := elem.GetProp("Rarity");
  endif
	var i := 1;
	ores := SplitWords(ores);
	percent := SplitWords(percent);
	var chance := RandomInt(1000);
	var oreToSearch;
	var skill := AP_GetSkill(who, GATHERING);

	if (GetObjProperty(who, "searchore") == error)
		oreToSearch := "pedras";
	else
		oreToSearch := GetObjProperty(who, "searchore");
	endif

	var achou := 0;
	foreach ore in ores
		if (ore == oreToSearch)
			achou := 1;
			break;
		else
			i := i + 1;
		endif
	endforeach

	if (!achou)
		SendSysMessage(who, "Nao tem o minerio que voce procura aqui.");
		return 0;
	endif

	if ( chance <= Cint(percent[i]))
		var oreid := GetResourceId(oreToSearch);
		//var oreid := GetResourceId("ario");
		if (oreid == 0x6666)
			return 0x5001;
		endif
		return oreid;
	else
		SendSysMessage(who, "Voce nao conseguiu achar o minerio que procura.");
		return 999;
	endif


endfunction

//dificuldade de PEGAR o minerio
function DetermineMiningDifficulty(oretype) 

	case (oretype)
		0x1bdd:	0xee36:	0xee37:	0xee38:	0xee39:	0xee3a:	0xee3b:	0xee3c:	0xee3d:	0xee3e:	0xee3f: return 5; // Carvão
		0xBD11: return 0;  //pedra pequena 
		//0xBD12: return 0;  //pedra media
		//0xBD13: return 0;  //pedra grande
		0xee99: return 0;  //calcario

		0x6300: return 0;  //iron ore
		0x62ff: return 40;  //ferro impuro
		0x6309: return 50; //iron temperado
		0x6301: return 70; //litan
        0x6314: return 82; //illyra
		0x6302: return 72; //fulmetita
		0x6303: return 72; //venetita
		0x6304: return 72; //ignitita
		0x6451: return 72; //frigutita
		0x6307: return 90; //galvorn
		0x6308: return 90; //mithril
		0x6450: return 20; //cobre
		0x6465: return 20; //cobre -> bronze
		
		0x699c: return 10; //bronze
		0x699d: return 20; // Bronze temperado
		0x6994: return 20; // bronze
		0x6995: return 60; //aco

		0x6992: return 50; //prata
 		0x6990: return 30; //estanho

		0x6075: return 5; //areia

		0x6666: return 0; //id falso de pedra
		0x8850: return 40; //marmore
		0x8852: return 30; //arenito
		0x5001: return 5; //id de stone

		0x6305: return 40; //golden <- nao faz nada

		default: return 100;
	endcase

endfunction

function GetOreRarityDesc(rarity)

	if (rarity <= 100 )
		return "muito raro";
	elseif (rarity <= 200 )
		return "raro";
	elseif (rarity <= 500 )
		return "incomum";
	elseif (rarity <= 700 )
		return "comum";
	else
		return "muito comum";
	endif

endfunction

function InRect(x_pos, y_pos, rect)
	var x1 := CInt(rect[1]);
	var y1 := CInt(rect[2]);
	var x2 := CInt(rect[3]);
	var y2 := CInt(rect[4]);

	return ( (x_pos >= x1 && x_pos <= x2) && (y_pos >= y1 && y_pos <= y2) );
endfunction

function GetMiningRegion(x, y)

  var df := opendatafile(":gathering:miningspots");
  foreach elem in (df.Keys())

    var region := df.FindElement(elem);
    var rect := region.getProp("Rect");
    if (inrect(x, y, SplitWords(rect)))
      return elem;
    endif

  endforeach

	return "Whole_World";

endfunction

function CanMine(targ, type)
	var config := ReadConfigFile(":gathering:config/miningtiles");
	config := config[type];

	var land_scape := CStr( GetMapInfo(targ.x, targ.y, targ.realm).landtile );
	if ( targ.objtype )
		if ( CStr(targ.objtype) in GetConfigStringArray(config, "static") )
			return 1;
		endif
	elseif ( land_scape in GetConfigStringArray(config, "landscape") )
		return 1;
	endif

	return 0;
endfunction

function ResourceAmount(resource_type, byref elem)
	var amount := DFGetProp(elem, resource_type, DF_CREATE, CStr(MAX_RESOURCE_AMOUNT));

	return CInt(amount);
endfunction

function SetResource(resource_type, byref elem, amount)
	elem.setprop( resource_type, CStr(amount) );
endfunction

function CheckLoop(who, tool, my_x, my_y, my_facing)
	if ( who.hidden )
		SendSysMessage(who, "Voce parou de minerar para ficar escondido.");
		return 0;
	elseif ( (who.x != my_x) || (who.y != My_y) )
		SendSysMessage(who, "Voce saiu de onde estava.");
		return 0;
	elseif ( who.facing != my_facing )
		SendSysMessage(who, "Voce virou sua atencao para outra coisa.");
		return 0;
	elseif ( GetEquipmentByLayer(who, LAYER_MOUNT) )
		SendSysMessage(who, "Voce nao pode minerar montado.");
		return 0;
	elseif (!tool)
		return 0;
	else
		return 1;
	endif
endfunction

function ConvertOreToIngot(ore)
	case (ore) //de acordo com ore no pot indica o ingot certo
		0x1bdd:	0xee36:	0xee37:	0xee38:	0xee39:	0xee3a:	0xee3b:	0xee3c:	0xee3d:	0xee3e:	0xee3f: return 0x1bde; //carvao

		0x6075: return 0x1be3; //vidro
		0x6450: return 0x6465; //copper
		0x6990: return 0x6991; //estanho
		0x6300: return 0x62ff; //ferro -> ferro impuro
		0x6465: return 0x699d; //copper -> bronze impuro
		0x699c: return 0x699d; //bronze impuro
		0x699d: return 0x6994; //bronze temperado
		0x62ff: return 0x6309; //ferro impuro -> ferro temperado
		0x6992: return 0x6993; //prata
		0x6305: return 0x630e; //ouro
		//0x6309: return 0x6309; //aco -> acho q esse aqui chorou
		0x6309: return 0x6995; // aco a partir de ferro temperado
		0x6995: return 0x6995; //ligas elementais
		0x6301: return 0x630a; //litan
		0x6314: return 0x6315; //illyra
		0x6307: return 0x6310; //galvorn
		0x6308: return 0x6311; //mithril
		0x6303: return 0x630c; //venetium
		0x6451: return 0x6460; //frigutium
		0x6304: return 0x630d; //ignium
		0x6302: return 0x630b; //fulmetium

		//esses ultimos nao estao sendo usados.
		0x6312: return 0x6313; //ario
		0x6452: return 0x6460; //gomril
		0x6306: return 0x630f; //durang

		0x1bde: return 0x1bde;

		default: return 0;
	endcase
endfunction

function getOreByOreName(param)
	var ores := getAllOres();

	foreach ore in ores
		if (ore.oreName == param)
			return ore;
		endif
	endforeach
endfunction
function getOreByOreObjType(param)
	var ores := getAllOres();

	foreach ore in ores
		if (ore.oreObjType == param)
			return ore;
		endif
	endforeach
endfunction
function getOreByIngotName(param)
	var ores := getAllOres();

	foreach ore in ores
		if (ore.ingotName == param)
			return ore;
		endif
	endforeach
endfunction
function getOreByIngotObjType(param)
	var ores := getAllOres();

	foreach ore in ores
		if (ore.ingotObjType == param)
			return ore;
		endif
	endforeach
endfunction

/*
 * getAllOres()
 *
 * Purpose
 * Cataloga todos as matérias primas e suas composições do shard.
 * Deve estar na ordem em que aparecerá nos gumps.
 * 
 * Parameters
 * Não possui parâmetros.
 * Verificar a construção do toStruct(params) para saber a ordem correta de criação.
 * Em caso de precisar passar os valores opcionais da função, lembrar de passar um array vazio({}) na posição othersOresNeeded. 
 *
 * Retorna value
 * Um array de struct com o valor de todos os minérios do shard
 */
function getAllOres()
	return array{
		/* Carvão */
		toStruct(ORE_CHARCOAL, "Madeira", "Carvão", 0x1bdd, {}, 2323),
		toStruct(ORE_CHARCOAL, "Madeira", "Carvão", 0xee36, {}, 2323),
		toStruct(ORE_CHARCOAL, "Madeira", "Carvão", 0xee37, {}, 2323),
		toStruct(ORE_CHARCOAL, "Madeira", "Carvão", 0xee38, {}, 2323),
		toStruct(ORE_CHARCOAL, "Madeira", "Carvão", 0xee39, {}, 2323),
		toStruct(ORE_CHARCOAL, "Madeira", "Carvão", 0xee3a, {}, 2323),
		toStruct(ORE_CHARCOAL, "Madeira", "Carvão", 0xee3b, {}, 2323),
		toStruct(ORE_CHARCOAL, "Madeira", "Carvão", 0xee3c, {}, 2323),
		toStruct(ORE_CHARCOAL, "Madeira", "Carvão", 0xee3d, {}, 2323),
		toStruct(ORE_CHARCOAL, "Madeira", "Carvão", 0xee3e, {}, 2323),
		toStruct(ORE_CHARCOAL, "Madeira", "Carvão", 0xee3f, {}, 2323),

		toStruct(ORE_SAND, "Areia", "Vidro", 0x6075),

		/* Metais primários */
		toStruct(ORE_COPPER, "Minério de Cobre", "Barra de Cobre", 0x6450),
		toStruct(ORE_IRON, "Minério de Ferro", "Barra de Ferro", 0x6300),
		toStruct(ORE_TIN, "Minério de Estanho", "Barra de Estanho", 0x6990),
		// toStruct(ORE_BRONZE, "Minério de Bronze", "Barra de Bronze", 0x699c),

		/* Ligas Metálicas */
		toStruct(ORE_BRONZE, "Barra de Cobre", "Barra de Bronze", 0x6465, {0x6991}),
		toStruct(ORE_IRON_TEMP, "Barra de Ferro", "Barra de Ferro Temperado", 0x62ff, {0x1BDE}),
		toStruct(ORE_BRONZE_TEMP, "Barra de Bronze Impuro", "Barra de Bronze Temperado", 0x699d, {0x1BDE}),
		toStruct(ORE_STEEL, "Barra de Ferro Temperado", "Barra de Aço", 0x6309, {0x1BDE, 0x6990}, 2599),

		/* Ligas Elementais */
		toStruct(ORE_IGNITITA, "Minério de Ignitita", "Barra de Ignium", 0x6304, {/* Colocar LIGA aqui */}, 2501),
		toStruct(ORE_VENETITA, "Minério de Venetita", "Barra de Venetium", 0x6303, {/* Colocar LIGA aqui */}, 1428),
		toStruct(ORE_FRIGUTITA, "Minério de Frigutita", "Barra de Frigutium", 0x6451, {/* Colocar LIGA aqui */}, 2360),
		toStruct(ORE_FULMETITA, "Minério de Fulmetita", "Barra de Fulmetium", 0x6302, {/* Colocar LIGA aqui */}, 2726),

		// toStruct(ORE_LITAN, "Minério de Litan", "Barra de Litan", 0x6301, {}, 1801),
		toStruct(ORE_ILLYRA, "Minério de Illyra", "Barra de Illyra", 0x6314, {}, 2278),
		toStruct(ORE_MITHRIL, "Minério de Mithril", "Barra de Mithril", 0x6308, {}, 2448),

		/* Metais preciosos */
		toStruct(ORE_SILVER, "Minério de Prata", "Barra de Prata", 0x6992, {}, 2326),
		toStruct(ORE_GOLD, "Minério de Ouro", "Barra de Ouro", 0x6305, {}, 2598)
	};
endfunction

/* 
 * Parameters
 *
 * oreType: Enum (fim deste documento) com o tipo do ore
 * oreName: Nome do minério bruto
 * ingotName: Nome da barra do minério
 * objType: ObjType do minério
 * othersOresNeeded: Array com os objType dos demais minérios necessários pra fazer o ingot (em caso de liga)
 * color: Cor do metal
 *
 * Return value
 * Um struct contendo os dados do material:
 * oreType, oreName, ingotName, oreObjType, ingotObjType, neededToCraft, color.
 */
function toStruct(oreType, oreName, ingotName, objType, othersOresNeeded := array{}, color := 0)
	return struct{
		"oreType" := oreType,
		"oreName" := oreName, 
		"ingotName" := ingotName,
		"oreObjType" := objType,
		"ingotObjType" := ConvertOreToIngot(objType),
		"neededToCraft" := othersOresNeeded,
		"difficulty" := DetermineMiningDifficulty(objType),
		"color" := color
	};
endfunction

enum ORE
	ORE_CHARCOAL := "ORE_CHARCOAL",
	ORE_SAND := "ORE_SAND",

	ORE_TIN := "ORE_TIN",
	ORE_COPPER := "ORE_COPPER",
	ORE_BRONZE := "ORE_BRONZE",
	ORE_IRON := "ORE_IRON",

	ORE_IRON_TEMP := "ORE_IRON_TEMP",
	ORE_BRONZE_TEMP := "ORE_BRONZE_TEMP",
	ORE_STEEL := "ORE_STEEL",
	
	ORE_IGNIUM := "ORE_IGNIUM",
	ORE_IGNITITA := "ORE_IGNITITA",
	ORE_VENETIUM := "ORE_VENETIUM",
	ORE_VENETITA := "ORE_VENETITA",
	ORE_FRIGUTIUM := "ORE_FRIGUTIUM",
	ORE_FRIGUTITA := "ORE_FRIGUTITA",
	ORE_FULMETIUM := "ORE_FULMETIUM",
	ORE_FULMETITA := "ORE_FULMETITA",
	
	ORE_LITAN := "ORE_LITAN",
	
	ORE_ILLYRA := "ORE_ILLYRA",
	ORE_MITHRIL := "ORE_MITHRIL",

	ORE_SILVER := "ORE_SILVER",
	ORE_GOLD := "ORE_GOLD"
endenum

enum INGOT
	INGOT_CHARCOAL := "INGOT_CHARCOAL"
endenum
