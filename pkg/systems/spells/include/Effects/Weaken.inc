use uo;

include "include/client";
include "include/sounds";
include "include/say";
include ":spells:Controller/SpellController";

// Single source of truth for the STR-debuff "enfraquecer" effect.
// Callers compute amt/duration their own way (resist roll, skill scaling,
// armed melee proc) and delegate the actual application here. School-
// specific flavor (e.g. necromantic corruption) stays in the caller —
// this helper is shared by the theology-based shaman melee proc too.
function ApplyWeakenEffect(caster, targ, amt, duration, spellId)
	SC_RemoveEffect(targ, spellId);

	var opts := struct{};
	opts.+str_mod := -amt;

	SC_ApplyEffect(caster, targ, opts, duration, spellId);
	PrintTextPrivate(targ, "-" + amt + " Str", caster, 3, 38);
	SC_HandleSound(caster, targ, SFX_SPELL_WEAKEN);

	SendSysMessageEx(caster, "Voce deixou o alvo mais fraco.", SSM_INFO);

	return 1;
endfunction
