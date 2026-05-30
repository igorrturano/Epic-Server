/* $Id: attributeConstants.inc 834 2005-11-02 14:09:50Z austin $
 *
 * Purpose
 * Provide various functions and constants for the attribute system.
 *
 */
use uo;

enum VITALS
	HITS			:= "Hits",
	MANA			:= "Mana",
	STAMINA			:= "Stamina",
	HUNGER			:= "Hunger",
	DEATH_POINTS 	:= "DP",
	ADDICTION_STR		:= "Viciostr",
	ADDICTION_DEX		:= "Viciodex",
	ADDICTION_INT		:= "Vicioint",
endenum

enum STATS
	STRENGTH		:= "Strength",
	DEXTERITY		:= "Dexterity",
	INTELLIGENCE	:= "Intelligence",
	FORTITUDE		:= "Fortitude",
	REFLEXES		:= "Reflexes",
	WILL			:= "Will"
endenum

enum SKILLS
	ALCHEMY			:= "Alchemy",
	MAGERY			:= "Magery",
	ARCANA			:= "Arcana",
	SCIENCE			:= "Science",
	THEOLOGY		:= "Theology",

	PARRY			:= "Parry",
	ONEHANDED		:= "One_Handed",
	TWOHANDED		:= "Two_Handed",
	DUALWIELD		:= "Dual_Wield",
	RANGED			:= "Ranged",
	TACTICS			:= "Tactics",

	SNEAK			:= "Sneak",
	PICKPOCKETING	:= "Pickpocketing",
	AWARENESS		:= "Awareness",
	MECHANICS		:= "Mechanics",

	MEDICINE		:= "Medicine",
	SURVIVAL		:= "Survival",

	METALWORKING	:= "Metal_Working",
	WOODWORKING		:= "Wood_Working",
	DOMESTIC_WORK	:= "Domestic_Work",
	GATHERING		:= "Gathering",
	HANDLEANIMAL	:= "Handle_Animal",

	// Path-rework additions (Phase 1 splits + Nature). Legacy entries above
	// stay until spec 06 cleanup so existing reads keep compiling during the
	// migration window.
	NATURE			:= "Nature",
	BEASTMASTERY	:= "Beastmastery",
	TAMING			:= "Taming",
	MINING			:= "Mining",
	LUMBERJACKING	:= "Lumberjacking",
	TAILORING		:= "Tailoring",
	PROVISIONING	:= "Provisioning",

	//pros npcs
	WRESTLING		:= "Wrestling"
endenum