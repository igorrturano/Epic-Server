use uo;
use os;
use basic;

include ":attributes:attributes";
include ":destroyable:include/destroyItemPersist";
include ":itemutils:itemInfo";
include "/include/utils";
include "/include/epicPropNames";
include ":traps:traps";
include ":architect:architect";
include ":crafting:/include/fabricacaoUtil";
include "/include/packets";
include ":combat:Model/AttackInfo";
include ":combat:Controller/DamageController";
include ":crafting:include/util/fabricacaoConstants";

const ELEM_CFG_DEST := "Destruiveis";
const TIPO_DISCORD_DESTRUIDO := "itemDestruido";

enum TIPO_MATERIAIS
	PEDRA := "Stone",
	TIJOLO := "Brick",
	MARMORE := "marmore",
	ARENITO := "arenito",
	OSSOS := "ossos",
	VIDRO := "Glass",
	MADEIRA := "Wood",
	PANO := "Cloth",
	METAL := "Metal",
	COURO := "Leather",
	CORPO := "Corpo",
	CORPO_HUMANO := "CorpoHumano",
	INDEFINIDO := "indefinido"
endenum

var catalogoDestrocos := MontaCatalogoDestrocos();
var configDestruiveis := ReadConfigFile(":destroyable:config/settings");

/*
 * ComportamentoWarMode(quem, itemFonte)
 *
 * Purpose
 * Criado para ser um gancho nos usescripts de armas e ferramentas
 *
 * Parameters
 * quem:	   Personagem que ativou o script
 * itemFonte:  o item que rodou o script
 *
 * Return value
 *  Sem retorno
 *
*/
function ComportamentoWarMode(quem, itemFonte)

	EraseObjProperty(quem, "IsMeditating");
	EraseObjProperty(quem, "HealTimer");
	SendSysMessage(quem, "[Modo Guerra] Escolha o item que deseja destruir.");
	var targ := TargetCoordinates(quem);

	if (!targ)
		SendSysMessage(quem, "Cancelado.");
		return 0;
	endif

	if (targ == quem.weapon)
		MoveItemToContainer(targ, quem.backpack);
		Message("Você desequipa sua arma.").SetColor(SSM_INFO).SendSys(quem);
		return 0;
	endif

	if ( !targ.item )
		SendSysMessage(quem, "Você não pode destruir isso.");
		return 0;
	endif

	start_script(":tn:destroyitems", { quem, targ.item, itemFonte });
endfunction

/*
 * EnviaEventoDanoItem(quem, item, atributosEvento)
 *
 * Purpose
 * Envia evento de item danificado a todos os NPCs na região.
 *
 * Parameters
 * quem:	   		Personagem que danificou
 * tiem:  			item danificado.
 * atributosEvento: atributos do ato de danificar.
 * Return value
 *  Sem retorno
 *
*/
function EnviaEventoDanoItem(quem, item, atributosEvento)
	// var mobiles:=ListMobilesNearLocation( item.x, item.y, item.z, 20, item.realm );

	var evParams := array;
	evParams.append(quem);
	evParams.append(item);
	evParams.append(atributosEvento);

// TODO implementar envio de evento uma vez que a nova IA estiver mais madura.
/*
	var ev := struct;
		ev.+type := 0x78454;
		ev.+data := evParams;

	foreach mob in mobiles
		SendEvent(mob, ev);
		Sleepms(2);
	endforeach
*/
endfunction

