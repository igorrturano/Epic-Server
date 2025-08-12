enum ACHIEVEMENT_TAGS
	ACHIEVEMENT := "ACHIEVEMENT",
	ACHIEVEMENT_COMBAT := ACHIEVEMENT + "_COMBAT",
	ACHIEVEMENT_EXPLORATION := ACHIEVEMENT + "_EXPLORATION",
	ACHIEVEMENT_CRAFTING := ACHIEVEMENT + "_CRAFTING"
endenum

class Achievement()
	var criteria;
	var playerRef; // Readonly (FK)
	var refObj; // Readonly (FK)
	var refName;

	var level;
	var xp;
	var value;

	function Achievement(this, criteria, playerRef, refObj, refName := "", level := 0, xp := 0, value := 0)
		this.criteria := criteria;
		this.playerRef := playerRef;
		this.refObj := refObj;
		this.refName := refName == "" ? CStr(refObj) : refName;

		this.level := CInt(level);
		this.xp := CInt(xp);
		this.value := CInt(value);
	endfunction

	function GetCriteria(this)
		return this.criteria;
	endfunction

	function GetPlayerRef(this)
		return this.playerRef;
	endfunction

	function GetRefObj(this)
		return this.refObj;
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
			playerRef := this.playerRef,
			criteriaRef := this.criteriaRef,
			name := this.name,

			level := this.level,
			xp := this.xp,
			criteria := this.criteria
		};
	endfunction

	function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for Achievement deserialization." }; endif
		this.playerRef := data.playerRef;
		this.criteriaRef := data.criteriaRef;
		this.name := CStr(data.name);

		this.level := CInt(data.level);
		this.xp := CInt(data.xp);
		this.criteria := data.criteria;
	endfunction

	function toString(this)
		return $"Achievement para {this.name} (playerRef={this.playerRef}, criteriaRef={this.criteriaRef}, xp={this.xp}, level={this.level}, criteria={this.criteria})";
	endfunction
endclass

enum ACHIEVEMENT_CRITERIA_COMBAT
	ACHIEVEMENT_CRITERIA_HITS := "hits",
	ACHIEVEMENT_CRITERIA_BLOCKED := "blocked",
	ACHIEVEMENT_CRITERIA_KILLS := "kills",

	ACHIEVEMENT_CRITERIA_DAMAGEDONE := "damagedone",
	ACHIEVEMENT_CRITERIA_DAMAGETAKEN := "damagetaken",
	ACHIEVEMENT_CRITERIA_DAMAGEMAX := "damagemax",
	ACHIEVEMENT_CRITERIA_DAMAGEMAX_TAKEN := "damagemaxtaken",

	ACHIEVEMENT_CRITERIA_CRITICALHITS := "criticalhits",
	ACHIEVEMENT_CRITERIA_CRITICALDAMAGE := "criticaldamage",
	ACHIEVEMENT_CRITERIA_CRITICALDAMAGEMAX := "criticaldamagemax",

	ACHIEVEMENT_CRITERIA_HEALINGONSELF := "healingonself",
	ACHIEVEMENT_CRITERIA_HEALINGONOTHERS := "healingonothers",
	ACHIEVEMENT_CRITERIA_HEALINGMAX := "healingmax",
	ACHIEVEMENT_CRITERIA_HEALINGTOTAL := "healingtotal",

	ACHIEVEMENT_CRITERIA_MANASPENT := "manaspent",
	ACHIEVEMENT_CRITERIA_MANAREGEN := "manaregen",
	ACHIEVEMENT_CRITERIA_MANAREGENMAX := "manaregenmax",
	ACHIEVEMENT_CRITERIA_MANAREGENTOTAL := "manaregentotal",

	ACHIEVEMENT_CRITERIA_SPELLSCASTS := "spellscasts",
	ACHIEVEMENT_CRITERIA_SPELLSMOSTCASTED := "spellscastsmostcasted",
endenum

enum ACHIEVEMENT_CRITERIA_OTHERS
	ACHIEVEMENT_CRITERIA_FISHCAUGHT := "fishcaught",
	ACHIEVEMENT_CRITERIA_FISHCAUGHT_WITH_BAIT := "fishcaughtwithbait",
	ACHIEVEMENT_CRITERIA_RESOURCES_COLLECTED := "resourcescollected",
	ACHIEVEMENT_CRITERIA_CORPSES_LOOTED := "corpseslooted",
	ACHIEVEMENT_CRITERIA_MOBS_TRACKED := "mobstracked",
	ACHIEVEMENT_CRITERIA_MOBS_DETECTED_AWARENESS := "mobsdetectedawareness",
	ACHIEVEMENT_CRITERIA_BOSSES_KILLED := "bosseskilled",
	ACHIEVEMENT_CRITERIA_HIGHEST_TIER_TARGET_KILLED := "highesttiertargetkilled",
	ACHIEVEMENT_CRITERIA_MAX_DAILY_XP_GAIN := "maxdailyxpgain",
	ACHIEVEMENT_CRITERIA_TIMES_DISGUISED := "timesdisguised",
	ACHIEVEMENT_CRITERIA_CONSECUTIVE_HITS_RED := "consecutivehitsred",
	ACHIEVEMENT_CRITERIA_ALLIES_HEALED := "allieshealed",
	ACHIEVEMENT_CRITERIA_ALLIES_HEALED_AMOUNT := "allieshealedamount",
	ACHIEVEMENT_CRITERIA_ABILITIES_USED := "abilitiesused",
	ACHIEVEMENT_CRITERIA_ABILITY_MOST_USED := "abilitymostused"
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