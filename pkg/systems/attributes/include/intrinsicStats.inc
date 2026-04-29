use uo;
use util;

include ":attributes:attributes";
include ":charactercreation:habilidades";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:models/Ability";
include ":tn:cooldown";
// Intrinsic stat calculation core.
// Shared by intrinsicMods.src (exports cfg-registered hooks) and
// sendStats.src (packet hook that needs all 3 stats per paperdoll view).
//
// Values are returned multiplied by INTRINSIC_TENTHS (10) — same
// convention as the attributes system expects from GetIntrinsicMod
// callbacks. Divide by INTRINSIC_TENTHS to get user-facing values.

const INTRINSIC_TENTHS := 10;

// ============================================================
// Public: single-call helpers (create context then calc).
// Use these if you need ONE stat. For 2+ stats, use the batch.
// ============================================================

function IntrinsicFortitudeValue(mobile)
	if (mobile.npctemplate)
		return 0;
	endif
	var chardata := GetObjProperty(mobile, "chardata");
	if (chardata == error)
		return 0;
	endif
	var wrapper := MobileWrapper(mobile);
	var entropy := wrapper.GetEntropy();
	var commonStats := IntrinsicCommonStatsIncrease(wrapper, chardata);
	return IntrinsicCalcFortitude(mobile, wrapper, chardata, entropy, commonStats);
endfunction

function IntrinsicReflexesValue(mobile)
	if (mobile.npctemplate)
		return 0;
	endif
	var chardata := GetObjProperty(mobile, "chardata");
	if (chardata == error)
		return 0;
	endif
	var wrapper := MobileWrapper(mobile);
	var commonStats := IntrinsicCommonStatsIncrease(wrapper, chardata);
	return IntrinsicCalcReflexes(mobile, wrapper, chardata, commonStats);
endfunction

function IntrinsicWillValue(mobile)
	if (mobile.npctemplate)
		return 0;
	endif
	var chardata := GetObjProperty(mobile, "chardata");
	if (chardata == error)
		return 0;
	endif
	var wrapper := MobileWrapper(mobile);
	var entropy := wrapper.GetEntropy();
	var commonStats := IntrinsicCommonStatsIncrease(wrapper, chardata);
	return IntrinsicCalcWill(mobile, wrapper, chardata, entropy, commonStats);
endfunction

// ============================================================
// Public batch: builds context ONCE, computes all 3 stats.
// Use when the caller needs 2+ intrinsics for the same mobile
// (ex: sendStats packet builder, character sheet gumps).
// Saves 2x MobileWrapper creation + 2x chardata ObjProperty read.
// ============================================================

function IntrinsicStatsBatch(mobile)
	var result := struct;
	result.+fortitude := 0;
	result.+reflexes := 0;
	result.+will := 0;

	if (mobile.npctemplate)
		return result;
	endif
	var chardata := GetObjProperty(mobile, "chardata");
	if (chardata == error)
		return result;
	endif

	var wrapper := MobileWrapper(mobile);
	var entropy := wrapper.GetEntropy();
	var commonStats := IntrinsicCommonStatsIncrease(wrapper, chardata);

	result.fortitude := IntrinsicCalcFortitude(mobile, wrapper, chardata, entropy, commonStats);
	result.reflexes := IntrinsicCalcReflexes(mobile, wrapper, chardata, commonStats);
	result.will := IntrinsicCalcWill(mobile, wrapper, chardata, entropy, commonStats);

	return result;
endfunction

// ============================================================
// Calc helpers — receive pre-built context.
// Wrappers/structs em POL já são passados por referência;
// não precisa byref explícito (segue padrão de HitController
// CalculateWeaponHitModifier e AttackInfo constructor).
// ============================================================

function IntrinsicCalcFortitude(mobile, wrapper, chardata, entropy, commonStats)
	var fortitude := 0;

	if (chardata.trait == "Fortitude do Urso")
		fortitude += 5;
	endif

	if (wrapper.HasAbility(ABILITY_ID::CACADOR_IMPLACAVEL))
		fortitude += 2;
	endif

	if (GetObjProperty(mobile, "corruption"))
		fortitude -= CInt(GetObjProperty(mobile, "corruption_penalty"));
	endif

	if (chardata.classe == "Guerreiro")
		fortitude += 5;
	else
		fortitude += 2;
	endif

	if (chardata.raca == HUMANO)
		fortitude += 2;
	endif

	if (entropy > 4000)
		fortitude += CInt(entropy / 4000);
	endif
	if (entropy <= -4000)
		fortitude -= CInt(abs(entropy) / 4000);
	endif

	fortitude += commonStats;
	fortitude += CInt(AP_GetStat(mobile, STRENGTH) / 20);
	fortitude += CInt(GetObjProperty(mobile, "#FortitudeMod"));
	return fortitude * INTRINSIC_TENTHS;
endfunction

function IntrinsicCalcReflexes(mobile, wrapper, chardata, commonStats)
	var reflexos := 0;

	if (chardata.trait == "Reflexos de Pantera")
		reflexos += 5;
	endif

	if (wrapper.HasAbility(ABILITY_ID::REAPROVEITAR_PERGAMINHO))
		reflexos += 5;
	endif

	if (chardata.classe == "Ladino")
		reflexos += 5;
	else
		reflexos += 2;
	endif

	if (chardata.raca == HUMANO)
		reflexos += 2;
	endif

	reflexos += commonStats;
	reflexos += CInt(AP_GetStat(mobile, DEXTERITY) / 20);
	reflexos += CInt(GetObjProperty(mobile, "#ReflexosMod"));
	return reflexos * INTRINSIC_TENTHS;
endfunction

function IntrinsicCalcWill(mobile, wrapper, chardata, entropy, commonStats)
	var vontade := 0;

	if (chardata.trait == "Vontade da Aguia")
		vontade += 5;
	endif

	if (chardata.classe == "Sabio")
		vontade += 5;
	else
		vontade += 2;
	endif

	if (chardata.culture == ALURIEL)
		vontade += 2;
	endif

	if (GetCooldown(mobile, "VontadeIndomavel"))
		vontade += 5;
	elseif (wrapper.HasAbility(ABILITY_ID::SIGILO_DE_PROTECAO))
		vontade += 5;
	elseif (wrapper.HasAbility(ABILITY_ID::CONEXAO_DIVINA))
		vontade += 8;
	elseif (wrapper.HasAbility(ABILITY_ID::CONSAGRACAO_DIVINA))
		vontade += CInt(AP_GetSkill(mobile, THEOLOGY) / 20);
	endif

	if (entropy > 4000)
		vontade += CInt(entropy / 4000);
	endif
	if (entropy <= -4000)
		vontade -= CInt(abs(entropy) / 4000);
	endif

	vontade += commonStats;
	vontade += CInt(AP_GetStat(mobile, INTELLIGENCE) / 20);
	vontade += CInt(GetObjProperty(mobile, "#VontadeMod"));
	return vontade * INTRINSIC_TENTHS;
endfunction

function IntrinsicCommonStatsIncrease(wrapper, chardata)
	var valor := 0;

	if (chardata.raca == HUMANO)
		valor += 2;
	endif

	if (wrapper.HasAbility(ABILITY_ID::BODY_AND_MIND))
		valor += 5;
	endif

	return valor;
endfunction
