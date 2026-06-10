/*
 * mochilaPackhorse.inc — helpers da Mochila de Packhorse (0x71F0 / 0x71F1).
 *
 * A mochila craftada substitui o backpack default do packhorse. O maxweight
 * escala pelo tier do couro principal usado na fabricação (resourceList.cfg).
 *
 * Tabela de bonus aditivo a `max_weight_mod` (base do cfg = 400):
 *   Comum     (0x71F0): t1=+25  t2=+50   t3=+75   t4=+100  t5=+150
 *   Reforcada (0x71F1): t1=+50  t2=+100  t3=+150  t4=+200  t5=+300
 *
 * Tier 4 não tem couro disponível hoje (gap canônico do resourceList).
 * A entrada existe para que receitas futuras encaixem sem mudar o código.
 *
 * CProps na mochila:
 *   #mochila_tier            (1..5)  tier do couro principal
 *   #mochila_couro_objtype   (int)   objtype do couro usado (informativo)
 */

use uo;
use os;

include ":wrappers:Objects/Model/ContainerWrapper";

const MOCHILA_COMUM_OBJTYPE     := 0x71F0;
const MOCHILA_REFORCADA_OBJTYPE := 0x71F1;

function IsMochilaPackhorse(item)
	if (!item)
		return 0;
	endif
	return item.objtype == MOCHILA_COMUM_OBJTYPE
	    || item.objtype == MOCHILA_REFORCADA_OBJTYPE;
endfunction

function GetMochilaMaxWeightMod(objtype, tier)
	tier := CInt(tier);
	if (tier < 1)
		tier := 1;
	elseif (tier > 5)
		tier := 5;
	endif

	if (objtype == MOCHILA_REFORCADA_OBJTYPE)
		case (tier)
			1: return 50;
			2: return 100;
			3: return 150;
			4: return 200;
			5: return 300;
		endcase
	endif

	// Comum (default)
	case (tier)
		1: return 25;
		2: return 50;
		3: return 75;
		4: return 100;
		5: return 150;
	endcase

	return 0;
endfunction

// Le o tier do principal Encaixe da fabricacao. Retorna 0 se nao for um
// item fabricado com componentes (ex.: spawn manual de mochila default).
function ReadFabricadoTier(item)
	var fabricado := GetObjProperty(item, "fabricado");
	if (!fabricado || fabricado == error)
		return 0;
	endif
	if (!fabricado.esquema || !fabricado.esquema.componentesEscolhidos)
		return 0;
	endif
	foreach c in (fabricado.esquema.componentesEscolhidos)
		Sleepms(2);
		if (c.componenteNecessario && c.componenteNecessario.principal)
			if (c.tier)
				var tierId := c.tier.id;
				if (!tierId)
					tierId := CInt(c.tier);
				endif
				return CInt(tierId);
			endif
		endif
	endforeach
	return 0;
endfunction

function GetMochilaTier(mochila)
	var tier := CInt(GetObjProperty(mochila, "#mochila_tier"));
	if (tier >= 1)
		return tier;
	endif
	tier := ReadFabricadoTier(mochila);
	if (tier >= 1)
		SetObjProperty(mochila, "#mochila_tier", tier);
		return tier;
	endif
	return 1;
endfunction

// Nome canonico do couro pelo objtype (ver resourceList.cfg).
function GetCouroNomeByObjtype(objtype)
	case (CInt(objtype))
		0xee45: return "Couro Curtido";
		0xef23: return "Couro Negro";
		0xef24: return "Couro Peludo";
		0xef25: return "Couro Efervecido";
		0xef26: return "Couro Polar";
		0xef20: return "Couro Escamoso";
		0xed29: return "Couro Draconiano";
		0xef27: return "Couro de Gigante";
		0xc605: return "Couro das Profundezas";
	endcase
	return "";
endfunction

// Tenta extrair o objtype do couro principal a partir do fabricado da mochila.
// Usado em mochilas craftadas (sem #mochila_couro_objtype explicito).
function ExtractCouroObjtypeFromFabricado(mochila)
	var fabricado := GetObjProperty(mochila, "fabricado");
	if (!fabricado || fabricado == error)
		return 0;
	endif
	if (!fabricado.esquema || !fabricado.esquema.componentesEscolhidos)
		return 0;
	endif
	foreach c in (fabricado.esquema.componentesEscolhidos)
		Sleepms(2);
		if (c.componenteNecessario && c.componenteNecessario.principal)
			if (c.material)
				if (c.material.objtype)
					return CInt(c.material.objtype);
				endif
				if (c.material.chave)
					return CInt(c.material.chave);
				endif
			endif
		endif
	endforeach
	return 0;
endfunction

function GetMochilaCouroNome(mochila)
	var ot := CInt(GetObjProperty(mochila, "#mochila_couro_objtype"));
	if (ot <= 0)
		ot := ExtractCouroObjtypeFromFabricado(mochila);
		if (ot > 0)
			SetObjProperty(mochila, "#mochila_couro_objtype", ot);
		endif
	endif
	return GetCouroNomeByObjtype(ot);
endfunction

