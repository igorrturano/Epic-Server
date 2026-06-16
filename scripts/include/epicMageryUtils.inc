use os;
use math;
use util;

include "client";
include "include/epicConstants";
include "include/say";
include ":charactercreation:habilidades";
include ":abilities:models/Ability";
include ":wrappers:Objects/Model/MobileWrapper";
include "include/packets";
include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":tn:cooldown";
include ":combat:Model/Damage";

function playAltarAnimations(who, item)
	var altar := altarByObjtype(who, item);
	var newGraphic := Cint(graphicByAltar(altar));
	item.graphic := newGraphic;
	sleep(2);
	item.graphic := item.objtype;
	var effect := effectByAltar(altar);
	PlaySoundEffect(who, SFX_SPELL_HEAL);
	PlayStationaryEffectEx(who.x, who.y, who.z, who.realm, effect, 255, 80, 0, 0);
endfunction

function effectByAltar(altar)
	var effect := 0;
	case (altar)
		ELUITHORTH:  effect := 0x54F7;
		LORE:        effect := 0xB462;
		VENERATIO:   effect := 0x54B3;
		BOKK_BHARAZ: effect := 0x6DF1;
		KAI:         effect := 0x6d57;
		USHKRAR:     effect := 0X6C91;
		CETICOS:     effect := 0;
		HALLEUAD:    effect := 0x6CEB;
		ARADALORE:   effect := 0xA2CC;
	endcase

	return effect;
endfunction

function altarByObjtype(who, altar)
	var objtype := altar.objtype;
	var religion := "";
	case (objtype)
		0xA4A8: // 0xFA67
			religion := ELUITHORTH;
		0xA06F: // 0xFA68
			religion := LORE;
		0xA065: // 0xFA63
			religion := VENERATIO;
		0x9E81: // 0xFA64
			religion := BOKK_BHARAZ;
		0xA252: // 0xFA65
			religion := KAI;
		0xA06A: // 0xFA66
			religion := USHKRAR;
		0xA261: religion := CETICOS;
		0xA060: religion := HALLEUAD;
		0xA49E: religion := ARADALORE;
	endcase

	return religion;
endfunction

function graphicByAltar(altar)
	var graphic := 0;
	case (altar)
		ELUITHORTH:  graphic := 0xA4A9; // 0xFA20;
		LORE:        graphic := 0xA070; // 0xFA25;
		VENERATIO:   graphic := 0xA066; // 0xD1EA;
		BOKK_BHARAZ: graphic := 0x9DF2; // 0xD1EF;
		KAI:         graphic := 0xA253; // 0xD1F4;
		USHKRAR:     graphic := 0xA06B; // 0xD1F9;
		HALLEUAD:    graphic := 0xA061;
		CETICOS:     graphic := 0xA262;
		ARADALORE:   graphic := 0xA49F;
		default:     graphic := 0;
	endcase
	return graphic;
endfunction

function distanceAllowedForSpells(who, skill, school)
	// Base: 5 + skill/10, with a floor of 7 (early-game survivability) and
	// a cap of 12 at skill 70 (graceful late-game plateau).
	var base := 5 + cint(skill / 10);
	if (base < 7)
		base := 7;
	endif
	if (base > 12)
		base := 12;
	endif
	var dist_allowed := base;

	// Specialist on own school, or NPC: small +skill/30 nudge, capped at +3.
	var spec_bonus := 0;
	if (MobileWrapper(who).HasAbility(ABILITY_ID::EVOCATOR) && school == EVOCATIUM)
		spec_bonus := cint(skill / 30);
	elseif (MobileWrapper(who).HasAbility(ABILITY_ID::VALDRUK) && school == VELKYN)
		spec_bonus := cint(skill / 30);
	elseif (who.npctemplate)
		spec_bonus := cint(skill / 30);
	endif
	if (spec_bonus > 3)
		spec_bonus := 3;
	endif
	dist_allowed := dist_allowed + spec_bonus;

	// Caminho: Spell Range ability (+5 tiles, always meaningful — top reach
	// of 20 is hit by specialist + this ability at 100 skill).
	if (MobileWrapper(who).HasAbility(ABILITY_ID::SPELL_RANGE))
		dist_allowed := dist_allowed + 5;
	endif

	return dist_allowed;