/*
 * Destruir(quem, item, fonteDano)
 *
 * Purpose
 * Roda a lógica de destruição de um item que participe do escopo do sistema de destruição.
 *
 * Parameters
 * quem:	   		Personagem que danificou
 * itemAlvo:  		item danificado.
 * fonteDano: 		fonte de dano. Pode <itemRef>, struct com membro <dano> e <desc>, ou apenas um valor real.
 *
 * Return value
 *  Sem retorno
 *
*/
function Destruir(quem, itemAlvo, fonteDano)
	LimpaObjPropsDestruiveis(itemAlvo);
	var envelopeDeed := struct;
	envelopeDeed.+isDeed := GetObjProperty(itemAlvo, PROPNAME_DEED_SERIALS);

	if (envelopeDeed.isDeed)
		envelopeDeed.+itensAssociados := array;
		foreach serial in (envelopeDeed.isDeed)
			var item := SystemFindObjectBySerial(serial);
			envelopeDeed.itensAssociados.append(item);
			Sleepms(2);
		endforeach
	endif

	var deveMarcar := DeveMarcarRecriarItem(itemAlvo);
	if (deveMarcar) // se não foi criado por player, não é um corpo e não foi marcado como dispensável..
		var serialOriginal := GetObjProperty(itemAlvo, PROPNAME_CHAVE_ORIGINAL);
		if (!serialOriginal)
			SetObjProperty(itemAlvo, PROPNAME_CHAVE_ORIGINAL, itemAlvo.serial); // Grava serial do item original.
		endif
		if (envelopeDeed.isDeed)
			foreach item in (envelopeDeed.itensAssociados)
				MarcarRecriarItem(item);
				Sleepms(2);
			endforeach
		else
			MarcarRecriarItem(itemAlvo);
		endif
	endif

	var isObjetoMovido := false;

	// TODO talvez criar um container "morto" e colocar todos os espolios dentro?.
	if (itemAlvo.ISA(POLCLASS_CONTAINER))
		var listaItens := EnumerateItemsInContainer(itemAlvo);
		foreach espolio in listaItens
			if (RandomInt(100) <= configDestruiveis[ELEM_CFG_DEST].espolioDestChance)
				continue; // deixa o equipamento para ser destruido junto com o container.
			else
				if (espolio.ISA(POLCLASS_EQUIPMENT))
					espolio.hp := espolio.hp - RandomIntMinMax(CInt(espolio.maxhp / 5), espolio.maxhp); // Danifica o equipamento caso não seja destruído.
				endif
				MoveObjectToLocation(espolio, itemAlvo.x, itemAlvo.y, itemAlvo.z, itemAlvo.realm,
									  MOVEOBJECT_FORCELOCATION);
				isObjetoMovido := true;
			endif
			Sleepms(2);
		endforeach
	endif

	RunTraps(itemAlvo, quem);
	TocaSFXDestruicao(itemAlvo, isObjetoMovido);
	if (!IsDestroco(itemAlvo))
		var itemAlvoDesc := itemAlvo.desc;
		if (DeveCriarDestroco(itemAlvo))
			if (envelopeDeed.isDeed)
				CriaDestrocos(itemAlvo, envelopeDeed.isDeed.size(), 4);
				itemAlvoDesc += "[Deed]";
			else
				CriaDestrocos(itemAlvo, CalculaMaxDestrocos(itemAlvo), 4);
			endif
		endif
		var descFonteDano := fonteDano.desc;
		if (!descFonteDano)
			descFonteDano := "Desconhecido";
		endif
		if(TypeOfInt(itemAlvo.x) != OT_UNINIT || TypeOfInt(itemAlvo.x) != OT_ERROR)//TODO tem um unintialized vindo de algum lugar, essa é uma solução provisória.
			InsereLogItemDestruido(quem,POLCore().systime,itemAlvo.x,itemAlvo.y,itemAlvo.z,itemAlvo.realm,itemAlvoDesc,descFonteDano,deveMarcar);
			DisparaAvisoDiscord(quem,itemAlvo.x,itemAlvo.y,itemAlvo.z,itemAlvo.realm,itemAlvoDesc,descFonteDano);
		endif
	endif

	if (!envelopeDeed.isDeed)
		DestroyItem(itemAlvo);
	else
		foreach item in (envelopeDeed.itensAssociados)
			DestroyItem(item);
			Sleepms(2);
		endforeach
	endif
endfunction

/*
 * DisparaAvisoDiscord(quem, x,y,z,reino, itemAlvoDesc, descFonteDano)
 *
 * Purpose
 * Dispara um aviso em um canal do discord, informando que um item foi destruído.
 *
 * Parameters
 * quem:	   		Personagem que danificou
 * x:	   			coordenada x  do acontecido
 * y:	   			coordenada y  do acontecido
 * z:	   			coordenada z  do acontecido
 * reino:	   		Preino
 * itemAlvoDesc:  		nome do item destruído
 * descFonteDano: 		descrição da fonte de dano.
 *
 * Return value
 *  Sem retorno
 *
*/
function DisparaAvisoDiscord(quem, x, y, z, reino, itemAlvoDesc, descFonteDano)
	if (quem.acctname && !Find(UPPER(descFonteDano), "ARMADILHA", 1))
		var nome := GetObjProperty(quem, "nome_inicial");
		var tipo := TIPO_DISCORD_DESTRUIDO;
		Start_script(":DiscordWebhook:discord", { tipo,
										  		nome,
										  		quem.acctname,
										  		descFonteDano,
										  		reino,
										  		{ x, y, z }, itemAlvoDesc });
	endif
endfunction

/*
 * DeveMarcarRecriarItem(itemAlvo)
 *
 * Purpose
 * Avalia se o item deve ser marcado para recriação pelo vigilante de itens destruídos.
 * Parameters
 * itemAlvo:	   		item a ser avaliado
 *
 * Return value
 *  true se o item se qualifica para ressurgimento.
 *
*/
function DeveMarcarRecriarItem(itemAlvo)
	return !GetObjProperty(itemAlvo,PROPNAME_ARTIFICE_SERIAL) 
	&& !GetObjProperty(itemAlvo,PROPNAME_DISPENSAVEL_FLAG) //Coloco todo objeto como indispensavel para que itens destruidos antes do sistema entrar nao sejam descartados.
	&& !itemAlvo.ISA(POLCLASS_CORPSE)
	&& itemAlvo.ISA(POLCLASS_ITEM)
	&& !IsDestroco(itemAlvo)
	&& TypeOfInt(itemAlvo.x) != OT_UNINIT;// TODO solução temporarária. tem um unintialized vindo de algum lugar
