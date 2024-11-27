include "include/epicConstants";
include ":combat:damage";


function playAltarAnimations(who, item)
   var altar := altarByObjtype(who, item);
   var newGraphic := Cint(graphicByAltar(altar));
   item.graphic := newGraphic;
   sleep(2);
   item.graphic := item.objtype;
   var effect := effectByAltar(altar);
   PlaySoundEffect(who, SFX_SPELL_HEAL);
   PlayStationaryEffectEx(who.x, who.y, who.z,who.realm, effect, 255, 80, 0, 0);
endfunction

function effectByAltar(altar)
   var effect := 0;
   case (altar)
	  ELUITHORTH:
		 effect := 0x54F7;
	  LORE:
		 effect := 0xB462;
	  VENERATIO:
		 effect := 0x54B3;
	  BOKK_BHARAZ:
		 effect := 0x6DF1;
	  KAI:
		 effect := 0x6d57;
	  USHKRAR:
		 effect := 0X6C91;
	  CETICOS:
		 effect := 0;
	  HALLEUAD:
		 effect := 0x6CEB; //
   endcase

   return effect;
endfunction

function altarByObjtype(who, altar)
   var objtype := altar.objtype;
   var religion := "";
   case (objtype)
	  0xA4A8: //0xFA67
		 religion := ELUITHORTH;
	  0xA06F: //0xFA68
		 religion := LORE;
	  0xA065: // 0xFA63
		 religion := VENERATIO;
	  0x9E81: // 0xFA64
		 religion := BOKK_BHARAZ;
	  0xA252: //0xFA65
		 religion := KAI;
	  0xA06A: // 0xFA66
		 religion := USHKRAR;
	  0xA261:
		 religion := CETICOS;
	  0xA060:
		 religion := HALLEUAD;
   endcase

   return religion;
endfunction

function graphicByAltar(altar)
	print(altar);
   var graphic := 0;
   case (altar)
	  ELUITHORTH:
		 graphic := 0xA4A9; //0xFA20;
	  LORE:
		 graphic := 0xA070;//0xFA25;
	  VENERATIO:
		 graphic := 0xA066;//0xD1EA;
	  BOKK_BHARAZ:
		 graphic := 0x9DF2; // 0xD1EF;
	  KAI:
		 graphic := 0xA253;//0xD1F4;
	  USHKRAR:
		 graphic := 0xA06B;//0xD1F9;
	  HALLEUAD:
		 graphic := 0xA061;
	  CETICOS:
		 graphic := 0xA262;
	  default:
		 graphic := 0;
   endcase
   return graphic;
endfunction

function listAllMobilesNearbyButPetsAndParty(who, range, targ := 0)
   if (targ == 0)
	  targ := who;
   endif

   var mobiles := array{};
   var allies := who.party;
	var alreadydamaged := array{};

   var party_members := who.party.members;
	  if (who.party)
	  foreach member in party_members
		 if (member != who)
			   var memberObj := SystemFindObjectBySerial(member.serial);
			   var allPetsAndSummonedPets := ListAllPetsAndSummonedPetsNear(memberObj, 8 * 4);
			   foreach pet in allPetsAndSummonedPets
				  mobiles.append(pet);
			   endforeach
		 endif
	  endforeach
   endif

   var nearby := ListMobilesNearLocation( targ.x ,targ.y, targ.z, range, targ.realm);
   foreach char in nearby
	  var char_owner := GetObjProperty(char, "owner");
	  if (!(char in (allies.members) || char == who))
		 if (!(char.serial in alreadydamaged) && !(char in mobiles) && !(char in who.party.members) && !(char_owner == who.serial))
			alreadydamaged.Append(char.serial);
			mobiles.append(char);
		 endif
	  endif
   endforeach

   return mobiles;
endfunction