endfunction

function _HasSchoolSpecialist(who, school)
	var mw := MobileWrapper(who);
	if (school == EVOCATIUM && mw.HasAbility(ABILITY_ID::EVOCATOR))
		return 1;
	endif
	if (school == VELKYN && mw.HasAbility(ABILITY_ID::VALDRUK))
		return 1;
	endif
	if (school == KAIJIN && mw.HasAbility(ABILITY_ID::CETRO_DA_MORTE))
		return 1;
	endif
	if (school == ARADALORE && mw.HasAbility(ABILITY_ID::CORACAO_DA_MATILHA))
		return 1;
	endif
	return 0;
endfunction

function epicSpellDamageRoll(who, spell_circle, school := 0)
	var dice := cstr("1d4");
	if (spell_circle == 1)
		dice := cstr("2d4");
	elseif (spell_circle == 2)
		dice := cstr("4d6");
	elseif (spell_circle == 3)
		dice := cstr("6d8");
	elseif (spell_circle == 4)
		dice := cstr("8d10");
	elseif (spell_circle == 5)
		dice := cstr("10d12");
	endif

	var dmg := RandomDiceRoll(dice);

	var bonus := epicWisdomPathBonus(who);
	dmg := Cint(dmg + bonus);

	return dmg;
endfunction

function bonusSpellDamageForSpecialists(who, dmg, school)
	if (school != 0 && _HasSchoolSpecialist(who, school))
		return Cint(dmg * 1.2);
	endif
	return dmg;
endfunction

function handleSilentMagic(who, targ := 0, sound := 0)
	if (!GetObjProperty(who, "#magiasilenciosa") && sound != 0 && targ != 0)
		PlaySoundEffect(targ, sound);
		return 0;
	endif
	if (GetObjProperty(who, "#magiasilenciosa"))
		SendSysMessageEx(who, "O efeito da magia silenciosa acabou.", SSM_INFO);
		EraseObjProperty(who, "#magiasilenciosa");
	endif

	return 0;
endfunction

function handleSpellResistDiffForSpecialists(who, school)
	var diff := RESIST_EASY;
	if (MobileWrapper(who).HasAbility(ABILITY_ID::EVOCATOR) && school == EVOCATIUM)
		diff := RESIST_MEDIUM;
	elseif (MobileWrapper(who).HasAbility(ABILITY_ID::VALDRUK) && school == VELKYN)
		diff := RESIST_MEDIUM;
	elseif (MobileWrapper(who).HasAbility(ABILITY_ID::CETRO_DA_MORTE) && school == KAIJIN)
		diff := RESIST_MEDIUM;
	endif

	return diff;
endfunction

function bonusSpellDurationForSpecialists(who, duration, school)
	var bonus := 1.2;

   if (MobileWrapper(who).HasAbility(ABILITY_ID::EVOCATOR) && school == EVOCATIUM)
	  duration := Cint(duration * 1.5);  // Evocator: +50% duration
   elseif (MobileWrapper(who).HasAbility(ABILITY_ID::VALDRUK) && school == VELKYN)
	  duration := Cint(duration * bonus);
   elseif(MobileWrapper(who).HasAbility(ABILITY_ID::CORACAO_DA_MATILHA) && school == ARADALORE)
	  duration := Cint(duration * bonus);
   endif

	return duration;
endfunction

function checkForElementalWeapons(who, targ)

	if (GetCooldown(targ, "blessweapon") > 0)
		SendSysMessageEx(who, "Essa arma já está com um encantamento.", SSM_FAIL);
		return 1;
	endif

	if (targ.objtype == FIST)
		SendSysMessageEx(who, "Você não pode encantar suas mãos.", SSM_FAIL);
		return 1;
	endif

	return 0;
endfunction

function epicWisdomPathBonus(who)
	var int_bonus := Cint((AP_GetStat(who, INTELLIGENCE) - 50) / 10);
	var mlore := Cint(HandleSpellSkill(who) / 10);

	return (int_bonus + mlore);
