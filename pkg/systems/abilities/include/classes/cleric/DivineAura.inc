include ":abilities:models/CombatAbility";
include ":timedscripts:timedScripts";
include ":attributes:attributes";
include ":spells:summon";
include "include/facings";
include "include/epicCombatUtils";

/**
 * Aura Divina (Divine Aura) - ABILITY_ID::AURA_DIVINA (2802)
 *
 * Active (OnUse):
 * - Religion-specific targeted buff/summon ability
 * - Scaling: theology_bonus = Theology / 5
 *
 * Religion effects:
 *   ELUITHORTH: Summons dark spiders (start_script prolesombria)
 *   BOKK_BHARAZ: Anvil jump (start_script saltobigorna)
 *   HALLEUAD: Speed buff on target
 *   VENERATIO: Stat buffs (STR/DEX/INT, capped 50%) on target
 *   KAI: Hit accuracy bonus on target
 */
class DivineAura(CombatAbility)

	function DivineAura(this)
		Super(ABILITY_ID::AURA_DIVINA);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		var religiao := mobile.GetReligion();
		var theology_bonus := CInt(mobile.GetSkill(THEOLOGY) / 5);

		PerformAction(who, HUMAN_ANIM::CAST_DIR);

		case (religiao)
			ELUITHORTH:
				if (mobile.IsNPC())
					return false;
				endif
				var targ := Target(who);
				if (!targ)
					Message("Voce deve selecionar um local.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
				var ps_lvl := CInt(theology_bonus * 5);
				var ps_template := ":epicai:GiantSpider";
				if (ps_lvl >= 90)
					ps_template := ":epicai:BlackHugeSpider";
				elseif (ps_lvl >= 70)
					ps_template := ":epicai:HugeRedSpider";
				elseif (ps_lvl >= 50)
					ps_template := ":epicai:DreadGiantSpider";
				endif
				who.frozen := 1;
				var ps_i := 0;
				while (ps_i < 4)
					PerformAction(who, HUMAN_ANIM::CAST_DIR);
					sleep(1);
					ps_i++;
				endwhile
				who.frozen := 0;
				PlaySoundEffect(targ, SFX_SPELL_SUMMON_ELEMENTAL);
				PlaySoundEffect(who, 0x686);
				var ps_dur := 120 + CInt(180 * ps_lvl / 100);
				var ps_mount := summon(ps_template, targ, who);
				if (ps_mount)
					TS_StartTimer(ps_mount, "summon", ps_dur);
					PlayObjectCenteredEffect(ps_mount, 0x37C4, 10, 20);
				endif
				var ps_isDark := (GetGlobalProperty("dia") != "dia") || (who.realm != "britannia");
				if (ps_isDark)
					var ps_mount2 := summon(ps_template, targ, who);
					if (ps_mount2)
						TS_StartTimer(ps_mount2, "summon", ps_dur);
					endif
				endif
				Message("Sua prole sombria esta pronta para servi-la", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			BOKK_BHARAZ:
				var sb_dano := CInt(theology_bonus * 5 / 2);
				var sb_dur := theology_bonus;
				if (sb_dur < 1)
					sb_dur := 1;
				endif
				PerformAction(who, 12);
				Message("*Arremessa o martelo*").SendAbove(who);
				var sb_wz := GetStandingHeight(who.x, who.y, who.z + 15, who.realm);
				var sb_moved := 0;
				if (sb_wz.z <= who.z + 1)
					sb_moved := 1;
					MoveObjectToLocation(who, who.x, who.y, who.z + 15, who.realm, MOVEOBJECT_FORCELOCATION);
				endif
				sleepms(100);
				_SaltoBigornaJump(who, 5, 500);
				sleepms(200);
				if (sb_moved)
					MoveObjectToLocation(who, who.x, who.y, who.z - 15, who.realm);
				endif
				sleepms(500);
				PlayStationaryEffect(who.x + 2, who.y + 2, who.z, 0x6C9B, 0, 10, 0, who.realm);
				PlaySoundEffect(who, 0x221);
				sleepms(500);
				var sb_nearby := mobile.GetNearbyMobiles(2);
				var sb_party := who.party ? who.party.members : array{};
				var sb_pets := array{};
				foreach sb_member in sb_party
					if (sb_member != who)
						var sb_mobj := SystemFindObjectBySerial(sb_member.serial);
						if (sb_mobj)
							var sb_mpets := ListAllPetsAndSummonedPetsNear(sb_mobj, 8);
							foreach sb_pet in sb_mpets
								sb_pets.append(sb_pet);
							endforeach
						endif
					endif
				endforeach
				foreach sb_char in sb_nearby
					if (sb_char == who || sb_char in sb_party || sb_char in sb_pets)
						continue;
					endif
					if (GetObjProperty(sb_char, "owner") == who.serial)
						continue;
					endif
					PlayObjectCenteredEffect(sb_char, 0x6d67, 0, 10);
					TS_StartTimer(sb_char, "tombar", sb_dur);
					TS_StartTimer(sb_char, "lentidao", sb_dur * 2);
					TS_StartTimer(sb_char, "paralysis", sb_dur);
					ApplyRawDamage(sb_char, sb_dano);
				endforeach

			HALLEUAD:
				var targ;
				if (mobile.IsNPC())
					targ := who;
				else
					targ := Target(who);
					if (!targ)
						Message("Voce deve selecionar um alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
						return false;
					endif
				endif
				PlayObjectCenteredEffectEx(targ, FX_SPEED_BOOTS, 7, 32, 0, 1);
				PlaySoundEffect(targ, 0x20F);
				TS_StartTimer(targ, "velocidade", theology_bonus);

			VENERATIO:
				var targ;
				if (mobile.IsNPC())
					targ := who;
				else
					targ := Target(who);
					if (!targ)
						Message("Voce deve selecionar um alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
						return false;
					endif
				endif

				PlayObjectCenteredEffectEx(targ, FX_MORALE_UP, 7, 32);
				PlaySoundEffect(targ, 0x1F5);

				var buff_percent := CInt(theology_bonus * 2);
				if (buff_percent > 50)
					buff_percent := 50;
				endif

				var buff_duration := theology_bonus * 2;

				// STR buff
				var str_val := AP_GetStat(targ, STRENGTH);
				var str_points := CInt((str_val * buff_percent) / 100);
				TS_StartTimer(targ, "strength", buff_duration, str_points, who, 1);

				// DEX buff
				var dex_val := AP_GetStat(targ, DEXTERITY);
				var dex_points := CInt((dex_val * buff_percent) / 100);
				TS_StartTimer(targ, "agility", buff_duration, dex_points, who, 1);

				// INT buff
				var int_val := AP_GetStat(targ, INTELLIGENCE);
				var int_points := CInt((int_val * buff_percent) / 100);
				TS_StartTimer(targ, "intelligence", buff_duration, int_points, who, 1);

				Message("Voce se sente fortalecido pela aura divina!", Color(SYSTEM_COLOR::INFO)).SendSys(targ);
				Message("Seu alvo foi abencoado com forca divina!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			KAI:
				var targ;
				if (mobile.IsNPC())
					targ := who;
				else
					targ := Target(who);
					if (!targ)
						Message("Voce deve selecionar um alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
						return false;
					endif
				endif

				PlayObjectCenteredEffect(targ, 0x373A, 10, 20);
				PlaySoundEffect(targ, 0x1F7);

				SetObjProperty(targ, "#GuaranteedHit", 1);

				Message("Sua precisao foi aprimorada pela aura divina!", Color(SYSTEM_COLOR::INFO)).SendSys(targ);
				Message("Voce concede precisao divina ao alvo!", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		endcase

		return true;
	endfunction

endclass

function _SaltoBigornaJump(who, dist, delay)
	var mod := GetModByFacing(who.facing);
	var x := mod[1];
	var y := mod[2];

	for i := 1 to dist
		var finalx := who.x + x;
		var finaly := who.y + y;
		if (!CheckLosAt(who, finalx, finaly, who.z))
			return 0;
		endif
		var world_z := GetStandingHeight(who.x, who.y, who.z, who.realm);
		if (world_z.z > who.z)
			return 0;
		endif
		var err := MoveObjectToLocation(who, finalx, finaly, who.z, who.realm, MOVEOBJECT_NORMAL);
		if (err == error)
			return 0;
		endif
		sleepms(delay);
	endfor
	return 1;
endfunction