// Atualiza o tooltip da mochila com nome do couro + capacidade efetiva.
// Stones efetivos = base do cfg (400) + max_weight_mod aplicado.
function UpdateMochilaDescmod(mochila)
	if (!IsMochilaPackhorse(mochila))
		return;
	endif
	var tier := GetMochilaTier(mochila);
	var mod := GetMochilaMaxWeightMod(mochila.objtype, tier);
	var efetivo := 400 + mod;
	var couroNome := GetMochilaCouroNome(mochila);
	var prefixo := "Mochila";
	if (mochila.objtype == MOCHILA_REFORCADA_OBJTYPE)
		prefixo := "Mochila Reforcada";
	endif
	var nome := prefixo;
	if (couroNome)
		nome := prefixo + " de " + couroNome;
	endif
	SetObjProperty(mochila, "descmod", nome + "<br>Capacidade: " + efetivo + " stones<br>");
endfunction

function ApplyMochilaTier(mochila)
	if (!IsMochilaPackhorse(mochila))
		return 0;
	endif
	var tier := GetMochilaTier(mochila);
	var mod := GetMochilaMaxWeightMod(mochila.objtype, tier);
	ContainerWrapper(mochila).SetMaxWeight(mod);
	UpdateMochilaDescmod(mochila);
	return mod;
endfunction

function IsPackAnimalGraphic(graphic)
	return graphic == 291 || graphic == 292;
endfunction

// Mapeia o npctemplate de um cavalo/llama de carga pra graphic packhorse
// correspondente. Retorna 0 se o template nao se aplica.
function GetPackhorseGraphicForTemplate(npctemplate)
	var tmpl := Lower(CStr(npctemplate));
	if (tmpl["packhorse2"] || tmpl["llama"])
		return 292;
	elseif (tmpl["packhorse"] || tmpl["horse3"])
		return 291;
	endif
	return 0;
endfunction

// Garante que o NPC tem backpack equipada. Packhorses ganham uma Mochila
// Comum tier 1 default; outros NPCs ganham o backpack generico.
// Reaplica o tier da mochila atual se houver. Chame no Setup() do brain.
function EnsureNpcBackpack(npc)
	if (!npc.backpack)
		if (IsPackAnimalGraphic(npc.graphic))
			var new_pack := CreateItemAtLocation(npc.x, npc.y, npc.z, MOCHILA_COMUM_OBJTYPE, 1);
			SetObjProperty(new_pack, "#mochila_tier", 1);
			EquipItem(npc, new_pack);
		else
			var new_pack := CreateItemAtLocation(npc.x, npc.y, npc.z, "backpack", 1);
			EquipItem(npc, new_pack);
		endif
	endif
	if (IsPackAnimalGraphic(npc.graphic) && IsMochilaPackhorse(npc.backpack))
		ApplyMochilaTier(npc.backpack);
	endif
endfunction

// Instala uma mochila craftada no packhorse. Converte cavalo/llama normal pra
// packhorse se necessario, redistribui conteudo da bag antiga (player → nova
// mochila → chao), destroi a antiga, equipa a nova, aplica tier.
// Retorna 1 em sucesso, 0 em rejeicao (envia SysMessage explicativa).
function InstallMochilaOnPackhorse(npc, mochila, source)
	if (!IsPackAnimalGraphic(npc.graphic))
		var convertedGraphic := GetPackhorseGraphicForTemplate(npc.npctemplate);
		if (convertedGraphic == 0)
			SendSysMessage(source, "Esta mochila so pode ser usada em packhorses.");
			return 0;
		endif
		SetObjProperty(npc, "beforepack", npc.graphic);
		npc.graphic := convertedGraphic;
	endif
	if (npc.backpack)
		foreach old_item in EnumerateItemsInContainer(npc.backpack)
			Sleepms(2);
			if (!MoveItemToContainer(old_item, source.backpack))
				if (!MoveItemToContainer(old_item, mochila))
					MoveObjectToLocation(old_item, source.x, source.y, source.z, source.realm);
				endif
			endif
		endforeach
		DestroyItem(npc.backpack);
	endif
	if (!EquipItem(npc, mochila))
		SendSysMessage(source, "Falha ao instalar a mochila.");
		MoveItemToContainer(mochila, source.backpack);
		return 0;
	endif
	ApplyMochilaTier(npc.backpack);
	PrintTextAbove(source, "*equipa a mochila no packhorse*");
	return 1;
endfunction

// Faz o "Retira": reverte graphic do packhorse pra cavalo/llama normal e
// devolve a mochila atual ao source (mantendo tier/props) ou cria bag default
// no inv do source se nao for mochila craftada.
function HandlePackhorseRetira(npc, source)
	npc.graphic := GetObjProperty(npc, "beforepack");
	if (!GetObjProperty(npc, "beforepack"))
		if (npc.graphic == 292)
			npc.graphic := 220;
		elseif (npc.graphic == 291)
			npc.graphic := 200;
		endif
	endif
	if (IsMochilaPackhorse(npc.backpack))
		ContainerWrapper(npc.backpack).SetMaxWeight(0);
		MoveItemToContainer(npc.backpack, source.backpack);
	else
		CreateItemInBackpack(source, "backpack");
	endif
	EraseObjProperty(npc, "beforepack");
endfunction
