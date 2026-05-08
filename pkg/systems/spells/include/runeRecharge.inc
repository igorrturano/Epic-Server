use uo;
use os;

include ":attributes:attributes";
include "include/client";
include "include/say";
include "include/sounds";

const RUNE_RECHARGE_MANA_COST := 25;

/*
 * Refills `runecharges` on rune-bearing equipment to the caster's THEOLOGY.
 * Reads `priestrune` to identify rune items.
 *
 * targ may be a player (recharges all equipped runed items, paying mana per
 * item up to the caster's available pool) or a single item.
 *
 * Returns the number of items recharged (0 on failure / no work needed).
 */
function RechargeRunesOnTarget(who, targ)
	if (!targ)
		SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
		return 0;
	endif

	var theology := CInt(AP_GetSkill(who, THEOLOGY));
	var maxCharges := theology;

	if (targ.acctname)
		var equippedItems := ListEquippedItems(targ);
		var itemsToRecharge := array{};
		var totalManaNeeded := 0;

		foreach item in equippedItems
			if (GetObjProperty(item, "priestrune"))
				if (CInt(GetObjProperty(item, "runecharges")) < maxCharges)
					itemsToRecharge.append(item);
					totalManaNeeded += RUNE_RECHARGE_MANA_COST;
				endif
			endif
			sleepms(2);
		endforeach

		var currentMana := AP_GetVital(who, "Mana");
		if (currentMana < totalManaNeeded)
			SendSysMessageEx(who, "Você não possui mana suficiente para recarregar todos os itens. Mana necessária: " + totalManaNeeded, SSM_FAIL);
			var affordableItems := CInt(currentMana / RUNE_RECHARGE_MANA_COST);
			if (affordableItems <= 0)
				return 0;
			endif

			var tempArray := array{};
			for i := 1 to affordableItems
				if (i <= itemsToRecharge.size())
					tempArray.append(itemsToRecharge[i]);
				endif
			endfor
			itemsToRecharge := tempArray;
		endif

		var itemsRecharged := 0;
		foreach item in itemsToRecharge
			SetObjProperty(item, "runecharges", maxCharges);
			AP_ConsumeVital(who, "Mana", RUNE_RECHARGE_MANA_COST);
			itemsRecharged += 1;
			PlayObjectCenteredEffect(item, FX_BLESS_EFFECT, 7, 16);
			sleepms(200);
			IncRevision(item);
		endforeach

		if (itemsRecharged > 0)
			PlaySoundEffect(targ, SFX_SPELL_BLESS);
			PlayStationaryEffect(targ.x, targ.y, targ.z, FX_SPARK_EFFECT, 2, 2);
			SendSysMessageEx(who, "Você recarregou " + itemsRecharged + " runa(s) sagrada(s) em " + targ.name + ".", SSM_INFO);
			SendSysMessageEx(targ, "Suas runas sagradas foram recarregadas pela benção divina.", SSM_INFO);
		else
			SendSysMessageEx(who, targ.name + " não possui runas que precisam ser recarregadas.", SSM_INFO);
		endif

		return itemsRecharged;
	endif

	if (!targ.isA(POLCLASS_ITEM))
		SendSysMessageEx(who, "Você deve alvejar um jogador ou um item.", SSM_FAIL);
		return 0;
	endif

	if (!GetObjProperty(targ, "priestrune"))
		SendSysMessageEx(who, "Este item não possui uma runa sagrada.", SSM_FAIL);
		return 0;
	endif

	if (CInt(GetObjProperty(targ, "runecharges")) >= maxCharges)
		SendSysMessageEx(who, "Esta runa já está totalmente carregada.", SSM_INFO);
		return 0;
	endif

	if (AP_GetVital(who, "Mana") < RUNE_RECHARGE_MANA_COST)
		SendSysMessageEx(who, "Você não possui mana suficiente.", SSM_FAIL);
		return 0;
	endif

	SetObjProperty(targ, "runecharges", maxCharges);
	AP_ConsumeVital(who, "Mana", RUNE_RECHARGE_MANA_COST);

	PlaySoundEffect(targ, SFX_SPELL_BLESS);
	PlayObjectCenteredEffect(targ, FX_BLESS_EFFECT, 7, 16);
	SendSysMessageEx(who, "Você recarregou a runa sagrada.", SSM_INFO);
	IncRevision(targ);
	return 1;
endfunction