endfunction

/*
 * Purpose -
 * Função para aplicar o efeito de arma elemental do swordmage
 *
 * Parameters -
 * who:	A creatura que terá sua arma encantada.
 * elemento: Que elemento será aplicado.
 *
 * Return value -
 *  0  caso a creatura não esteja equipada.
*/
function ApplyElementalWeaponProperties(who, elemento)

	var weapon := who.weapon;
	if (weapon == error)
		return 0;
	endif

	// Normalize legacy English keys ("Fire"/"Cold"/"Energy") into the
	// canonical DAMAGE_TYPE::* enum values ("Fogo"/"Gelo"/"Energia").
	// The 14 swordmage follow-up spells, spiritualBladeControl, and the
	// combat hit-script paths now all compare against DAMAGE_TYPE::*; the
	// English-string storage left over from before bf72d738a was making
	// every check silently fail, which read in-game as "blade enchanted
	// but no attack-elemental / no charge consumption / damage never
	// stops".
	if (elemento == "Fire" || elemento == DAMAGE_TYPE::FIRE)
		elemento := DAMAGE_TYPE::FIRE;
	elseif (elemento == "Cold" || elemento == DAMAGE_TYPE::COLD)
		elemento := DAMAGE_TYPE::COLD;
	elseif (elemento == "Energy" || elemento == DAMAGE_TYPE::ENERGY)
		elemento := DAMAGE_TYPE::ENERGY;
	endif

	var elementalPropAtual := GetObjProperty(weapon, "#elementalweapon");
	var jaEncantada := elementalPropAtual != error;

	var dmgdice := DefineElementalWeaponDice(who);
	var cargas := Floor(AP_GetSkill(who, ARCANA) / 4);
	cargas := cargas <= 0 ? 1 : cargas; // no minimo uma carga.

	var elementalWeapon := struct;
	elementalWeapon.+type := elemento;
	elementalWeapon.+serial := who.serial;
	elementalWeapon.+bonus := dmgdice;
	elementalWeapon.+cargas := cargas;
	elementalWeapon.+cargaMaxima := cargas;
	elementalWeapon.+endtime := ReadGameClock() + 1200;
	if (!jaEncantada)
		elementalWeapon.+originalColor := weapon.color;
	else
		elementalWeapon.+originalColor := elementalPropAtual.originalColor;
		var decayScript := GetProcess(elementalPropAtual.decayScriptPid);
		decayScript.SendEvent({ weapon, elementalWeapon.endtime });

	endif
	elementalWeapon.decayScriptPid := start_script(":spells:elementalweapon", { weapon,
																				elementalWeapon.endtime }).pid;

	// `elemento` was normalized to DAMAGE_TYPE::* above, so compare against
	// the canonical enum values here.
	if (elemento == DAMAGE_TYPE::FIRE)
		weapon.color := COLOR_ELEMENTAL_FIRE_LIGHT;
	elseif (elemento == DAMAGE_TYPE::COLD)
		weapon.color := COLOR_ELEMENTAL_ICE_LIGHT;
	elseif (elemento == DAMAGE_TYPE::ENERGY)
		weapon.color := COLOR_ELEMENTAL_ENERGY_LIGHT;
	endif

	IncRevision(weapon);
	SetObjProperty(weapon, "#elementalweapon", elementalWeapon);

endfunction

/*
 * Purpose -
 * Resposável por definir o dado de dano de uma aram encantada
 *
 * Parameters -
 * who:	A creatura que será avaliada.
 *
 * Return value -
 *  uma string especificando qual dado de dano serrá utilizado
*/
function DefineElementalWeaponDice(who)

	var faixaMLore := AP_GetSkill(who, ARCANA);
	var dmgdice := "0";

	if (faixaMLore < 40)
		dmgdice := "1d2";
	elseif (faixaMLore >= 40 && faixaMLore < 70)
		dmgdice := "1d4";
	elseif (faixaMLore >= 70 && faixaMLore < 90)
		dmgdice := "1d6";
	elseif (faixaMLore >= 90)
		dmgdice := "1d8";
	endif

	return dmgdice;
