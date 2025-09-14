enum ACHIEVEMENT_TYPE
	ACHIEVEMENT := "ACHIEVEMENT",
	ACHIEVEMENT_COMBAT := ACHIEVEMENT + "_COMBAT",
	ACHIEVEMENT_EXPLORATION := ACHIEVEMENT + "_EXPLORATION",
	ACHIEVEMENT_CRAFTING := ACHIEVEMENT + "_CRAFTING"
endenum

class Achievement()
	var name := ""; // { get; }
	var ref := ""; // { get; }
	var refName := ""; // { get; set; }

	var level := 0; // { get; set; }
	var xp := 0; // { get; set; }
	var value := 0; // { get; set; }

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
		return $"Achievement para {this.name} (ref={this.ref}, refName={this.refName}, xp={this.xp}, level={this.level}, value={this.value})";
	endfunction
endclass

enum class ACHIEVEMENT_COMBAT_CATEGORY
	HITS := "hits",
	BLOCKED := "blocked",
	KILLS := "kills",

	DAMAGE_DONE := "damage_done",
	DAMAGE_TAKEN := "damage_taken",
	DAMAGE_MAX := "damage_max",
	DAMAGE_MAX_TAKEN := "damage_max_taken",

	CRITICAL_HITS := "critical_hits",
	CRITICAL_DAMAGE := "critical_damage",
	CRITICAL_DAMAGE_MAX := "critical_damage_max",

	HEALING_ON_SELF := "healing_on_self",
	HEALING_ON_OTHERS := "healing_on_others",
	HEALING_MAX := "healing_max",
	HEALING_TOTAL := "healing_total",

	MANA_SPENT := "mana_spent",
	MANA_REGEN := "mana_regen",
	MANA_REGEN_MAX := "mana_regen_max",
	MANA_REGEN_TOTAL := "mana_regen_total",

	SPELLS_CASTS := "spells_casts",
	SPELLS_MOST_CASTED := "spells_casts_most_casted",
endenum

enum class ACHIEVEMENT_NAME_OTHERS
	FISH_CAUGHT := "fish_caught",
	FISH_CAUGHT_WITH_BAIT := "fish_caught_with_bait",
	RESOURCES_COLLECTED := "resources_collected",
	CORPSES_LOOTED := "corpses_looted",
	MOBS_TRACKED := "mobs_tracked",
	MOBS_DETECTED_AWARENESS := "mobs_detected_awareness",
	BOSSES_KILLED := "bosses_killed",
	HIGHEST_TIER_TARGET_KILLED := "highest_tier_target_killed",
	MAX_DAILY_XP_GAIN := "max_daily_xp_gain",
	TIMES_DISGUISED := "times_disguised",
	CONSECUTIVE_HITS_RED := "consecutive_hits_red",
	ALLIES_HEALED := "allies_healed",
	ALLIES_HEALED_AMOUNT := "allies_healed_amount",
	ABILITIES_USED := "abilities_used",
	ABILITY_MOST_USED := "ability_most_used"
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