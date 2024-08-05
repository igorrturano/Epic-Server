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
	MAGICLORE		:= "Magic_Lore",

	PARRY			:= "Parry",
	ONEHANDED		:= "One_Handed",
	TWOHANDED		:= "Two_Handed",
	DUALWIELD		:= "Dual_Wield",
	RANGED			:= "Ranged",
	TACTICS			:= "Tactics",

	SNEAK			:= "Sneak",
	PICKPOCKETING		:= "Pickpocketing",
	AWARENESS		:= "Awareness",
	MECHANICS	:= "Mechanics",

	MEDICINE		:= "Medicine",
	SURVIVAL		:= "Survival",

	METALWORKING		:= "Metal_Working",
	WOODWORKING		:= "Wood_Working",
	Domestic_Work		:= "Domestic_Work",
	GATHERING		:= "Gathering",
	HANDLEANIMAL		:= "Handle_Animal",

	//pros npcs
	WRESTLING		:= "Wrestling"
endenum