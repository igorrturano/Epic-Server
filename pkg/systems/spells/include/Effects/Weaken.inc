use uo;

include ":spells:tnmagery";
include "include/client";
include "include/sounds";
include "include/say";
include ":spells:Controller/SpellController";

// Single source of truth for the STR-debuff "enfraquecer" effect.
// Callers compute amt/duration their own way (resist roll, skill scaling,
// armed melee proc) and delegate the actual application here.
function ApplyWeakenEffect(caster, targ, amt, duration, spellId)
	SC_RemoveEffect(targ, spellId);

	var opts := struct{};
	opts.+str_mod := -amt;

	SC_ApplyEffect(caster, targ, opts, duration, spellId);
	PrintTextPrivate(targ, "-" + amt + " Str", caster, 3, 38);
	SC_HandleSound(caster, targ, SFX_SPELL_WEAKEN);

	SendSysMessageEx(caster, "Voce deixou o alvo mais fraco.", SSM_INFO);
	Corromper(caster, 1, targ);

	return 1;
endfunction
