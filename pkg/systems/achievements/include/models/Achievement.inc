class Achievement()
	var name := "";
	var ref := "";
	var refName := "";

	var level := 0;
	var xp := 0;
	var value := 0;

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

	function GetValue(this)
		return this.value;
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

	function IncrementLevel(this, value := 1)
		this.level += CInt(value);
	endfunction

	function IncrementValue(this, value)
		this.value += CInt(value);
	endfunction

	function toString(this)
		return $"Achievement para {this.name} (ref={this.ref}, refName={this.refName}, xp={this.xp}, level={this.level}, value={this.value})";
	endfunction
endclass

enum class ACHIEVEMENT_TYPE
	ACHIEVEMENT := "ACHIEVEMENT",
	COMBAT := ACHIEVEMENT + "_COMBAT",
	EXPLORATION := ACHIEVEMENT + "_EXPLORATION",
	CRAFTING := ACHIEVEMENT + "_CRAFTING"
endenum

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

enum class ACHIEVEMENT_EXPLORATION_CATEGORY
	DUNGEONS_EXPLORED := "dungeons_explored",
	LORE_ITEMS_FOUND := "lore_items_found",
	CHESTS_OPENED := "chests_opened",
	TRAPS_DISARMED := "traps_disarmed",
	MAX_SNEAK_STEPS := "max_sneak_steps",
	DUNGEONS_COMPLETED := "dungeons_completed",
	UNIQUE_BOSSES_DEFEATED := "unique_bosses_defeated",
	SECRET_AREAS_DISCOVERED := "secret_areas_discovered",
	UNIQUE_NPCS_FOUND := "unique_npcs_found"
endenum

enum class ACHIEVEMENT_MARKET_CATEGORY
	COINS_SPENT := "coins_spent",
	COINS_ACQUIRED := "coins_acquired",
	MAX_COINS_ACCUMULATED := "max_coins_accumulated",
	ITEMS_PLACED_ON_MERCHANT := "items_placed_on_merchant"
endenum

enum class ACHIEVEMENT_SKILL_CATEGORY
	_PLACEHOLDER // TODO: Colocado apenas pra conseguir compilar, adicionar achievements aqui
endenum

enum class ACHIEVEMENT_CRAFTING_CATEGORY
	CRAFTED_NORMAL := "crafted_normal",
	CRAFTED_EXCEPTIONAL := "crafted_exceptional",
	FORGED_WEAPONS := "forged_weapons",
	FORGED_SHIELDS := "forged_shields",
	CREATED_POTIONS := "created_potions",
	REPAIRED_EQUIPMENT := "repaired_equipment"
endenum

enum class ACHIEVEMENT_COLLECTION_CATEGORY
	BOOKS_READ := "books_read",
	RECIPES_DISCOVERED := "recipes_discovered",
	MOUNTS_TAMED := "mounts_tamed",
	PETS_COLLECTED := "pets_collected",
	TITLES_UNLOCKED := "titles_unlocked"
endenum

enum class ACHIEVEMENT_OTHERS_CATEGORY
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
	ABILITIES_MOST_USED := "abilities_most_used"
endenum
