use os;

include ":equipsys:Controller/ArmorController";

function getDurabilityBySlot(item)
	return Max(getResistPhysicalBySlot(item.getSlot()));
endfunction

function getWeightByType(item)
	var weight;

	if(item.isA(POLCLASS_WEAPON))
		var maxMainDamage := item.getMainDamageMax();
		maxMainDamage := Max(array{maxMainDamage.SlashDamage, maxMainDamage.PiercingDamage, maxMainDamage.BluntDamage});

		var maxOffDamage := item.getOffDamageMax();
		maxOffDamage := Max(array{maxOffDamage.SlashDamage, maxOffDamage.PiercingDamage, maxOffDamage.BluntDamage});

		weight := CInt(maxMainDamage + maxOffDamage);
	elseif (item.isA(POLCLASS_ARMOR))
		if (item.isShield())
			weight := item.getAttrBase().DexPen;
		else
			// Poderia ter sido feito direto no weight. Deixar assim pra caso precise de algum MOD futuramente.
			var armorAttr := item.getAttrBase();
			var maxResist := Max(array{armorAttr.SlashResist, armorAttr.PiercingResist, armorAttr.BluntResist});
			maxResist += CInt(getDurabilityBySlot(item.getSlot()));
			weight := maxResist;
		endif
	endif
	return weight;
endfunction

function getSTRRequired(stats, item)
	stats.+STRRequired := 0;

	var weight := getWeightByType(item);
	stats.STRRequired := CInt(weight * 7.5);
	return stats;
endfunction