endfunction

/*
 * CalculaMaxDestrocos(item)
 *
 * Purpose
 * Calcula um valor aleatório baseado na altura do item.
 * Parameters
 * item:	   		item a ser avaliado
 *
 * Return value
 *  Retorna um inteiro.
 *
*/
function CalculaMaxDestrocos(item)
	var numeroMaximoDest := Ceil(item.height / 5);
	numeroMaximoDest := numeroMaximoDest > 1 ? numeroMaximoDest : 1;
	return RandomInt(cInt(numeroMaximoDest)) + 1;
endfunction

/*
 * AvaliaDisparoArmadilha(item)
 *
 * Purpose
 * Realiza a chance de disparo de armadilha de um item.
 * Parameters
 * item:	   		item com a  armadilha
 *
 * Return value
 *   true se o valor aleatório for menor ou igual a chance.
 *
*/
function AvaliaDisparoArmadilha(item)
	var armadilhas := GetObjProperty(item, PROPNAME_ARMADILHAS);
	if (armadilhas)
		var maiorForca := 0;
		foreach armadilha in armadilhas
			maiorforca := maiorForca < armadilha.strength ? armadilha.strength : maiorForca;
			Sleepms(2);
		endforeach
		if (RandomInt(100) < maiorForca * configDestruiveis[ELEM_CFG_DEST].multDisparoArmadilha)
			return true;
		endif
	endif
	return false;
endfunction

/*
 * MontaCatalogoDestrocos()
 *
 * Purpose
 * Guarda os Graphics que são usados nos destroços.
 * Parameters
 *
 * Return value
 *   um Dict com todos os diferentes tipos de material, sendo a chave <Type> do resourceList.cfg.
 *
*/
function MontaCatalogoDestrocos()
	var catalogo := dictionary;

	catalogo.insert(PEDRA, { 0x46C8, 0x46CB, 0x1038, 0x19BA, 0x19B8, 0x0F3B });
	catalogo.insert(TIJOLO, { 0x46C8, 0x46CB, 0x1038, 0x0F3B });
	catalogo.insert(MARMORE, { 0x46C8, 0x46CB, 0x1038, 0x19BA, 0x19B8, 0x0F3B });
	catalogo.insert(ARENITO, { 0x46C8, 0x46CB, 0x0F3B });
	catalogo.insert(OSSOS, { 0x1B1A, 0x1B19, 0x1B12, 0x1B11 });
	catalogo.insert(VIDRO, { 0x5738 });
	catalogo.insert(MADEIRA, { 0x0c2f, 0x0c2e, 0x1038, 0x1B9D, 0xABEA, 0xABED });
	catalogo.insert(PANO, { 0x0DD5, 0x0EE9, 0x1766 });
	catalogo.insert(METAL, { 0x1038, 0x1B26, 0x1B9D, 0x1B9E });
	catalogo.insert(COURO, { 0x4006, 0x1B9E });
	catalogo.insert(CORPO, { 0x122F, 0x1CF0, 0x1D10, 0x1D0E, 0x1D09, 0x1D0D });
	catalogo.insert(CORPO_HUMANO, { 0x1CED, 0x1CEE, 0x1CF0, 0x122F, 0x1CE2, 0x1CE9, 0x1CE6, 0x1CE4, 0x1CE1,
									0x1CE5, 0x1CEA, 0x1CDD, 0x1D10, 0x1D0E, 0x1D09, 0x1D0D });
	catalogo.insert(INDEFINIDO, { 0x1038, 0x1B26, 0x1B9D, 0x1B9E, 0x0F3B, 0x1038 });

	return catalogo;
endfunction

/*
 * TocaSFXDestruicao(itemAlvo,deveTocarTesouro)
 *
 * Purpose
 *	Toca o sound effect para cada material.
 *
 * Parameters
 * itemAlvo:	   		item que foi destruído.
 * deveTocarTesouro:	booleano, true se deve tocar som de tesouro ao destruir o item.
 *
 * Return value
 *   sem retorno
 *
*/
function TocaSFXDestruicao(itemAlvo, deveTocarTesouro)
	var material := "";
	var sfx := 0;

	if (itemAlvo.isA(POLCLASS_CORPSE))
		material := CORPO;
	else
		var materialId := GetObjProperty(itemAlvo, PROPNAME_MATERIAL);
		material := configRecursos[materialId].Type;
	endif

	case (material)
		PEDRA:        sfx := 0x11C; break;
		TIJOLO:       sfx := 0x11C; break;
		MARMORE:      sfx := 0x11C; break;
		ARENITO:      sfx := 0x11C; break;
		OSSOS:        sfx := 0x342; break;
		VIDRO:        sfx := 0x03F; break;
		MADEIRA:      sfx := 0x476; break;
		PANO:         sfx := 0x33A; break;
		METAL:        sfx := 0x057; break;
		COURO:        sfx := 0x04B; break;
		CORPO:        sfx := 0x146; break;
		CORPO_HUMANO: sfx := 0x146; break;
		default:      sfx := 0x33C;
	endcase
	PlaySoundEffect(itemAlvo, sfx);
	if (deveTocarTesouro)
		PlaySoundEffect(itemAlvo, 0x2E1);
	endif