function distanceAllowedForSpells(who, mlore, school)
   var dist_allowed := 5 + mlore;

   if (TemHabilidade(who, "Evocator") && school == EVOCATIUM)
	  dist_allowed := dist_allowed + (mlore * 2);
   elseif (TemHabilidade(who, "Valdruk") && school == VELKYN)
	  dist_allowed := dist_allowed + (mlore * 2);
   elseif (TemHabilidade(who, "Tirothan") && school == EVOCATIUM)
	  dist_allowed := dist_allowed + (mlore * 2);
   endif

   return dist_allowed;
endfunction

function bonusSpellDamageForSpecialists(who, dmg, school)
   var bonus := 1.2;

   if (TemHabilidade(who, "Evocator") && school == EVOCATIUM)
	  dmg := Cint(dmg * bonus);
   elseif (TemHabilidade(who, "Valdruk") && school == VELKYN)
	  dmg := Cint(dmg * bonus);
   endif

   return dmg;
endfunction

function epicSpellDamageRoll(who, spell_circle)
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
   if (TemHabilidade(who, "Evocator") && school == EVOCATIUM)
	  diff := RESIST_MEDIUM;
   elseif (TemHabilidade(who, "Valdruk") && school == VELKYN)
	  diff := RESIST_MEDIUM;
   elseif (TemHabilidade(who, "Cetro da Morte") && school == KAIJIN)
		diff := RESIST_MEDIUM;
   endif
 
   return diff;
endfunction

function bonusSpellDurationForSpecialists(who, duration, school)
   var bonus := 1.2;

   if (TemHabilidade(who, "Evocator") && school == EVOCATIUM)
	  duration := Cint(duration * bonus);
   elseif (TemHabilidade(who, "Valdruk") && school == VELKYN)
	  duration := Cint(duration * bonus);
   elseif(Temhabilidade(who, "Treinamento Druídico") && school == ARADALORE)
	  duration := Cint(duration * bonus);
   endif

   return duration;
endfunction

function checkForElementalWeapons(who, targ)

	if ( GetCooldown(targ, "blessweapon") > 0 )
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
   var mlore := Cint(AP_GetSkill(who, MAGICLORE) / 10); 

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
function ApplyElementalWeaponProperties( who, elemento )

  var weapon := who.weapon;
  if ( weapon == error )
	return 0;
  endif

  var elementalPropAtual := GetObjProperty( weapon, "#elementalweapon" );
  var jaEncantada := elementalPropAtual != error;

  var dmgdice := DefineElementalWeaponDice( who );
  var cargas := Floor( AP_GetSkill( who, MAGICLORE ) / 4 );
  cargas := cargas <= 0 ? 1 : cargas; // no minimo uma carga.

  var elementalWeapon := struct;
  elementalWeapon.+type := elemento;
  elementalWeapon.+serial := who.serial;
  elementalWeapon.+bonus := dmgdice;
  elementalWeapon.+cargas := cargas;
  elementalWeapon.+cargaMaxima := cargas;
  elementalWeapon.+endtime := ReadGameClock() + 1200;
  if ( !jaEncantada )
	elementalWeapon.+originalColor := weapon.color;
  else
	elementalWeapon.+originalColor := elementalPropAtual.originalColor;
	var decayScript := GetProcess(elementalPropAtual.decayScriptPid); 
	decayScript.SendEvent({weapon,elementalWeapon.endtime});
	
  endif
  elementalWeapon.decayScriptPid := start_script( ":spells:elementalweapon",{weapon,elementalWeapon.endtime }).pid;
 

  if ( elemento == DMG_FIRE )
	weapon.color := FOGO_LIGHT;
  elseif ( elemento == DMG_COLD )
	weapon.color := GELO_LIGHT;
  elseif ( elemento == DMG_ENERGY )
	weapon.color := RAIO_LIGHT;
  endif

  IncRevision( weapon );
  SetObjProperty( weapon, "#elementalweapon", elementalWeapon );

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
function DefineElementalWeaponDice( who )

  var faixaMLore := AP_GetSkill( who, MAGICLORE );
  var dmgdice := "0";

  if ( faixaMLore < 40 )
    dmgdice := "1d2";
    elseif ( faixaMLore >= 40 && faixaMLore < 70 )
    dmgdice := "1d4";
    elseif ( faixaMLore >= 70 && faixaMLore < 90 )
    dmgdice := "1d6";
    elseif ( faixaMLore >= 90 )
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
function ConsumeArcaneCharges( who, quantidade )

  var elementalWeapon := GetObjProperty( who.weapon, "#elementalweapon" );
  var retorno := ConsumeItemArcaneCharges( who.weapon, quantidade );

  if ( elementalWeapon != error && retorno.isSucesso )
	if ( elementalWeapon.serial != who.serial && !TemHabilidade( who, "Vínculo Elemental" ))
	  ApplyDamageHabElementalWeapon( 0, who, elementalWeapon.bonus, elementalWeapon.type,
								  retorno.divisor );
		SendSysMessageEx(who,"O controle das energias arcanas da arma se torna caótico e acaba ferindo você.", SSM_INFO);
	endif
  endif
  return retorno;