endfunction

/*
 * Purpose -
 * Resposável por consumir as cargas arcanas de uma arma equipada.
 *
 * Parameters -
 * who:	A creatura que terá cargas removidas da arma.
 * quantidade:	quantas cargas serão removidas.
 *
 * Return value -
 * [struct].divisor: Será o divisor de eficiência, pois consumimos mais do que tinhamos.
 * [struct].isSucesso: retorna true se reduzimos ou false caso a arma não esteja encantada.
*/

/*
 * Purpose -
 * Resposável por consumir as cargas arcanas de um item.
 *
 * Parameters -
 * item:	item que terá cargas removidas.
 * quantidade:	quantas cargas serão removidas.
 * consumeAll: se deve ignorar a quantidade e consumir todas.
 *
 * Return value -
 * [struct].divisor: Será o divisor de eficiência, pois consumimos mais do que tinhamos.
 * [struct].isSucesso: retorna true se reduzimos ou false caso a arma não esteja encantada.
*/
function ConsumeItemArcaneCharges(item, quantidade, consumeAll := false)

	var elementalWeapon := GetObjProperty(item, "#elementalweapon");
	var retorno := struct;
	retorno.+divisor := 1;
	retorno.+isSucesso := true;

	if (elementalWeapon != error)
		var typeColor;
		if (elementalWeapon.type == DAMAGE_TYPE::FIRE)
			typeColor := COLOR_ELEMENTAL_FIRE_LIGHT;
		elseif (elementalWeapon.type == DAMAGE_TYPE::COLD)
			typeColor := COLOR_ELEMENTAL_ICE_LIGHT;
		elseif (elementalWeapon.type == DAMAGE_TYPE::ENERGY)
			typeColor := COLOR_ELEMENTAL_ENERGY_LIGHT;
		endif

		var cargasAtualizada := 0;
		if (!consumeAll)
			cargasAtualizada := elementalWeapon.cargas - quantidade;
		endif

		if (cargasAtualizada <= 0) // remove as propiedades do encantamento
			retorno.divisor := cargasAtualizada == 0 ? 1 : cargasAtualizada * -1;
			EraseObjProperty(item, "#elementalweapon");
			item.color := elementalWeapon.originalColor;
			if (item.character_owner != error)
				PlaySoundEffect(item.character_owner, SFX_SPELL_SUMMON_CREATURE);
				SendSysMessageEx(item.character_owner, "Sua arma nao esta mais encantada.", SSM_INFO);
			endif
		else

			elementalWeapon.cargas := cargasAtualizada; // Atualiza as cargas

			var faixaCor := elementalWeapon.cargas * 100 / elementalWeapon.cargaMaxima; // decide a faixa de cor
			if (faixaCor <= 25) // Aplica a nova cor.
				item.color := typeColor - 4;
			elseif (faixaCor > 25 && faixaCor <= 50)
				item.color := typeColor - 3;
			elseif (faixaCor > 50 && faixaCor <= 75)
				item.color := typeColor - 2;
			elseif (faixaCor > 75 && faixaCor <= 90)
				item.color := typeColor - 1;
			endif
			SetObjProperty(item, "#elementalweapon", elementalWeapon);
		endif
		IncRevision(item);
		return retorno;
	else
		retorno.isSucesso := false;
		return retorno;
	endif
endfunction

function LogOffElementalWeapon(who)
	foreach item in (EnumerateItemsInContainer(who))
		var elementalWeapon := GetObjProperty(item, "#elementalweapon");
		if (elementalWeapon != error)
			ConsumeItemArcaneCharges(item, 0, true);
		endif
	endforeach
endfunction


function ApplyChargesCalculation(weapon, dmg, chargesAmount)
	var cargasRetorno := ConsumeItemArcaneCharges(weapon, chargesAmount);
	var damage := Floor(dmg / cargasRetorno.divisor);
	return damage;
endfunction

function FailSpell(who, spellinfo := 0)
	who.frozen := 0;
	EraseObjProperty(who, "#medding");
	EraseObjProperty(who, "#Casting");
	PlayObjectCenteredEffect(who, FX_FIZZLE, 5, 50);
	PlaySoundEffect(who, SFX_SPELL_FAIL);
	return 1;