endfunction

/*
 * LimpaObjPropsDestruiveis(itemAlvo)
 *
 * Purpose
 *	Lmpa as props utilizadas que não são mais necessárias ao final do script.
 *
 * Parameters
 * itemAlvo:	   		item que vai receber  a limpeza.
 *
 * Return value
 *   sem retorno
 *
*/
function LimpaObjPropsDestruiveis(itemAlvo)
	EraseObjProperty(itemAlvo, PROPNAME_DURABILIDADE);
endfunction

/*
 * PegaResistenciaItem(itemAlvo)
 *
 * Purpose
 *	Calcula e retorna a resistÊncia do item, baseado no seu material, se tem prop de multiplicador e em um valor base.
 *
 * Parameters
 * itemAlvo:	   		item que terá sua resistência calculada.
 * fonteDano: a fonte causadora do dano, pode ser um: ItemRef, Struct com membro <desc>, <dano> e <tipo> ou um valor real.
 *
 * Return value
 *   resistência total do item.
 *
 */
function PegaResistenciaItem(itemAlvo, fonteDano ,materialId, isInflamavel)
	var propRes := GetObjProperty(itemAlvo,PROPNAME_RESITEM_MULT);
	if(!propRes)
		propRes := itemAlvo.CProps[PROPNAME_RESITEM_MULT];
	endif
	var resistenciaFabricacao := PegaValorFabricacao(itemAlvo,CARACTERISTICA_FABRICACAO_RESISTENCIA);

	if(!resistenciaFabricacao)
		var efeito := InterpretaMaterialEfeitos(CARACTERISTICA_FABRICACAO_RESISTENCIA,materialId);
		resistenciaFabricacao := efeito.valorPrincipal * 2;
	endif

	var resFinal := configDestruiveis[ELEM_CFG_DEST].resistenciaBase;
	if (propRes)
		resFinal := resFinal * propRes;
	endif
	if (materialId)
		resFinal := resFinal + resistenciaFabricacao;
		resFinal := isInflamavel && fonteDano.tipo == "Fire" ? 0 : resFinal;
	endif

	return resFinal;
endfunction

/*
 * CalculaDanoRecebido(fonteDano, atributosItem)
 *
 * Purpose
 *	Calcula o dano recebido, após subtração da resistência do item.
 *
 * Parameters
 * quemDanificou:       O mobile que geroou o dano
 * fonteDano:	   		a fonte causadora do dano, pode ser um: ItemRef, Struct com membro <desc> e <dano> ou um valor real.
 * atributosItem:	   	atributos do item que receberá o dano. comtem a durabilidade, resistencia e durabildiade total.
 *
 * Return value
 *   retorna um struct com danoCalculado.+dano valor total do dano e .+isEficaz, um boolean informando se foi eficaz.
 *
*/
function CalculaDanoRecebido(quemDanificou, fonteDano, atributosItem)
	var danoCalculado := struct;
	if (atributosItem.isIndestrutivel)
		danoCalculado.+isEficaz := false;
		return danoCalculado;
	endif

	var danoCru := 0;
	if (fonteDano.ISA(POLCLASS_WEAPON))
		var danocausado := 0;
		var attackInfo := AttackInfo(MobileWrapper(quemDanificou), 0);
		attackInfo.SetWeapon(WeaponWrapper(fonteDano));
		CalculateAttackerDamage( attackInfo);
		
		if (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::SLASH)
			danocausado += attackInfo.GetDamage().GetSlashRawDamage();
		elseif (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::PIERCING)
			danocausado += attackInfo.GetDamage().GetPiercingRawDamage();
		elseif (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::BLUNT)
			danocausado += attackInfo.GetDamage().GetBluntRawDamage();
		endif

		danocausado += attackInfo.GetDamage().GetFireRawDamage() +  attackInfo.GetDamage().GetColdRawDamage() + attackInfo.GetDamage().GetPoisonRawDamage() + attackInfo.GetDamage().GetEnergyRawDamage();
		danoCru := danocausado;
	elseif (TypeOf("Struct"))
		danoCru := fonteDano.dano;
	else
		danoCru := fonteDano;
	endif
	
	if(fonteDano.tipo == "Fire")
		if(atributosItem.inflamabilidade > 0)
			danoCru := danoCru * (1 + (atributosItem.inflamabilidade / 2));
		else
			danoCru := danoCru / 2;
		endif
	endif

	var danoRecebido := cint(danoCru - atributosItem.resistencia);
	danoCalculado.+dano := danoRecebido < 0 ? 0 : danoRecebido;
	danoCalculado.+isEficaz := danoRecebido > 0;
	return danoCalculado;
