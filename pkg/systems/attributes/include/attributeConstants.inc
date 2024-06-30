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
	HUNGER			:= "Hunger"
endenum

enum STATS
	STRENGTH		:= "Strength",
	DEXTERITY		:= "Dexterity",
	INTELLIGENCE		:= "Intelligence",
	//CONSTITUTION		:= "Constitution",
	//WISDOM			:= "Wisdom",
	//CHARISMA		:= "Charisma"
endenum

enum SKILLS
	ALCHEMY			:= "Alchemy",
	MAGERY			:= "Magery",
	MAGICLORE		:= "MagicLore",

	PARRY			:= "Parry",
	MELEECOMBAT1H		:= "MeleeCombat1H",
	MELEECOMBAT2H		:= "MeleeCombat2H",
	RANGEDCOMBAT		:= "RangedCombat",
	TACTICS			:= "Tactics",

	SNEAK			:= "Sneak",
	PICKPOCKETING		:= "Pickpocketing",
	AWARENESS		:= "Awareness",
	MECHANICALAPITUDE	:= "MechanicalApitude",

	MEDICINE		:= "Medicine",
	SURVIVAL		:= "Survival",

	METALWORKING		:= "MetalWorking",
	WOODWORKING		:= "WoodWorking",
	DomesticWork		:= "DomesticWork",
	GATHERING		:= "Gathering",
	HANDLEANIMAL		:= "HandleAnimal",

	//pros npcs
	WRESTLING		:= "Wrestling"
endenum