endfunction

function EquipmentCheck(who)
	var hand_1 := GetEquipmentByLayer(who, LAYER_HAND1);
	if (hand_1)
		if (!hand_1.IsA(POLCLASS_SPELLBOOK) && !GetObjProperty(hand_1, "cancast"))
			return 0;
		endif
	endif
	var hand_2 := GetEquipmentByLayer(who, LAYER_HAND2);
	if (hand_2)
		if (!hand_2.IsA(POLCLASS_SPELLBOOK) && !GetObjProperty(hand_2, "cancast"))
			return 0;
		endif
	endif

	return 1;
endfunction

function HandleSpellSkill(who)
	var chardata := getobjProperty(who, "chardata");
	var skill := Cint(AP_GetSkill(who, ARCANA));

	// Detect arcane casters so they scale on Arcana; nature casters (Druida/
	// Shaman) scale on Nature; everyone else uses Theology. Path-rework chars
	// are tagged via chardata.magia (applyClassStarter sets "spellcaster" for
	// Magus/Necromante/Lamina_Arcana, "arcane_archer" for Arqueiro_Arcano).
	// Legacy chars use chardata.wisdom_path = "Mago" — kept as a fallback for
	// any char predating the rework.
	var is_arcane := 0;
	if (chardata != error)
		var magia := chardata.magia;
		if (magia == "spellcaster" || magia == "arcane_archer")
			is_arcane := 1;
		elseif (chardata.wisdom_path == "Mago")
			is_arcane := 1;
		endif
	endif

	// Malandro Arcano scales casting on Arcana from level 1 by class identity
	// (CanCastAsClass); the SENSIBILIDADE_ARCANA ability is a separate reward.
	if (is_arcane || CanCastAsClass(who, "Malandro_Arcano"))
		return skill;
	endif

	// Druida and Shaman cast on Nature (path rework spec 02). All other divine
	// casters (Cleric, Paladino, Caçador de Demônios) remain on Theology.
	if (CanCastAsClass(who, "Druida") || CanCastAsClass(who, "Shaman"))
		return Cint(AP_GetSkill(who, NATURE));
	endif

	return Cint(AP_GetSkill(who, THEOLOGY));
endfunction

function handleSpellResistDiffForDivineCasters(who, school)
   // HandleSpellSkill already branches Druida/Shaman to Nature; Cleric/Paladino/
   // Caçador de Demônios stay on Theology. Routing through it keeps a single
   // source of truth for the divine-caster primary skill.
   var diff := Cint(HandleSpellSkill(who) / 4);

   return diff;
endfunction

// Returns 1 if the caster has an active "mobile cast" window matching the given spell school.
// Ability ExecuteOn sets the "#mobilecast" property with an endtime game-clock value.
// School-gating ensures Evocator only walks while casting Evocation, etc.
function isMobileCastActive(who, school)
	var info := GetObjProperty(who, "#mobilecast");
	if (!info || info is @Exception)
		return 0;
	endif

	if (info.endtime == error || info.endtime <= ReadGameClock())
		EraseObjProperty(who, "#mobilecast");
		return 0;
	endif

	if (info.school == error || info.school != school)
		return 0;
	endif

	return 1;
endfunction

/*
 * SummonSpiritualBlade - Summons a spiritual blade from the player's weapon pool.
 * Used by elemental weapon spells (Lamina Flamejante/Gelo/Eletrica).
 * Returns the created weapon item, or 0 on failure.
 */
