use uo;
use os;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include ":destroyable:include/destroyItem";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include "include/shapes";
include "include/sounds";
include "include/epicConstants";

/**
 * Elemental Storm (Tempestade Elemental) - ABILITY_ID::TEMPESTADE_ELEMENTAL (2404)
 *
 * Active (OnUse): AoE elemental attack based on weapon's elemental enchant.
 * - Fire → fire trail + explosion (Rastrodefogo)
 * - Cold → AoE freeze nearby enemies
 * - Energy → AoE lightning storm
 * Party members excluded from AoE.
 */
class ElementalStorm(CombatAbility)

	function ElementalStorm(this)
		Super(ABILITY_ID::TEMPESTADE_ELEMENTAL);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		var element := GetObjProperty(who.weapon, "#elementalweapon");

		if (element == error)
			Message("Voce precisa estar equipado com uma arma encantada.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var element := GetObjProperty(who.weapon, "#elementalweapon");
		var elementType := element.type;
		var duracao := mobile.GetSkill(ARCANA);

		var targ := Target(who);

		case (elementType)
			"Fire":
				Message("Voce conjura uma explosao na direcao apontada.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
				_FireTrail(who, targ, duracao);

			"Cold":
				Message("Voce invoca uma forte nevasca ao seu redor.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
				foreach defender in (ListMobilesNearLocation(who.x, who.y, who.z, 10, who.realm))
					if (!(defender in who.party.members))
						if (defender.serial != who.serial)
							PlayObjectCenteredEffect(defender, FX_BLUE_SPARKLE_GROUND, 3, 4);
							PlaySoundEffect(defender, SFX_GLASS_BREAK_3);
							DamageFLS(defender, RandomIntMinMax(20, 35), DMG_ENERGY, who);
							TS_StartTimer(defender, "congelamento", 3);
						endif
					endif
				endforeach

			"Energy":
				Message("Voce invoca uma tempestade.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
				foreach defender in (ListMobilesNearLocation(who.x, who.y, who.z, 8, who.realm))
					if (!(defender in who.party.members))
						if (defender.serial != who.serial)
							PlaySoundEffect(defender, 0x2A);
							PlayLightningBoltEffect(defender);
							DamageFLS(defender, RandomIntMinMax(25, CInt(duracao)), DMG_ENERGY, who);
						endif
					endif
				endforeach
		endcase

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[TEMPESTADE_ELEMENTAL] Tipo: {elementType}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass

function _FireTrail(npc, opponent, duracao)
	if (!CheckLineOfSight(npc, opponent))
		Message("Voce nao ve o alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(npc);
		return 0;
	endif

	PerformAction(npc, 12);
	PlaySoundEffect(npc, 0x208);
	sleep(2);
	PlayMovingEffect(npc, opponent, 0x36D4, 10, 10, 1);

	var coord_list := GetCoordsInLine(npc.x, npc.y, opponent.x, opponent.y);
	foreach coord in coord_list
		if (_coord_iter < 4)
			continue;
		endif

		var field := CreateItemAtLocation(coord.x, coord.y, npc.z, "FireField_NS", 1);
		field.SetDuration(0);
		sleepms(105);
	endforeach

	foreach coord in GetCoordsInSquare(opponent.x, opponent.y, 3)
		if (RandomInt(2) != 1)
			continue;
		endif
		case (RandomInt(3))
			default: PlayStationaryEffect(coord.x, coord.y, npc.z, 0x36CB, 10, 10, 0); break;
			1: PlayStationaryEffect(coord.x, coord.y, npc.z, 0x36BD, 10, 10, 0); break;
			2: PlayStationaryEffect(coord.x, coord.y, npc.z, 0x36B0, 10, 10, 0); break;
		endcase
		var field;
		if (RandomInt(2))
			field := CreateItemAtLocation(coord.x, coord.y, opponent.z, "FireFieldNS", 1);
		else
			field := CreateItemAtLocation(coord.x, coord.y, opponent.z, "FireFieldEW", 1);
		endif
		field.SetOwner(npc);
		field.SetDuration(20);
		sleepms(5);
	endforeach

	PlaySoundEffect(opponent, 0x208);

	var chars := ListMobilesNearLocation(opponent.x, opponent.y, opponent.z, 4, npc.realm);
	DanificaObjetosProximos(npc, opponent.x, opponent.y, opponent.z, opponent.realm, struct{ dano := RandomIntMinMax(
																					 	40,
																					 	CInt(duracao)),
																					 	desc := "Tempestade Elemental." }, 2, 0x36BD);
	foreach mobile in chars
		DamageFLS(mobile, RandomIntMinMax(40, CInt(duracao)), DMG_FIRE, npc);
	endforeach

	return 1;
endfunction