endfunction

/*
 * PegaDurabilidadeTotal(item)
 *
 * Purpose
 *	Calcula a durabilidade total do item que está recebendo dano.
 *	Se for equipamento, entao: MaxHP.
 *	Se não for, então  escolhe uma precedência, sendo: Prop  > peso itemdesc > peso Tiles.cfg. e soma-se um modificador do material.
 *
 * *Nota*
 *  Se  o teim não tiver a propriedade de durabilidade total, ele coloca.
 * Parameters
 * item: item que terá sua durabildiade estabelecida.
 *
 * Return value
 *   o valor da durabildiade total.
 *
*/
function PegaDurabilidadeTotal(item)

	if (item.ISA(POLCLASS_EQUIPMENT))
		return item.maxhp;
	else
		var itemdesc := ReadConfigFile(":*:itemdesc");
		var elem := FindConfigElem(itemdesc, item.objtype);
		var pesoEscolhido := 0;
		var materialId := GetObjProperty(item, PROPNAME_MATERIAL);
		var recursoMod := configRecursos[materialId].hpmod;
		var fabricacaoDurabilidade := PegaValorFabricacao(item, CARACTERISTICA_FABRICACAO_DURABILIDADE);
		fabricacaoDurabilidade := (fabricacaoDurabilidade / 10) + 1;

		var propMaxDurab := GetObjProperty(item, PROPNAME_DURABILIDADE_MAX);
		if (GetObjProperty(item, PROPNAME_DURABILIDADE_MAX))
			if (recursoMod)
				propMaxDurab := propMaxDurab + recursoMod;
			endif
			return propMaxDurab;
		endif

		if (elem)
			if (elem.weight > 0)
				pesoEscolhido := CInt(elem.weight);
				return Ceil(pesoEscolhido) * configDestruiveis[ELEM_CFG_DEST].hpMaxMultiplicador + recursoMod;
			endif
		endif

		var infoTile := IU_GetTileInfo(item.Graphic);
		pesoEscolhido := CInt(infoTile.Weight * configDestruiveis[ELEM_CFG_DEST].hpMaxMultiplicador * fabricacaoDurabilidade);
		return Ceil(pesoEscolhido) > 0 ? pesoEscolhido : 1;
	endif
endfunction

/*
 * IsEmperrado(item)
 *
 * Purpose
 *	Verifica se o item satisfaz a condição de emperrado.
 *
 * Parameters
 * item: item a ser avaliado
 *
 * Return value
 *   true caso o item esteja emperrado.
 *
*/
function IsEmperrado(item)
	if (!item.ISA(POLCLASS_LOCKABLE))
		return false;
	endif

	var atributos := MontaAtributosItem(item);
	return atributos.durabTotal / 2 > atributos.durabilidade;
endfunction

/*
 * MontaAtributosItem(item)
 *
 * Purpose
 *	Monta os atributos do item que está sendo danificado neste evento.
 *
 * Parameters
 * item: item a ser avaliado.
 * fonteDano: a fonte causadora do dano, pode ser um: ItemRef, Struct com membro <desc>, <dano> e <tipo> ou um valor real. Parametro opcional.
 *
 * Return value
 *   retorna um struct com:
 *		.+resistencia //valor calculado da resistência do objeto.
 *		.+durabTotal //valor da durabildiade máxima do objeto
 *		.+durabilidade //valor da durabildiade atual do objeto
 *		.+isIndestrutivel // Se o item é considerádo indestrutivel ou não.
 *
*/
function MontaAtributosItem(item, fonteDano := 0)

	var materialId := GetObjProperty(item, PROPNAME_MATERIAL);
	if(!materialId)
		materialId := item.CProps[PROPNAME_MATERIAL];
	endif
	var config := PegaMaterialConfig(materialId);

	var itemAtrib := struct;
	itemAtrib.+inflamabilidade := config.inflamavel;
	itemAtrib.+resistencia := cInt(PegaResistenciaItem(item, fonteDano, materialId,
														itemAtrib.inflamabilidade));
	itemAtrib.+durabTotal := cInt(PegaDurabilidadeTotal(item));
	itemAtrib.+isIndestrutivel := GetObjProperty(item, PROPNAME_INDESTRUTIVEL);

	if (item.ISA(POLCLASS_EQUIPMENT))
		itemAtrib.+durabilidade := item.hp;
		SetObjProperty(item, PROPNAME_DURABILIDADE, item.hp);
	else
		var propDur := GetObjProperty(item, PROPNAME_DURABILIDADE);
		if (!propDur)
			SetObjProperty(item, PROPNAME_DURABILIDADE, itemAtrib.durabTotal);
			itemAtrib.+durabilidade := itemAtrib.durabTotal;
		else
			itemAtrib.+durabilidade := propDur;
		endif
	endif
	return itemAtrib;
