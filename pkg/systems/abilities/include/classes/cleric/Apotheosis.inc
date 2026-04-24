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
				var pp_duration := CInt(theology_skill / 10);
				if (pp_duration < 3)
					pp_duration := 3;
				endif
				PlaySoundEffect(who, 0x216);
				PlayObjectCenteredEffect(who, 0x6C79, 0, 25);
				PlaySoundEffect(who, 0x65b);
				GrantPrivilege(who, "invul");
				who.enable("invul");
				var pp_i := 0;
				while (pp_i < CInt(pp_duration / 3))
					PlayObjectCenteredEffect(who, 0x6CE8, 10, 60);
					sleep(3);
					pp_i++;
				endwhile
				RevokePrivilege(who, "invul");
				who.disable("invul");
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
						var isDarknessCreature := GetObjProperty(mob, "Mobnoturno") || Lower(npc_elem.Environment) == "undead";

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
				var tor_range := CInt(theology_skill / 5);
				var tor_dano := CInt(theology_skill / 2);
				var tor_dur := CInt(theology_skill / 8);
				if (tor_dur < 1)
					tor_dur := 1;
				endif
				var isInDarkness := (GetGlobalProperty("dia") != "dia") || (who.realm != "britannia");
				PerformAction(who, HUMAN_ANIM::CAST_AREA);
				PlaySoundEffect(who, 0x684);
				var tor_enemies := GetNearbyEnemies(who, tor_range);
				foreach mob in tor_enemies
					TS_StartTimer(mob, "paralysis", tor_dur);
					PlayObjectCenteredEffect(mob, 0x54F7, 10, 30);
					Message("*atormentado*").SendAbove(mob);
					ApplyRawDamage(mob, tor_dano);
					if (isInDarkness)
						TS_StartTimer(mob, "blinded", tor_dur + CInt(theology_skill / 5), 0, who);
						Message("*cegueira*").SendAbove(mob);
					endif
					sleepms(20);
				endforeach
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