function SummonSpiritualBlade(who)
	if (GetCooldown(who, "spiritual_weapon"))
		Message("Voce ja possui uma lamina espiritual invocada!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	var pool := GetObjProperty(who, "spiritualWeaponPool");
	if (!pool || pool.size() < 1)
		Message("Voce nao possui armas absorvidas! Use Absorver Arma primeiro.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	var weaponData;
	if (pool.size() == 1)
		weaponData := pool[1];
	else
		// Multiple weapons - show selection gump
		var result := _ShowSpiritualBladeGump(who, pool);
		var buttonId := result[0];

		if (!buttonId || buttonId == 0)
			Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return 0;
		endif

		// Handle "forget weapon" buttons (300+)
		if (buttonId >= 300)
			var forgetIdx := buttonId - 300;
			if (forgetIdx >= 1 && forgetIdx <= pool.size())
				var forgotten := pool[forgetIdx];
				pool.Erase(forgetIdx);
				SetObjProperty(who, "spiritualWeaponPool", pool);
				Message($"A essencia de {forgotten.name} foi dissipada.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			endif
			return 0;
		endif

		if (buttonId < 200 || buttonId >= 300)
			return 0;
		endif

		var selectedIdx := buttonId - 200;
		if (selectedIdx < 1 || selectedIdx > pool.size())
			return 0;
		endif

		weaponData := pool[selectedIdx];
	endif

	// Summoning ritual
	who.frozen := 1;
	PerformAction(who, HUMAN_ANIM::CAST_DIR);
	Message("*Invocando lamina espiritual*").SendAbove(who);
	PlaySoundEffect(who, 0x5cb);
	PlayStationaryEffectEx(who.x, who.y, who.z, who.realm, 0xB462, 8, 40, COLOR_ELEMENTAL_ENERGY);
	sleep(2);

	// Descongela ANTES de criar: o canInsert da mochila bloqueia insercao
	// de jogador congelado/paralisado ("Voce esta paralizado"), o que fazia
	// o CreateItemInBackpack falhar e a invocacao abortar.
	who.frozen := 0;

	var weapon := CreateItemInBackpack(who, 0x090C, 1);
	if (!weapon)
		Message("Erro ao criar lamina espiritual.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return;
	endif

	weapon.graphic := weaponData.graphic;
	weapon.color := 30000;

	var equipped := EquipItem(who, weapon);
	if (!equipped)
		DestroyItem(weapon);
		Message("Erro ao equipar lamina espiritual.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return;
	endif
	if (weapon.maxhp > 0)
		weapon.hp := weapon.maxhp;
	endif

	PlaySoundEffect(who, 0x245);
	PlayStationaryEffectEx(who.x, who.y, who.z, who.realm, 0xB462, 8, 40, COLOR_ELEMENTAL_ENERGY);

	SetCooldown(who, "spiritual_weapon", 99999);
	Message("Sua lamina espiritual se materializa!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);

	return weapon;
endfunction

function _ShowSpiritualBladeGump(player, pool)
	var gump := GFCreateGump();

	var rowHeight := 26;
	var gumpHeight := 125 + pool.size() * rowHeight;

	GFResizePic(gump, 17, 31, 9390, 400, gumpHeight);
	GFHTMLArea(gump, 114, 69, 250, 20, "<BASEFONT Size=5 Color=#999999 >Lamina Espiritual", 0, 0);
	GFHTMLArea(gump, 116, 71, 250, 20, "<BASEFONT Size=5 Color=#999999 >Lamina Espiritual", 0, 0);
	GFHTMLArea(gump, 115, 70, 250, 20, "<BASEFONT Size=5 Color=#ffffff >Lamina Espiritual", 0, 0);
	GFGumpPic(gump, 63, 91, 1469);
	GFGumpPic(gump, 80, 91, 1469);
	GFTextLine(gump, 63, 100, 0, "Armas Absorvidas");
	GFTextLine(gump, 259, 100, 0, "Invocar");
	GFTextLine(gump, 330, 100, 0, "Esquecer");
	GFGumpPic(gump, 38, 103, 9773, 0);

	var y := 121;
	var i;
	for (i := 1; i <= pool.size(); i++)
		var entry := pool[i];
		GFHTMLArea(gump, 63, y, 192, 24, "<BASEFONT color=#000000>{} (0x{:04X})".format(entry.name, entry.graphic), 1, 0);
		GFAddButton(gump, 274, y + 5, 10006, 10006, GF_CLOSE_BTN, 200 + i);
		GFAddButton(gump, 345, y + 5, 10006, 10006, GF_CLOSE_BTN, 300 + i);
		y += rowHeight;
	endfor

	return GFSendGump(player, gump);
endfunction