endfunction

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
function ConsumeItemArcaneCharges( item, quantidade, consumeAll := false )

  var elementalWeapon := GetObjProperty( item, "#elementalweapon" );
  var retorno := struct;
  retorno.+divisor := 1;
  retorno.+isSucesso := true;

  if ( elementalWeapon != error )
	var typeColor;
	if ( elementalWeapon.type == DMG_FIRE )
	  typeColor := FOGO_LIGHT;
	elseif ( elementalWeapon.type == DMG_COLD )
	  typeColor := GELO_LIGHT;
	elseif ( elementalWeapon.type == DMG_ENERGY )
	  typeColor := RAIO_LIGHT;
	endif

	var cargasAtualizada := 0;
	if ( !consumeAll )
	  cargasAtualizada := elementalWeapon.cargas - quantidade;
	endif

	if ( cargasAtualizada <= 0 ) // remove as propiedades do encantamento
	  retorno.divisor := cargasAtualizada == 0 ? 1 : cargasAtualizada * -1;
	  EraseObjProperty(item,"#elementalweapon");
	  item.color := elementalWeapon.originalColor;
	  if ( item.character_owner != error )
		PlaySoundEffect(item.character_owner, SFX_SPELL_SUMMON_CREATURE);
		SendSysMessageEx( item.character_owner, "Sua arma nao esta mais encantada.",SSM_INFO );
	  endif
	else

	  elementalWeapon.cargas := cargasAtualizada; // Atualiza as cargas

	  var faixaCor := elementalWeapon.cargas * 100 / elementalWeapon.cargaMaxima; // decide a faixa de cor
	  if ( faixaCor <= 25 ) // Aplica a nova cor.
		item.color := typeColor - 4;
	  elseif ( faixaCor > 25 && faixaCor <= 50 )
		item.color := typeColor - 3;
	  elseif ( faixaCor > 50 && faixaCor <= 75 )
		item.color := typeColor - 2;
	  elseif ( faixaCor > 75 && faixaCor <= 90 )
		item.color := typeColor - 1;
	  endif
	  SetObjProperty( item, "#elementalweapon", elementalWeapon );
	endif
	IncRevision( item );
	return retorno;
  else
	retorno.isSucesso := false;
	return retorno;
  endif
endfunction

function LogOffElementalWeapon(who)
	foreach item in (EnumerateItemsInContainer(who))
		var elementalWeapon := GetObjProperty( item, "#elementalweapon" );
		if (elementalWeapon != error)
			ConsumeItemArcaneCharges(item,0,true);
		endif
	endforeach
endfunction

function ApplyChargesCalculation( weapon, dmg, chargesAmount )
  var cargasRetorno := ConsumeItemArcaneCharges( weapon, chargesAmount );
  var damage := Floor( dmg / cargasRetorno.divisor );
  return damage;
endfunction