endfunction

/*
 * DanificaItem(quemDanificou, item, fonteDano, gfx, gfxHue)
 *
 * Purpose
 *	Responsável por executar a lógica de um item danificado. caso o item não tenha mais durabilidade,
 *	chama o metódo Destruir.
 *
 * Parameters
 * quemDanificou: O ator que danificou o item.
 * item: O item que foi danificado
 * fonteDano: a fonte causadora do dano, pode ser um: ItemRef, Struct com membro <desc>, <dano> e <tipo> ou um valor real.
 * gfx: o efeito especial que será disparado no item danificado.
 * gfxHue: a cor do efeito especial..
 *
 * Return value
 *   retorna um struct com o dano calculado, sendo sua estrutura:
 *		.+dano //valor total do dano recebido.
 *		.+isEficaz // true se o dano foi eficaz.
 *		.+isDestruido // true se o objeto foi destruído após a aplicação do dano.
 *
*/
function DanificaItem(quemDanificou, item, fonteDano, gfx := 0, gfxHue := 0)
	if (item.concealed || item.invisible)
		return struct;
	endif

	var atributosItem := MontaAtributosItem(item, fonteDano);
	var danoCalculado := CalculaDanoRecebido(quemDanificou, fonteDano, atributosItem);

	if (item.IsA(POLCLASS_CORPSE) && GetObjProperty(item, "account")) // Corpo de jogador.
		danoCalculado.isEficaz := false;
	endif

	if (!danoCalculado.isEficaz)
		return danoCalculado;
	endif

	var durabilidadeRestante := atributosItem.durabilidade - danoCalculado.dano;

	if (Item.ISA(POLCLASS_EQUIPMENT))
		item.hp := CInt(durabilidadeRestante);
	endif

	if (gfx != 0)
		if (gfxHue)
		// PlayObjectCenteredEffectHuefx(item, gfx, 5, 6,gfxHue); // Conflito com includes por funcões duplicadas =(
		else
			PlayObjectCenteredEffect(item, gfx, 5, 6);
		endif
	endif

	if (DeveCriarDestroco(item))
		if (RandomInt(100) <= configDestruiveis[ELEM_CFG_DEST].criaDestrocoChance)
			CriaDestrocos(item, 1, 3, false);
		endif
	endif
	
	SetObjProperty(item,PROPNAME_DURABILIDADE,durabilidadeRestante);
	if(!IsDestroco(item))
		DisplayDamage(item,danoCalculado.dano,quemDanificou);
		EnviaEventoDanoItem(quemDanificou,item,atributosItem);
	endif

	if (durabilidadeRestante <= 0)
		Destruir(quemDanificou, item, fonteDano);
		danoCalculado.+isDestruido := true;
	else
		if (AvaliaDisparoArmadilha(item))
			RunTraps(item, quemDanificou);
		endif
	endif

	return danoCalculado;
endfunction

function IsDestroco(item)
	return GetObjProperty(item, PROPNAME_DESTROCO);
endfunction

/*
 * DanificaObjetosProximos(item)
 *
 * Purpose
 *	Avaliar se há necessidade de criar destroço do item danificado.
 *
 * Parameters
 * item: o item que foi danificado
 *
 * Return value
 *   true se deve os destroços devem ser criados
*/
function DeveCriarDestroco(item)
	return !IsDestroco(item) && item.weight > 5;
endfunction

/*
 * DanificaObjetosProximos(quem,x,y,z,realm,dano,alcance, gfx := 0, igrnoreSerial :=0,  gfxHue := 0)
 *
 * Purpose
 *	Danifica todos os itens próximos a uma coordenada x ,y, z.
 *
 * Parameters
 * quem: O ator que danificou o item.
 * x: coordenada x do ponto central da "esfera" de efeito.
 * y: coordenada y do ponto central da "esfera" de efeito.
 * z: coordenada z do ponto central da "esfera" de efeito.
 * realm: o reino  do item danificado.
 * dano: a quantidade de dano desferida.
 * alcance: o raio de efeito.
 * gfx: o efeito especial que será disparado no item danificado.
 * igrnoreSerial: uma serial que será ignorada pela esfera de influencia, usada para que o dano seja ignorado naquele item. (armadilha, por exemplo.)
 * gfxHue: a cor do efeito especial..
 *
 * Return value
 *   sem retorno.
 *
*/
function DanificaObjetosProximos(quem, x, y, z, realm, dano, alcance, gfx := 0, ignoreSerial := 0,
								 gfxHue := 0)
	var itens := ListItemsNearLocation(x, y, z, alcance, realm);
	foreach item in itens
		if (item.invisible)
			continue;
		endif
		if (CheckLosBetween(x, y, z, item.x, item.y, item.z, realm) && item.serial != ignoreSerial)
			DanificaItem(quem, item, dano, gfx, gfxHue);
		endif
		Sleepms(3);
	endforeach
