use uo;

include ":abilities:models/CombatAbility";

// Extractor Vigor - ABILITY_ID::EXTRACTOR_VIGOR (3560)
// Passive: Reduces stamina cost on mining/chopping/gathering.
// Effect handled in gathering scripts.
class ExtractorVigor(CombatAbility)
	function ExtractorVigor(this)
		Super(ABILITY_ID::EXTRACTOR_VIGOR);
	endfunction
endclass

// Gem Miner - ABILITY_ID::GEM_MINER (3561)
// Passive: 8% gem chance (vs 2%) while mining.
// Effect handled in mining.inc.
class GemMiner(CombatAbility)
	function GemMiner(this)
		Super(ABILITY_ID::GEM_MINER);
	endfunction
endclass

// Mine Rarities - ABILITY_ID::MINE_RARITIES (3563)
// Passive: Chance for rare ores in any mine.
// Effect handled in oremining/mining.src.
class MineRarities(CombatAbility)
	function MineRarities(this)
		Super(ABILITY_ID::MINE_RARITIES);
	endfunction
endclass

// Treasure From Trash - ABILITY_ID::TREASURE_FROM_TRASH (3565)
// Passive: Rare drops from smelting impurities.
// Effect handled in smelting/smelt.inc.
class TreasureFromTrash(CombatAbility)
	function TreasureFromTrash(this)
		Super(ABILITY_ID::TREASURE_FROM_TRASH);
	endfunction
endclass

// Pack Mule - ABILITY_ID::PACK_MULE (3566)
// Passive: +60 carry capacity.
// Effect handled in weight/capacity system.
class PackMule(CombatAbility)
	function PackMule(this)
		Super(ABILITY_ID::PACK_MULE);
	endfunction
endclass

// Talented Miner - ABILITY_ID::TALENTED_MINER (3567)
// Passive: +2 ore per harvest.
// Effect handled in mining scripts.
class TalentedMiner(CombatAbility)
	function TalentedMiner(this)
		Super(ABILITY_ID::TALENTED_MINER);
	endfunction
endclass

// Talented Lumberjack - ABILITY_ID::TALENTED_LUMBERJACK (3568)
// Passive: +4 wood per harvest.
// Effect handled in lumberjack.src.
class TalentedLumberjack(CombatAbility)
	function TalentedLumberjack(this)
		Super(ABILITY_ID::TALENTED_LUMBERJACK);
	endfunction
endclass

// Prospector - ABILITY_ID::PROSPECTOR (3569)
// Passive: -10 mining difficulty.
// Effect handled in mining.src.
class Prospector(CombatAbility)
	function Prospector(this)
		Super(ABILITY_ID::PROSPECTOR);
	endfunction
endclass
