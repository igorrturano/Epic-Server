use uo;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Elemental Affinity (Ainalaith) - ABILITY_ID::ELEMENTAL_AFFINITY (2401)
 *
 * Passive (OnPassiveBeforeDamage + OnPassiveOnMiss, attacker-side):
 * - Tracks consecutive hits with the same arrow element via #shooting_effects
 * - Every 3rd consecutive hit triggers a status effect on the defender:
 *   - fire_arrow → "queimar" (burn) 10s
 *   - cold_arrow → "lentidao" (slow) 10s
 *   - energy_arrow → bonus damage (ARCANA/20)
 * - On miss, all counters reset to 0
 */
class ElementalAffinity(CombatAbility)

	function ElementalAffinity(this)
		Super(ABILITY_ID::ELEMENTAL_AFFINITY);
	endfunction

	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();
		var weapon := attacker.GetEquipBySlot(WEAPON);

		if (!weapon)
			return false;
		endif

		var weaponRef := weapon.GetObjRef();

		// Determine which arrow type is active on the weapon
		var activeArrow := "";
		if (GetObjProperty(weaponRef, "fire_arrow"))
			activeArrow := "fire_arrow";
		elseif (GetObjProperty(weaponRef, "cold_arrow"))
			activeArrow := "cold_arrow";
		elseif (GetObjProperty(weaponRef, "energy_arrow"))
			activeArrow := "energy_arrow";
		endif

		if (activeArrow == "")
			return false;
		endif

		var arrow_effects := GetObjProperty(who, "#shooting_effects");
		if (!arrow_effects)
			arrow_effects := struct{};
			arrow_effects.+fire_arrow := 0;
			arrow_effects.+cold_arrow := 0;
			arrow_effects.+energy_arrow := 0;
		endif

		// Reset other counters (consecutive same-element tracking)
		var arrow_types := array{"fire_arrow", "cold_arrow", "energy_arrow"};
		foreach arrow_type in arrow_types
			if (arrow_type != activeArrow)
				arrow_effects[arrow_type] := 0;
			endif
		endforeach

		// Increment active arrow counter
		arrow_effects[activeArrow] := arrow_effects[activeArrow] + 1;

		var defender := attackInfo.GetDefender().GetObjRef();

		// Check if 3rd consecutive hit
		if (arrow_effects[activeArrow] >= 3)
			case (activeArrow)
				"fire_arrow":
					TS_StartTimer(defender, "queimar", 10);
				"cold_arrow":
					TS_StartTimer(defender, "lentidao", 10);
				"energy_arrow":
					var bonus := CInt(attacker.GetSkill(ARCANA) / 20);
					attackInfo.GetDamage().IncreaseEnergyBaseDamage(bonus);
					DebugMsg(
						who,
						DEBUG_HAB,
						Message($"[ELEMENTAL_AFFINITY] Bonus dano energia: +{bonus}", Color(SYSTEM_COLOR::SUCCESS))
					);
			endcase

			arrow_effects[activeArrow] := 0;
			SetObjProperty(who, "#elemental_affinity_triggered", activeArrow);

			DebugMsg(
				who,
				DEBUG_HAB,
				Message($"[ELEMENTAL_AFFINITY] 3o disparo consecutivo! Efeito: {activeArrow}", Color(SYSTEM_COLOR::SUCCESS))
			);
		endif

		SetObjProperty(who, "#shooting_effects", arrow_effects);
		return true;
	endfunction

	function OnPassiveOnMiss(this, attackInfo)
		var who := attackInfo.GetAttacker().GetObjRef();
		var arrow_effects := GetObjProperty(who, "#shooting_effects");

		if (!arrow_effects)
			return false;
		endif

		if (arrow_effects["fire_arrow"] > 0 || arrow_effects["cold_arrow"] > 0 || arrow_effects["energy_arrow"] > 0)
			arrow_effects["fire_arrow"] := 0;
			arrow_effects["cold_arrow"] := 0;
			arrow_effects["energy_arrow"] := 0;
			SetObjProperty(who, "#shooting_effects", arrow_effects);

			DebugMsg(
				who,
				DEBUG_HAB,
				Message("[ELEMENTAL_AFFINITY] Errou! Contadores resetados.", Color(SYSTEM_COLOR::FAIL))
			);

			return true;
		endif

		return false;
	endfunction

endclass