endfunction

/*
 * CriaDestrocos(itemAlvo,quantidade,alcanceMax := 0, deveCriarOrigem := true)
 *
 * Purpose
 *	Cria e espalha destroços de algum item, levando em conta seu material.
 *
 * Parameters
 * itemAlvo: o item que está sendo danificado..
 * quantidade: A quantidade de destroços que será criada.
 * alcanceMax: O alcance máximo de lançamento dos destroços
 * deveCriarOrigem: true se o primeiro destroço sempre vai na origem.
 *
 * Return value
 *   sem retorno.
 *
*/
function CriaDestrocos(itemAlvo, quantidade, alcanceMax := 0, deveCriarOrigem := true)
	var materialId := GetObjProperty(itemAlvo, PROPNAME_MATERIAL);

	var arrayDestrocos := array;
	var corDestroco := 1001;
	var material := "";

	if (materialId)
		arrayDestrocos := catalogoDestrocos[configRecursos[materialId].Type];
		corDestroco := configRecursos[materialId].Color;
		material := configRecursos[materialId].Type;
	else
		if (itemAlvo.ISA(POLCLASS_CORPSE))
			var templateProp := GetObjProperty(itemAlvo, "npctemplate");
			if (templateProp)
				var parsed := NPC_ParseTemplateName(templateProp);
				var cfg := ReadConfigFile(":brainai:npcdesc");
				case (cfg[parsed.template].Taxonomy)
					"homo":
						arrayDestrocos := catalogoDestrocos[CORPO_HUMANO];
						material := CORPO_HUMANO;
						corDestroco := 0;
						break;
					default:
						arrayDestrocos := catalogoDestrocos[CORPO];
						material := CORPO;
						corDestroco := 0;
				endcase
			endif
		else
			arrayDestrocos := catalogoDestrocos[INDEFINIDO];
		endif
	endif

	var i;
	for (i := 1; i <= quantidade; i += 1)
		var adicionalX := RandomInt(alcanceMax) * PositivoNegativo(RandomInt(100));
		var adicionalY := RandomInt(alcanceMax) * PositivoNegativo(RandomInt(100));
		var destrocoGraphic := arrayDestrocos.randomentry();
		var destrocoPos := struct;
		destrocoPos.+x := itemAlvo.x + adicionalX;
		destrocoPos.+y := itemAlvo.y + adicionalY;
		destrocoPos.+z := itemAlvo.z;

		if (!CheckLosBetween(itemAlvo.x, itemAlvo.y, itemAlvo.z, destrocoPos.x, destrocoPos.y, destrocoPos.z,
	  itemAlvo.realm) || (deveCriarOrigem && i == 1))
			destrocoPos.x := itemAlvo.x;
			destrocoPos.y := itemAlvo.y;
			destrocoPos.z := itemAlvo.z;
		endif

		PlayMovingEffectXYZEx(itemAlvo.x, itemAlvo.y, itemAlvo.z, destrocoPos.x, destrocoPos.y,
							   destrocoPos.z, itemAlvo.realm, destrocoGraphic, 2, 2, corDestroco);

		var destroco := CreateItemAtLocation(destrocoPos.x, destrocoPos.y, destrocoPos.z, destrocoGraphic);
		ConfiguraDestroco(destroco, material, corDestroco);
		Sleepms(2);
	endfor
endfunction

/*
 * ConfiguraDestroco(byref destroco,tipo,corDestroco)
 *
 * Purpose
 *	~Monta as configurações de um destroço que acaba de ser criado.
 *
 * Parameters
 * destroco: o destroço.
 * quantidade: tipo o tipo de material do destroço.
 * corDestroco: a cor do material.
 *
 * Return value
 *   sem retorno.
 *
*/
function ConfiguraDestroco(byref destroco, tipo, corDestroco)
	destroco.color := corDestroco;
	destroco.movable := 1;
	destroco.decayat := ReadGameClock() + configDestruiveis[ELEM_CFG_DEST].decayDestroco;
	var novoNome := "destroços de ";
	case (tipo)
		PEDRA:   novoNome := novoNome + "pedra"; break;
		TIJOLO:  novoNome := novoNome + "tijolo"; break;
		MARMORE: novoNome := novoNome + "marmore"; break;
		ARENITO: novoNome := novoNome + "arenito"; break;
		OSSOS:   novoNome := novoNome + "ossos"; break;
		VIDRO:   novoNome := novoNome + "vidro"; break;
		MADEIRA: novoNome := novoNome + "madeira"; break;
		PANO:    novoNome := novoNome + "pano"; break;
		METAL:   novoNome := novoNome + "metal"; break;
		CORPO:
		CORPO_HUMANO:
			novoNome := "Restos de corpo";
			break;
		default: novoNome := "destroços";
	endcase
	SetObjProperty(destroco, PROPNAME_DESTROCO, 1);
	SetName(destroco, novoNome);
