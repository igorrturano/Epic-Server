include ":abilities:models/CombatAbility";
include ":timedscripts:timedScripts";
include ":tn:cooldown";
include ":brainai:npcUtil";
include ":abilities:helpers/clericHelpers";

/**
 * Apoteose (Apotheosis) - ABILITY_ID::APOTEOSE (2804)
 *
 * Active (OnUse):
 * - Ultimate divine manifestation ability
 * - Sets "divine_manifestation" cooldown (divine spells free of mana/delay)
 * - Scaling: duration = Theology / 5
 *
 * Religion effects:
 *   VENERATIO: Marks nearby enemies with "apodrecer" (double damage taken)
 *   BOKK_BHARAZ: Stone skin (start_script pelepedra)
 *   HALLEUAD: Heals allies + destroys undead/darkness creatures
 *   ELUITHORTH: Torments all enemies (start_script tormentoidrith)
 *   KAI: Guarantees critical hits for all nearby allies
 */
class Apotheosis(CombatAbility)

	function Apotheosis(this)
		Super(ABILITY_ID::APOTEOSE);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var religiao := mobile.GetReligion();
		var theology_skill := mobile.GetSkill(THEOLOGY);
		var duration := CInt(theology_skill / 5);

		Message("*Ascende a manifestacao divina*").SendEmote(who);
		PlayObjectCenteredEffect(who, 0x37C4, 1, 32);
		PlaySoundEffect(who, 0x1F2);

		SetCooldown(who, "divine_manifestation", duration);
		Message("Voce ascendeu a manifestacao divina! Suas magias divinas nao consomem mana nem possuem delay!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

		case (religiao)
			VENERATIO:
				var nearby_enemies := GetNearbyEnemies(who, 8);
				foreach enemy in (nearby_enemies)
					TS_StartTimer(enemy, "apodrecer", duration);
					Message("*marcado pela gloria eterna*").SendAbove(enemy);
					PlayObjectCenteredEffect(enemy, 0x37C4, 10, 20);
					Message("Voce foi marcado pela gloria divina eterna!", Color(SYSTEM_COLOR::FAIL)).SendSys(enemy);
				endforeach
				Message("Voce manifesta a Gloria Eterna de Veneratio!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			BOKK_BHARAZ:
				start_script(":charactercreation:habilidades/pelepedra", who);
				Message("Voce manifesta a Fortaleza Inabalavel de Bokk Bharaz!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			HALLEUAD:
				var nearby_mobiles := ListMobilesNearLocation(who.x, who.y, who.z, 8, who.realm);
				PlayStationaryEffect(who.x, who.y, who.z + 10, 0x37C4, 10, 30);

				foreach mob in (nearby_mobiles)
					if (mob == who)
						continue;
					endif

					var is_ally := IsPlayerAlly(who, mob);

					if (is_ally)
						var heal_amount := CInt(theology_skill * 1.4);
						applyEpicHealing(mob, heal_amount);
						PlayObjectCenteredEffect(mob, 0x375A, 0, 16);
						Message("Voce e curado pela luz divina!", Color(SYSTEM_COLOR::INFO)).SendSys(mob);
					else
						var npc_elem := NPC_GetNPCConfig(mob.npctemplate);
						var isDarknessCreature := GetObjProperty(mob, "Mobnoturno") || Lower(npc_elem.Category) == "undead";

						// Darkness check: night time or in dungeon
						var isInDarkness := (GetGlobalProperty("dia") != "dia") || (mob.realm != "britannia");

						if (isDarknessCreature && !isInDarkness)
							ApplyRawDamage(mob, CInt(AP_GetVitalMaximumValue(mob, "Hits")));
							Message("*destruido pela luz divina*").SendAbove(mob);
							PlayObjectCenteredEffect(mob, 0x36BD, 10, 20);
						endif
					endif
					sleepms(50);
				endforeach
				Message("Voce manifesta o Sol Divino de Halleuad!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			ELUITHORTH:
				start_script(":charactercreation:habilidades/tormentoidrith", who);
				Message("Voce manifesta o Tormento de Idrith!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			KAI:
				var nearby_allies := GetNearbyAllies(who, 8);
				foreach ally in (nearby_allies)
					SetObjProperty(ally, "#GuaranteedHit", 1);
					SetObjProperty(ally, "#CriticalChance", 100);

					Message("*destino manifesto*").SendAbove(ally);
					PlayObjectCenteredEffect(ally, 0x373A, 10, 20);
					Message("Seu destino foi manifestado! Seu proximo ataque sera um acerto critico!", Color(SYSTEM_COLOR::INFO)).SendSys(ally);
				endforeach
				Message("Voce manifesta o Destino de Kai!", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		endcase

		return true;
	endfunction

endclass
