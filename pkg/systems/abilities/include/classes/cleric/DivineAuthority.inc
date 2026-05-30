include ":abilities:models/CombatAbility";
include ":timedscripts:timedScripts";
include ":tn:cooldown";

/**
 * Autoridade Divina (Divine Authority) - ABILITY_ID::AUTORIDADE_DIVINA (2803)
 *
 * Active (OnUse):
 * - Creates consecrated ground in 3-tile radius around caster
 * - Scaling: theology_bonus = Theology / 5 (= duration)
 *
 * Religion effects:
 *   VENERATIO: Silences all non-caster in area
 *   KAI: Allies get "balance_restored" cooldown (crit immunity)
 *   BOKK_BHARAZ: Allies get "mountains_strength" cooldown (damage reduction)
 *   HALLEUAD: Enemies get "blinded" timer
 *   ELUITHORTH: All nearby get "true_darkness" + "darkness" cooldowns
 */
class DivineAuthority(CombatAbility)

	function DivineAuthority(this)
		Super(ABILITY_ID::AUTORIDADE_DIVINA);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var religiao := mobile.GetReligion();
		var theology_bonus := CInt(mobile.GetSkill(THEOLOGY) / 5);
		var duration := theology_bonus;

		Message("*Cria terreno consagrado*").SendEmote(who);
		PlayObjectCenteredEffect(who, 0x376A, 1, 32);
		PlaySoundEffect(who, 0x1F2);

		var nearby_mobiles := mobile.GetNearbyMobiles(3);
		var allies := array{};
		var enemies := array{};

		foreach mob in (nearby_mobiles)
			if (mob == who)
				continue;
			endif

			if (mobile.IsAllyOf(MobileWrapper(mob)))
				allies.append(mob);
			else
				enemies.append(mob);
			endif
		endforeach

		case (religiao)
			VENERATIO:
				PlayStationaryEffect(who.x, who.y, who.z, 0x37C4, 10, 20);
				foreach mob in (nearby_mobiles)
					if (mob != who)
						SetCooldown(mob, "silence", duration);
						Message("*silenciado*").SendAbove(mob);
					endif
				endforeach
				Message("Voce cria um campo de silencio divino!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			KAI:
				PlayStationaryEffect(who.x, who.y, who.z, 0x373A, 10, 20);
				foreach ally in (allies)
					SetCooldown(ally, "balance_restored", duration);
					Message("*equilibrio restaurado*").SendAbove(ally);
					Message("Voce esta protegido contra ataques criticos!", Color(SYSTEM_COLOR::INFO)).SendSys(ally);
				endforeach
				Message("Voce restaura o equilibrio divino aos aliados!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			BOKK_BHARAZ:
				PlayStationaryEffect(who.x, who.y, who.z, 0x6d67, 10, 20);
				foreach ally in (allies)
					SetCooldown(ally, "mountains_strength", duration);
					Message("*forca da montanha*").SendAbove(ally);
					Message("Voce sente a forca inabalavel da montanha!", Color(SYSTEM_COLOR::INFO)).SendSys(ally);
				endforeach
				Message("Voce concede a forca da montanha aos aliados!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			HALLEUAD:
				foreach enemy in (enemies)
					TS_StartTimer(enemy, "blinded", duration, 0, who);
					Message("Voce foi cegado pelo eclipse divino!", Color(SYSTEM_COLOR::FAIL)).SendSys(enemy);
				endforeach
				Message("Voce invoca um eclipse cegante!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			ELUITHORTH:
				PlayStationaryEffect(who.x, who.y, who.z, 0x37C4, 10, 20);
				foreach mob in (nearby_mobiles)
					SetCooldown(mob, "true_darkness", duration);
					SetCooldown(mob, "darkness", duration);
					Message("*envolvido em trevas*").SendAbove(mob);
					Message("Voce e envolvido pelas trevas sagradas!", Color(SYSTEM_COLOR::INFO)).SendSys(mob);
				endforeach
				Message("Voce concede o dominio das sombras aos aliados!", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		endcase

		return true;
	endfunction

endclass