endfunction

/*
 * VerificaRegiaoSegura(item)
 *
 * Purpose
 *	Verifica se a região onde o item está é considerada segura (Architect ou estalagem.)
 *
 * Parameters
 * item: O item.
 *
 * Return value
 *   true caso esteja numa região segura.
 *
*/
function VerificaRegiaoSegura(item)
	return isDentroAreaArchitecct(item) || isDentroQuartoEstalagem(item);
endfunction

/*
 * isDentroQuartoEstalagem(objeto)
 *
 * Purpose
 *	Verifica se o objeto se encontra dentro de um quarto de estalagem.
 *
 * Parameters
 * item: O objeto.
 *
 * Return value
 *   true caso esteja dentro de um quarto de estalagem.
 *
*/
function isDentroQuartoEstalagem(objeto)
	var estalajadeiros := getglobalproperty("InnKeeper");
	foreach estalajadeiro in estalajadeiros
		Sleepms(2);
		if (estalajadeiro.errortext)
			continue;
		endif

		var quartos := GetObjProperty(estalajadeiro, "quartos");

		foreach quarto in quartos
			var centro := quarto.centro;
			var raio := 10; // mesma distância que usaram no estalagemcontrol.src
			if (centro)
				var distanciaEuclidiana := CoordinateDistanceEuclidean(objeto.x, objeto.y, centro[1], centro[2]);
				if (distanciaEuclidiana <= raio)
					return true;
				endif
			endif
			Sleepms(2);
		endforeach
	endforeach

	if (estalajadeiros[1] == error)
		return false;
	endif

	return false;
endfunction

/*
 * isDentroAreaArchitecct(objeto)
 *
 * Purpose
 *	Verifica se o objeto se encontra dentro de uma region do architect
 *
 * Parameters
 * item: O objeto.
 *
 * Return value
 *   true caso esteja dentro de uma region do architect.
 *
*/
function isDentroAreaArchitecct(objeto)
	var area := GetResidenciasRegion(objeto);

	return !(area == "Whole_World");
endfunction

/*
 * ConfiguraAlvo(quem,alvo,cfgselecao, mostrarMensagem := true)
 *
 * Purpose
 *	configura as propriedades de um alvo do gump do comando destruiveis.
 *
 * Parameters
 * quem: O ator que executou o comando.
 * alvo: item que será configurado.
 * cfgselecao: as configurações escolhidas no gump.
 * mostrarMensagem: true caso seja necessário mandar a mensagem individual.
 *
 * Return value
 *   sem retorno.
 *
 */
function ConfiguraAlvo(quem,alvo,cfgselecao, mostrarMensagem := true)
	if (alvo.invisible)
		return 0;
	endif
	if(alvo.ISA(POLCLASS_ITEM) && !alvo.ISA(POLCLASS_EQUIPMENT))
		
		if (GetObjProperty(alvo, "PropStealable"))
			SendSysMessageEx(quem, "Este item é um artifact roubável e não pode ser configurado como destrutível.", SSM_FAIL);
			return 0;
		endif
		
		var material := configRecursos[cfgselecao.material];

		if (cfgselecao.durabTotal)
			SetObjProperty(alvo, PROPNAME_DURABILIDADE_MAX, CInt(cfgselecao.durabTotal));
			var atributos := MontaAtributosItem(alvo);
			SetObjProperty(alvo, PROPNAME_DURABILIDADE, CInt(atributos.durabTotal));
		endif
		if (cfgselecao.mltRes)
			SetObjProperty(alvo, PROPNAME_RESITEM_MULT, CDbl(cfgselecao.mltRes));
		endif
		if (cfgselecao.material)
			SetObjProperty(alvo, PROPNAME_SEL_AREA_IS_COR_ORIGINAL_ALTERADA, true);
			alvo.color := material.Color;
			SetObjProperty(alvo, PROPNAME_MATERIAL, Cint(cfgselecao.material));
		endif
		if (cfgselecao.isEmperrada)
			if (alvo.ISA(POLCLASS_LOCKABLE))
				var durabTotal := GetObjProperty(alvo, PROPNAME_DURABILIDADE_MAX);
				SetObjProperty(alvo, PROPNAME_DURABILIDADE, CInt(durabTotal / 3));
			else
				SendSysMessageEx(quem, " Configuração [Emperrada] pulada. O item {} não é trancável.".format(alvo.desc),
								  SSM_INFO_IC);
			endif
		endif
		SetObjProperty(alvo, PROPNAME_DISPENSAVEL_FLAG, Cint(cfgselecao.isDispensavel));
		SetObjProperty(alvo, PROPNAME_INDESTRUTIVEL, Cint(cfgselecao.isIndestrutivel));

		if (mostrarMensagem)
			SendSysMessageEx(quem, "Configurações  aplicadas.", SSM_INFO);
		endif
	endif
endfunction
