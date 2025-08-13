enum ACHIEVEMENT_TYPE
	ACHIEVEMENT := "ACHIEVEMENT",
	ACHIEVEMENT_COMBAT := ACHIEVEMENT + "_COMBAT",
	ACHIEVEMENT_EXPLORATION := ACHIEVEMENT + "_EXPLORATION",
	ACHIEVEMENT_CRAFTING := ACHIEVEMENT + "_CRAFTING"
endenum

class Achievement()
	var name; // { get; }
	var ref; // { get; }
	var refName; // { get; set; }

	var level;
	var xp;
	var value;

	function Achievement(this, name, ref, refName := "", level := 0, xp := 0, value := 0)
		this.name := name;
		this.ref := ref;
		this.refName := refName;

		this.level := CInt(level);
		this.xp := CInt(xp);
		this.value := CInt(value);
	endfunction

	function GetName(this)
		return this.name;
	endfunction

	function GetRef(this)
		return this.ref;
	endfunction

	function GetRefName(this)
		return this.refName;
	endfunction

	function GetLevel(this)
		return this.level;
	endfunction

	function GetXP(this)
		return this.xp;
	endfunction

	function SetRefName(this, value)
		this.refName := value;
	endfunction

	function SetLevel(this, value)
		this.level := value;
	endfunction

	function SetXP(this, value)
		this.xp := value;
	endfunction

	function SetValue(this, value)
		this.value := CInt(value);
	endfunction

	function IncrementXP(this, value)
		this.xp += CInt(value);
	endfunction

	function IncrementLevel(this)
		this.level++;
	endfunction

	function IncrementValue(this, value)
		this.value += CInt(value);
	endfunction

	function Serialize(this)
		return struct{
			name := this.name,
			ref := this.ref,
			refName := this.refName,

			level := this.level,
			xp := this.xp,
			value := this.value
		};
	endfunction

	function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for Achievement deserialization." }; endif

		this.name := data.name;
		this.ref := data.ref;
		this.refName := data.refName;
		
		this.level := CInt(data.level);
		this.xp := CInt(data.xp);
		this.value := CInt(data.value);
	endfunction

	function toString(this)
		return $"Achievement para {this.name} (xp={this.xp}, level={this.level}, value={this.value})";
	endfunction
endclass

enum ACHIEVEMENT_NAME_COMBAT
	ACHIEVEMENT_NAME_HITS := "hits",
	ACHIEVEMENT_NAME_BLOCKED := "blocked",
	ACHIEVEMENT_NAME_KILLS := "kills",

	ACHIEVEMENT_NAME_DAMAGE_DONE := "damage_done",
	ACHIEVEMENT_NAME_DAMAGE_TAKEN := "damage_taken",
	ACHIEVEMENT_NAME_DAMAGE_MAX := "damage_max",
	ACHIEVEMENT_NAME_DAMAGE_MAX_TAKEN := "damage_max_taken",

	ACHIEVEMENT_NAME_CRITICAL_HITS := "critical_hits",
	ACHIEVEMENT_NAME_CRITICAL_DAMAGE := "critical_damage",
	ACHIEVEMENT_NAME_CRITICAL_DAMAGE_MAX := "critical_damage_max",

	ACHIEVEMENT_NAME_HEALING_ON_SELF := "healing_on_self",
	ACHIEVEMENT_NAME_HEALING_ON_OTHERS := "healing_on_others",
	ACHIEVEMENT_NAME_HEALING_MAX := "healing_max",
	ACHIEVEMENT_NAME_HEALING_TOTAL := "healing_total",

	ACHIEVEMENT_NAME_MANA_SPENT := "mana_spent",
	ACHIEVEMENT_NAME_MANA_REGEN := "mana_regen",
	ACHIEVEMENT_NAME_MANA_REGEN_MAX := "mana_regen_max",
	ACHIEVEMENT_NAME_MANA_REGEN_TOTAL := "mana_regen_total",

	ACHIEVEMENT_NAME_SPELLS_CASTS := "spells_casts",
	ACHIEVEMENT_NAME_SPELLS_MOST_CASTED := "spells_casts_most_casted",
endenum

enum ACHIEVEMENT_NAME_OTHERS
	ACHIEVEMENT_NAME_FISH_CAUGHT := "fish_caught",
	ACHIEVEMENT_NAME_FISH_CAUGHT_WITH_BAIT := "fish_caught_with_bait",
	ACHIEVEMENT_NAME_RESOURCES_COLLECTED := "resources_collected",
	ACHIEVEMENT_NAME_CORPSES_LOOTED := "corpses_looted",
	ACHIEVEMENT_NAME_MOBS_TRACKED := "mobs_tracked",
	ACHIEVEMENT_NAME_MOBS_DETECTED_AWARENESS := "mobs_detected_awareness",
	ACHIEVEMENT_NAME_BOSSES_KILLED := "bosses_killed",
	ACHIEVEMENT_NAME_HIGHEST_TIER_TARGET_KILLED := "highest_tier_target_killed",
	ACHIEVEMENT_NAME_MAX_DAILY_XP_GAIN := "max_daily_xp_gain",
	ACHIEVEMENT_NAME_TIMES_DISGUISED := "times_disguised",
	ACHIEVEMENT_NAME_CONSECUTIVE_HITS_RED := "consecutive_hits_red",
	ACHIEVEMENT_NAME_ALLIES_HEALED := "allies_healed",
	ACHIEVEMENT_NAME_ALLIES_HEALED_AMOUNT := "allies_healed_amount",
	ACHIEVEMENT_NAME_ABILITIES_USED := "abilities_used",
	ACHIEVEMENT_NAME_ABILITY_MOST_USED := "ability_most_used"
endenum

// OTHERS
// TODO: Peixes pescados
// TODO: Peixes pescados com isca
// TODO: Quantas magias castadas 
// TODO: Qual magia mais castada
// TODO: Quanto recurso recolhido
// TODO: Quantos corpos abertos
// TODO: Quantos mobs rastreados
// TODO: Quantos mobs descobertos com awareness
// TODO: Quantos boss mortos
// TODO: Qual tier mais alto voce ja matou/ajudou a matar
// TODO: Top ganho de XP num dia
// TODO: Quantas vezes ja se disfarçou
// TODO: Quantos golpes acertados consecutivos sem errar em criaturas red
// TODO: Quantos aliados voce ja curou
// TODO: Quanto voce ja curou seus aliados
// TODO: Quantas vezes utilizou suas habilidades
// TODO: Qual habilidade mais utilizada